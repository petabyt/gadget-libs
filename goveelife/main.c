// Based on https://github.com/Heckie75/govee-h5075-thermo-hygrometer/blob/main/API.md
#include <stdio.h>
#include <byteswap.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <runtime.h>
#include <bluetooth.h>

/*
service: 00001800-0000-1000-8000-00805f9b34fb
	chr: 00002a00-0000-1000-8000-00805f9b34fb // name
	chr: 00002a01-0000-1000-8000-00805f9b34fb
	chr: 00002a04-0000-1000-8000-00805f9b34fb
	chr: 00002ac9-0000-1000-8000-00805f9b34fb
service: 00001801-0000-1000-8000-00805f9b34fb
	chr: 00002a05-0000-1000-8000-00805f9b34fb
service: 494e5445-4c4c-495f-524f-434b535f4857
	chr: 494e5445-4c4c-495f-524f-434b535f2011 // Command
	chr: 494e5445-4c4c-495f-524f-434b535f2012 // DataControl
	chr: 494e5445-4c4c-495f-524f-434b535f2013 // DataResult
service: 02f00000-0000-0000-0000-00000000fe00
	chr: 02f00000-0000-0000-0000-00000000ff03
	chr: 02f00000-0000-0000-0000-00000000ff02
	chr: 02f00000-0000-0000-0000-00000000ff00
	chr: 02f00000-0000-0000-0000-00000000ff01
*/

struct ModulePriv {
	struct PakBtDevice *device;
	struct PakGattService *govee_service;
};

#define GOVEE_SERVICE "494e5445-4c4c-495f-524f-434b535f4857"
#define GOVEE_COMMAND "494e5445-4c4c-495f-524f-434b535f2011"
#define GOVEE_DATA_CONTROL "494e5445-4c4c-495f-524f-434b535f2012"
#define GOVEE_DATA_RESULT "494e5445-4c4c-495f-524f-434b535f2013"
#define GENERIC_ACCESS_SERVICE "00001800-0000-1000-8000-00805f9b34fb"
#define DEVICE_NAME "00002a00-0000-1000-8000-00805f9b34fb"

struct __attribute__((packed)) GoveeCmd {
	union U {
		uint16_t cmd;
		uint8_t buf[20];
		struct __attribute__((packed)) FwVer {
			uint16_t cmd;
			char value[17];
		}fwver;
		struct __attribute__((packed)) Measurement {
			uint16_t cmd;
			uint16_t temp;
			uint16_t humidity;
			uint8_t battery;
		}measurement;
		struct __attribute__((packed)) HistoricalDataReq {
			uint16_t cmd;
			uint16_t minutes_start;
			uint16_t minutes_end;
		}historicalreq;
		struct __attribute__((packed)) HistoricalDataResp {
			uint16_t minute;
			uint8_t records[3][6];
		}historicaldata;
	}u;
};

#define CMD_REQUEST_HISTORICAL_DATA 0x3301
#define CMD_REQUEST_MEASUREMENT 0xaa01
#define CMD_REQUEST_BATTERY 0xaa08
#define CMD_REQUEST_HUMIDITY_AND_TEMP 0xaa0a
#define CMD_REQUEST_MAC_ADDRESS 0xaa0c
#define CMD_REQUEST_HARDWARE_VER 0xaa0d
#define CMD_REQUEST_FIRMWARE_VER 0xaa0e

static void decode_measurement(const uint8_t record[3], int *humid, int *temp) {
	int v = record[0] | (record[1] << 8) | (record[2] << 16);
	(*temp) = v / 1000;
	(*humid) = v % 1000;
}

static int init(struct Module *mod) {
	pak_rt_set_tick_interval(mod, 3000 * 1000);
	mod->priv = calloc(1, sizeof(struct ModulePriv));
	return 0;
}

static int send_command(struct Module *mod, struct GoveeCmd *cmd) {
	const char *uuid = NULL;
	switch (cmd->u.cmd) {
		case CMD_REQUEST_HISTORICAL_DATA:
		case CMD_REQUEST_MEASUREMENT:
			uuid = GOVEE_DATA_CONTROL; break;
		default: uuid = GOVEE_COMMAND; break;
	}

	struct PakGattCharacteristic *chr = pak_bt_get_gatt_characteristic_uuid(mod->bt, mod->priv->govee_service, uuid);
	if (chr == NULL) {
		pak_bt_unref_gatt_service(mod->bt, mod->priv->govee_service);
		return PAK_ERR_UNSUPPORTED;
	}

	cmd->u.cmd = bswap_16(cmd->u.cmd);

	cmd->u.buf[19] = 0x0;
	for (int i = 0; i < 19; i++) {
		cmd->u.buf[19] ^= cmd->u.buf[i];
	}

	pak_bt_set_watching_characteristic(mod->bt, chr, 1);
	if (pak_bt_write_characteristic(mod->bt, chr, (uint8_t *)cmd, 20, 1)) {
		return PAK_ERR_IO;
	}
	pak_bt_watch_characteristic(mod->bt, chr, 100);
	pak_bt_set_watching_characteristic(mod->bt, chr, 0);

	unsigned int len = pak_bt_read_characteristic_cached_value(mod->bt, chr, (uint8_t *)cmd, 20);
	if (len != 20) pak_debug_log(mod, "Result is not 20");

#if 0
	char buf[200] = {0};
	for (int i = 0; i < len; i++) {
		sprintf(buf + strlen(buf), "%02x ", cmd->u.buf[i]);
	}
	pak_global_log("read: %s", buf);
#endif

	pak_bt_unref_gatt_characteristic(mod->bt, chr);
	return 0;
}

