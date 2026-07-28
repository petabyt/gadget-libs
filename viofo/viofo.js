import { WiFi } from "pak:wifi";
import { Module } from "pak:runtime";
import { HttpSocket } from "pak:http";
import { test } from "test.js";

// From https://github.com/Brandon-T/Viofo-Cam/blob/ec533e9080574d12bb490f377a2f01b7cd64485d/Viofo/Models/Constants/Command.swift
let Commands = {
	AUTO_POWER_OFF: 3007,
	BATTERY_CUT_OFF_TIME: 8232,
	BATTERY_CUT_OFF_VOLTAGE: 9343,
	BEEP_SOUND: 9094,
	BOOT_DELAY: 9424,
	CAMERA_MODEL_STAMP: 9216,
	CAPTURE_SIZE: 1002,
	CARD_FREE_SPACE: 3017,
	CAR_NUMBER: 9422,
	CHANGE_MODE: 3001,
	CUSTOM_TEXT_STAMP: 9417,
	DEFAULT_PORT: 3333,
	DELETE_ALL_FILE: 4004,
	DELETE_ONE_FILE: 4003,
	DISABLE_REAR_CAMERA: 8098,
	FIRMWARE_VERSION: 3012,
	FORMAT_MEMORY: 3010,
	FREQUENCY: 9406,
	FS_UNKNOW_FORMAT: 3025,
	GET_BATTERY_LEVEL: 3019,
	GET_CARD_STATUS: 3024,
	GET_CAR_NUMBER: 9426,
	GET_CURRENT_STATE: 3014,
	GET_CUSTOM_STAMP: 9427,
	GET_FILE_LIST: 3015,
	GET_SENSOR_STATUS: 9432,
	GET_UPDATE_FW_PATH: 3026,
	GET_WIFI_SSID_PASSWORD: 3029,
	GPS: 9410,
	GPS_INFO_STAMP: 9214,
	HDR_TIME: 8251,
	HDR_TIME_GET: 8252,
	HEART_BEAT: 3016,
	IMAGE_ROTATE: 9093,
	INTERIOR_CAM_FISHEYE_MODE: 9339,
	IR_CAMERA_COLOR: 9218,
	LANGUAGE: 3008,
	LENSES_NUMBER: 8250,
	LIVE_VIDEO_SOURCE: 3028,
	LIVE_VIEW_BITRATE: 2014,
	LIVE_VIEW_URL: 2019,
	LOGO_STAMP: 9229,
	MOTION_DET: 2006,
	MOVIE_AUDIO: 2007,
	MOVIE_AUTO_RECORDING: 2012,
	MOVIE_BITRATE: 9212,
	MOVIE_CYCLIC_REC: 2003,
	MOVIE_DATE_PRINT: 2008,
	MOVIE_EV_INTERIOR: 0,
	MOVIE_EV_REAR: 9217,
	MOVIE_EXPOSURE: 2005,
	MOVIE_GSENSOR_SENS: 2011,
	MOVIE_LIVE_VIEW_CONTROL: 2015,
	MOVIE_MAX_RECORD_TIME: 2009,
	MOVIE_RECORD: 2001,
	MOVIE_RECORDING_TIME: 2016,
	MOVIE_REC_BITRATE: 2013,
	MOVIE_RESOLUTION: 2002,
	MOVIE_WDR: 2004,
	MULTIPLEX_VIDEO: 9342,
	PARKING_FILES_STORAGE: 9340,
	PARKING_G_SENSOR: 9220,
	PARKING_MODE: 9421,
	PARKING_MOTION_DETECTION: 9221,
	PARKING_RECORDING_TIMER: 9428,
	PHOTO_AVAIL_NUM: 1003,
	PHOTO_CAPTURE: 1001,
	PRIVACY_MODE: 9330,
	REAR_CAMERA_MIRROR: 9219,
	RECONNECT_WIFI: 3018,
	REMOTE_CONTROL_FUNCTION: 2020,
	REMOVE_LAST_USER: 3023,
	RESET_SETTING: 3011,
	RESOLUTION_FRAMES: 8076,
	RESTART_CAMERA: 9095,
	SCREEN: 4002,
	SCREEN_SAVER: 9405,
	SET_DATE: 3005,
	SET_TIME: 3006,
	SPEED_UNIT: 9412,
	SSD_CARD: 0,
	STAMP_COLOR: 9331,
	STORAGE_TYPE: 9434,
	FORMAT_SSD: 9317,
	THUMB: 4001,
	TIME_LAPSE_RECORDING: 9201,
	TIME_ZONE: 9411,
	TRIGGER_RAW_ENCODE: 2017,
	TV_FORMAT: 3009,
	VOICE_CONTROL: 9453,
	VOICE_CONTROL_INFO: 9228,
	VOICE_NOTIFICATION_VOLUME: 8053,
	WIFI_NAME: 3003,
	WIFI_PWD: 3004,
	WIFI_STATION_CONFIGURATION: 3032,
};

