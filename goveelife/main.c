// Based on https://github.com/Heckie75/govee-h5075-thermo-hygrometer/blob/main/API.md
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <runtime.h>
#include <bluetooth.h>

/*
service: 00001800-0000-1000-8000-00805f9b34fb
	chr: 00002a00-0000-1000-8000-00805f9b34fb
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
	struct PakBtDevice device;
};


#define GOVEE_SERVICE "494e5445-4c4c-495f-524f-434b535f4857"
#define GOVEE_COMMAND "494e5445-4c4c-495f-524f-434b535f2011"
#define GOVEE_DATA_CONTROL "494e5445-4c4c-495f-524f-434b535f2012"
#define GOVEE_DATA_RESULT "494e5445-4c4c-495f-524f-434b535f2013"

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
	}u;
};

#define CMD_REQUEST_MEASUREMENT 0x01aa
#define CMD_REQUEST_BATTERY 0x08aa
#define CMD_REQUEST_HUMIDITY_AND_TEMP 0x0aaa
#define CMD_REQUEST_MAC_ADDRESS 0x0caa
#define CMD_REQUEST_HARDWARE_VER 0x0daa
#define CMD_REQUEST_FIRMWARE_VER 0x0eaa

static int init(struct Module *mod) {
	pak_rt_set_tick_interval(mod, 3000 * 1000);
	mod->priv = calloc(1, sizeof(struct ModulePriv));
	return 0;
}

static int read_char(struct Module *mod, struct PakGattService *service, const char *uuid) {
	struct PakGattCharacteristic chr;
	if (pak_bt_get_gatt_characteristic_uuid(mod->bt, service, &chr, uuid)) {
		return PAK_ERR_UNSUPPORTED;
	}
	if (pak_bt_read_characteristic(mod->bt, &chr, 1)) {
		pak_bt_unref_gatt_characteristic(mod->bt, &chr);
		return PAK_ERR_IO;
	}
	pak_bt_unref_gatt_characteristic(mod->bt, &chr);
	return 0;
}

static int send_command(struct Module *mod, struct GoveeCmd *cmd) {
	struct PakGattService service;
	if (pak_bt_get_gatt_service_uuid(mod->bt, &mod->priv->device, &service, GOVEE_SERVICE)) {
		return PAK_ERR_UNSUPPORTED;
	}

	const char *uuid = NULL;
	switch (cmd->u.cmd) {
		case CMD_REQUEST_MEASUREMENT: uuid = GOVEE_DATA_CONTROL; break;
		default: uuid = GOVEE_COMMAND; break;
	}

	struct PakGattCharacteristic chr;
	if (pak_bt_get_gatt_characteristic_uuid(mod->bt, &service, &chr, uuid)) {
		pak_bt_unref_gatt_service(mod->bt, &service);
		return PAK_ERR_UNSUPPORTED;
	}

	cmd->u.buf[19] = 0x0;
	for (int i = 0; i < 19; i++) {
		cmd->u.buf[19] ^= cmd->u.buf[i];
	}

	pak_bt_set_watching_characteristic(mod->bt, &chr, 1);
	if (pak_bt_write_characteristic(mod->bt, &chr, (uint8_t *)cmd, 20, 1)) {
		return PAK_ERR_IO;
	}
	pak_bt_watch_characteristic(mod->bt, &chr, 100);
	pak_bt_set_watching_characteristic(mod->bt, &chr, 0);

	unsigned int len = pak_bt_read_characteristic_cached_value(mod->bt, &chr, (uint8_t *)cmd, 20);
	if (len != 20) {
		pak_debug_log(mod, "Result is not 20");
	}

#if 1
	char buf[200] = {0};
	for (int i = 0; i < len; i++) {
		sprintf(buf + strlen(buf), "%02x ", cmd->u.buf[i]);
	}
	pak_global_log("read: %s", buf);
#endif

	pak_bt_unref_gatt_characteristic(mod->bt, &chr);
	pak_bt_unref_gatt_service(mod->bt, &service);
	return 0;
}

static int on_try_connect_bluetooth(struct Module *mod, struct PakBtDevice *device, int job) {
	mod->priv->device = *device;
	if (pak_bt_device_connect(mod->bt, device)) {
		pak_bt_unref_device(mod->bt, device);
		return PAK_ERR_NO_CONNECTION;
	}

	struct GoveeCmd cmd = {0};
	cmd.u.cmd = CMD_REQUEST_FIRMWARE_VER;
	send_command(mod, &cmd);
	pak_rt_set_session_property(mod, PAK_PROP_FW_VER, cmd.u.fwver.value);

	return 0;
}

static int on_idle_tick(struct Module *mod, unsigned int us_since_last_tick) {
	pak_bt_device_update(mod->bt, &mod->priv->device);
	if (!mod->priv->device.is_connected) return -1;

	struct GoveeCmd cmd = {0};
	cmd.u.cmd = CMD_REQUEST_MEASUREMENT;
	send_command(mod, &cmd);
	pak_rt_set_session_property_int(mod, PAK_PROP_BATTERY_MAIN, (int)cmd.u.measurement.battery);

	return 0;
}

static int on_disconnect(struct Module *mod) {
	pak_bt_device_disconnect(mod->bt, &mod->priv->device);
	pak_bt_unref_device(mod->bt, &mod->priv->device);
	return 0;
}

int get_module_goveelife(struct Module *mod) {
	mod->init = init;
	mod->on_try_connect_bluetooth = on_try_connect_bluetooth;
	mod->on_idle_tick = on_idle_tick;
	mod->on_disconnect = on_disconnect;
	return 0;
}