static int on_try_connect_bluetooth(struct Module *mod, struct PakBtDevice *device, struct PakSavedConnection *saved, int job) {
	mod->priv->device = device;
	if (pak_bt_device_connect(mod->bt, device)) {
		pak_bt_unref_device(mod->bt, device);
		return PAK_ERR_NO_CONNECTION;
	}

	mod->priv->govee_service = pak_bt_get_gatt_service_uuid(mod->bt, mod->priv->device, GOVEE_SERVICE);
	if (mod->priv->govee_service == NULL) {
		return PAK_ERR_UNSUPPORTED;
	}

	struct GoveeCmd cmd = {0};
	cmd.u.cmd = CMD_REQUEST_FIRMWARE_VER;
	send_command(mod, &cmd);
	pak_rt_set_session_property(mod, PAK_PROP_FW_VER, cmd.u.fwver.value);

	{
		struct PakGattService *service = pak_bt_get_gatt_service_uuid(mod->bt, mod->priv->device, GENERIC_ACCESS_SERVICE);
		if (service == NULL) {
			return PAK_ERR_UNSUPPORTED;
		}
		struct PakGattCharacteristic *chr = pak_bt_get_gatt_characteristic_uuid(mod->bt, service, DEVICE_NAME);
		if (chr == NULL) {
			pak_bt_unref_gatt_service(mod->bt, service);
			return PAK_ERR_UNSUPPORTED;
		}
		pak_bt_read_characteristic(mod->bt, chr, 1);
		char buffer[20];
		buffer[pak_bt_read_characteristic_cached_value(mod->bt, chr, (uint8_t *)buffer, 20)] = '\0';
		pak_rt_set_session_property(mod, PAK_PROP_NAME, buffer);
		pak_bt_unref_gatt_characteristic(mod->bt, chr);
		pak_bt_unref_gatt_service(mod->bt, service);
	}

	{
		struct PakGattCharacteristic *chr = pak_bt_get_gatt_characteristic_uuid(mod->bt, mod->priv->govee_service, GOVEE_DATA_RESULT);
		if (chr == NULL) {
			pak_bt_unref_gatt_service(mod->bt, mod->priv->govee_service);
			return PAK_ERR_UNSUPPORTED;
		}

		pak_bt_set_watching_characteristic(mod->bt, chr, 1);

		send_command(mod, &(struct GoveeCmd){
			.u.historicalreq = {CMD_REQUEST_HISTORICAL_DATA, bswap_16(60), bswap_16(0)}
		});

		int humitity[100];
		int temp[100];
		int i;
		for (i = 0; pak_bt_watch_characteristic(mod->bt, chr, 1000) == 0; i++) {
			unsigned int len = pak_bt_read_characteristic_cached_value(mod->bt, chr, (uint8_t *)&cmd, 20);
			if (i < 100 && len == 20) {
				decode_measurement(cmd.u.historicaldata.records[0], &humitity[i], &temp[i]);
			} else break;
		}

		pak_rt_set_dashboard_pane(mod, &(struct PakUserSetting) {
			.name = "temp",
			.title = "Temperature",
			.type = PAK_GRAPH,
			.u.graphv = {
				.points = temp,
				.n_points = i,
			}
		});
		pak_rt_set_dashboard_pane(mod, &(struct PakUserSetting) {
			.name = "humid",
			.title = "Humidity",
			.type = PAK_GRAPH,
			.u.graphv = {
				.points = humitity,
				.n_points = i,
			}
		});

		pak_bt_set_watching_characteristic(mod->bt, chr, 0);
	}

	return 0;
}

static int on_idle_tick(struct Module *mod, unsigned int us_since_last_tick) {
	pak_bt_device_update(mod->bt, mod->priv->device);
	if (!mod->priv->device->is_connected) return -1;

	struct GoveeCmd cmd = {0};
	cmd.u.cmd = CMD_REQUEST_MEASUREMENT;
	send_command(mod, &cmd);
	pak_rt_set_session_property_int(mod, PAK_PROP_BATTERY_MAIN, (int)cmd.u.measurement.battery);
	pak_rt_set_session_property_int(mod, "temperature", (int)bswap_16(cmd.u.measurement.temp));
	pak_rt_set_session_property_int(mod, "humidity", (int)bswap_16(cmd.u.measurement.humidity));

	return 0;
}

static int on_disconnect(struct Module *mod) {
	pak_bt_device_disconnect(mod->bt, mod->priv->device);
	pak_bt_unref_device(mod->bt, mod->priv->device);
	pak_bt_unref_gatt_service(mod->bt, mod->priv->govee_service);
	return 0;
}

int get_module_goveelife(struct Module *mod) {
	mod->init = init;
	mod->on_try_connect_bluetooth = on_try_connect_bluetooth;
	mod->on_idle_tick = on_idle_tick;
	mod->on_disconnect = on_disconnect;
	return 0;
}
__attribute__((weak)) int get_module(struct Module *mod) { return get_module_goveelife(mod); }