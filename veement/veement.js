import { WiFi } from "pak:wifi";
import { Module } from "pak:runtime";
import { HttpSocket } from "pak:http";

class Veement extends Module {
	userAgent = "PakUserAgent";
	socket = null;
	loopFiles = [];
	constructor() {
		super()
	}
	onTryConnectWiFi(wifiAdapter, saved, job) {
		let wifi = this.wifi;
		this.socket = new HttpSocket("192.168.169.1");

		this.socket.connect(function(fd) {
			wifi.bindSocketToAdapter(wifiAdapter, fd);
		});

		this.addWidget({
			"name": "format",
			"title": "Format SD Card",
			"type": "button"
		});
		this.addWidget({
			"name": "audio",
			"title": "Dashcam Speakers",
			"type": "bool",
			"value": true,
		});
	}
	onSwitchScreen(prev, to, job) {
		if (to == Module.PAK_SCREEN_DASHBOARD) {
			try {
				let d = new Date();
				let timestamp =
					d.getFullYear() +
					String(d.getMonth() + 1).padStart(2, "0") +
					String(d.getDate()).padStart(2, "0") +
					String(d.getHours()).padStart(2, "0") +
					String(d.getMinutes()).padStart(2, "0") +
					String(d.getSeconds()).padStart(2, "0");
				this.httpRequest("/app/settimezone?timezone=" + String(-5));
				this.httpRequest("/app/setsystime?date=" + String(timestamp));

				let resp = this.httpRequest("/app/getproductinfo");
				this.setProperty(Module.PAK_PROP_NAME, `${resp.info.company} ${resp.info.model}`);
				resp = this.httpRequest("/app/getdeviceattr");
				this.setProperty(Module.PAK_PROP_FW_VER, resp.info.softver);

				resp = this.httpRequest("/app/getfilelist?folder=loop&start=0&end=99");
				if (resp.result == 0) {
					this.setScreenSupported(Module.PAK_SCREEN_FILE_GALLERY, true);
					this.setScreenSupported(Module.PAK_SCREEN_FILE_VIEWER, true);

					this.loopFiles = resp.info[0].files
					this.setStorageInfo("sdcard", resp.info[0].count, Module.PAK_NEWEST_FIRST);
					for (let i = 0; i < this.loopFiles.length; i++) {
						this.addFileMetadata({
							"storageName": "sdcard",
							"index": i
						}, {
							"filename": this.loopFiles[i].name.substring(22, 30),
							"mimeType": "image/quicktime"
						});
					}
				}
			} catch (e) {
				this.fatalError(e);
				throw e;
			}
		}
	}
	onRequestFileThumbnail(job, handle) {
		Module.globalLog("onRequestFileThumbnail");
		let metadata = this.loopFiles[handle.index];
		let thumbBuf = new Uint8Array();
		let r = this.socket.requestChunks("/app/getthumbnail?file=" + metadata.name, 50000, (buf) => {
			let temp = new Uint8Array(thumbBuf.length + buf.length);
			temp.set(thumbBuf, 0);
			temp.set(buf, thumbBuf.length);
			thumbBuf = temp;
		});
		this.addFileThumbnail(handle, thumbBuf.buffer);
	}
	onDisconnect() {
		this.debugLog("Closing socket");
		this.socket.close();
	}
	onIdleTick(sinceLastTick) {
		this.debugLog("Tick");
	}
	httpRequest(path) {
		let r = this.socket.request(path);
		return JSON.parse(r[1]);
	}
};
Module.export(new Veement());