function parseXml(xml) {
	let offset = 0;
	function skip() {
		if (!(offset < xml.length)) return true;
		while (xml[offset] == ' ' || xml[offset] == '\n' || xml[offset] == '\r') offset++;
		return !(offset < xml.length);
	}
	function parse(curr) {
		let thisDepth = 0;
		for (; offset < xml.length; offset++) {
			if (skip()) return curr;
			let tag = "";
			if (xml[offset] == '<') {
				offset++;
				if (xml[offset] == '?') {
					while (xml[offset] != '>' && offset < xml.length) offset++;
					continue;
				} else if (xml[offset] == '/') {
					offset--;
					return curr;
				}
				while (xml[offset] != '>' && offset < xml.length) tag += xml[offset++];
				offset++;
			} else {
				let value = "";
				while (xml[offset] != '<' && offset < xml.length) value += xml[offset++];
				return value;
			}
			if (skip()) return curr;
			if (curr[tag] == undefined) {
				curr[tag] = parse({});
			} else if (Array.isArray(curr[tag])) {
				curr[tag].push(parse({}));
			} else {
				curr[tag] = [curr[tag]];
				curr[tag].push(parse({}));
			}
			if (xml[offset] == '<' && xml[offset + 1] == '/') {
				offset += 2;
				while (xml[offset] != '>' && offset < xml.length) offset++;
			}
		}
		return curr;
	}
	return parse({});
}

class Veement extends Module {
	ip = "192.168.1.254";
	wifiAdapter = null;
	fileList = null;
	constructor() {
		super()
	}
	sendCommand(id, par = null) {
		let socket = new HttpSocket(this.ip);
		socket.connect((fd) => {
			this.wifi.bindSocketToAdapter(this.wifiAdapter, fd);
		});
		let r = socket.request("/?custom=1&cmd=" + String(id));
		let xml = parseXml(r[1]);
		socket.close();
		return xml;
	}
	sendCommandHandleChunks(path, size, callback) {
		let socket = new HttpSocket(this.ip);
		socket.connect((fd) => {
			this.wifi.bindSocketToAdapter(this.wifiAdapter, fd);
		});
		let resp = socket.requestChunks(path, size, callback);
		socket.close();
	}
	onTryConnectWiFi(wifiAdapter, saved, job) {
		this.wifiAdapter = wifiAdapter;
		let resp = this.sendCommand(Commands.REMOVE_LAST_USER);

		resp = this.sendCommand(Commands.FIRMWARE_VERSION);

		this.setProperty(Module.PAK_PROP_NAME, "Viofo " + resp["Function"]["String"].split(" ")[0]);

		resp = this.sendCommand(Commands.GET_CARD_STATUS);

		if (resp["Function"]["Value"] == 1) {
			resp = this.sendCommand(Commands.GET_FILE_LIST);
			this.fileList = resp["LIST"]["ALLFile"];
			this.setStorageInfo("sdcard", this.fileList.length, Module.PAK_NEWEST_FIRST);
			for (let i in this.fileList) {
				this.addFileMetadata({
					"storageName": "sdcard",
					"index": i
				}, {
					"filename": this.fileList[i]["File"]["NAME"],
					"mimeType": "image/quicktime"
				});
			}
			this.setScreenSupported(Module.PAK_SCREEN_FILE_GALLERY, true);
			this.setScreenSupported(Module.PAK_SCREEN_FILE_VIEWER, true);
		}
	}
	onRequestFileThumbnail(job, handle) {
		Module.globalLog("onRequestFileThumbnail");
		let thumbBuf = new Uint8Array();
		let normalPath = this.fileList[handle.index]["File"]["FPATH"].substring(2).replaceAll("\\", "/") + "?custom=1&cmd=4001";
		let r = this.sendCommandHandleChunks(normalPath, 50000, (buf) => {
			let temp = new Uint8Array(thumbBuf.length + buf.length);
			temp.set(thumbBuf, 0);
			temp.set(buf, thumbBuf.length);
			thumbBuf = temp;
		});
		this.addFileThumbnail(handle, thumbBuf.buffer);
	}
	onRunTest() {
		test(this, parseXml);
	}
};
Module.export(new Veement());
