// Implements Nothing CMF proprietary protocol
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <runtime.h>
#include <bluetooth.h>
#include <byteswap.h>

struct ModulePriv {
	struct PakBtSocket *conn;
	struct PakBtDevice *dev;
	uint8_t message_counter;
};

struct __attribute__((packed)) Request {
	uint8_t magic;
	uint16_t transaction_counter;
	uint16_t command;
	uint16_t payload_length;
	uint8_t message_counter;
	uint8_t payload[];
};

struct __attribute__((packed)) Response {
	uint8_t magic;
	uint16_t control;
	uint16_t command;
	uint16_t payload_length;
	uint8_t res0;
	uint8_t payload[];
};

struct __attribute__((packed)) BatteryStat {
	uint8_t n_bats;
	struct BatteryStatItem {
		uint8_t id;
		uint8_t status;
	}batteries[];
};

#define GET_BATTERY 0xc007
#define GET_FW_VER 0xc042
#define SET_EAR_DETECTION 0xf004
#define SET_ULTRA_BASS 0xf051
#define GET_ULTRA_BASS 0xc04e
#define SET_NOISE_CANCELLATION 0xf00f
#define GET_NOISE_CANCELLATION 0xc01e
#define SET_EQUALIZER_PRESET 0xf01d
#define GET_EQUALIZER_PRESET 0xc050

#define GET_EQ 0xc01f

// https://developers.google.com/nearby/fast-pair/specifications/extensions/deviceinformation
static int google_fastpair(struct PakBt *bt, struct PakBtDevice *dev) {
	struct PakBtSocket *sock = pak_bt_connect_to_service_channel(bt, dev, "df21fe2c-2515-4fdb-8886-f12c4d67927c");

	struct FastPairPacket {
		uint8_t msggroup;
		uint8_t msgcode;
		uint16_t datalen;
		uint8_t data[10];
	};

	struct FastPairPacket send_android_info = {
		.msggroup = 3,
		.msgcode = 8,
		.datalen = bswap_16(2),
		.data = {0x1, 0x24}
	};

	// < 0000   03 01 00 03 2f 45 f5
	// < 0000   03 02 00 06 47 c0 43 c7 c7 e5
	// < 0000   03 03 00 03 64 64 37
	// > 0000   03 08 00 02 01 24
	return 0;
}

static uint16_t crc16_modbus(const uint8_t *data, unsigned int len) {
    uint16_t crc = 0xFFFF;

    for (unsigned int i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i];

        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void hexdump(void *data, unsigned int length) {
	for (unsigned int i = 0 ; i < length; i++) {
		printf("%02x ", ((uint8_t *)data)[i]);
	}
	printf("\n");
}

static int transaction(struct ModulePriv *priv, void *resp, unsigned int max_read, uint16_t cmd, void *payload, unsigned int payload_length) {
	struct PakBtSocket *conn = priv->conn;
	char buffer[500];
	struct Request *req = (struct Request *)buffer;
	req->magic = 0x55;
	req->transaction_counter = 0x0160;
	req->command = cmd;
	req->payload_length = payload_length;
	req->message_counter = priv->message_counter++;
	if (payload != NULL) memcpy(req->payload, payload, payload_length);
	((uint16_t *)&req->payload[payload_length])[0] = crc16_modbus((uint8_t *)req, sizeof(struct Request) + payload_length);

	int rc = pak_bt_write(conn, req, sizeof(struct Request) + payload_length + 2);
	if (rc < 0) {
		pak_global_log("pak_bt_write: %d", rc);
		return -1;
	}
	rc = pak_bt_read(conn, resp, max_read);
	pak_global_log("pak_bt_write: %d", rc);
	if (rc < 0) return -1;
	return 0;
}

static int update_battery(struct PakModule *mod) {
	char buffer[500];
	int rc = transaction(mod->priv, buffer, sizeof(buffer), GET_BATTERY, NULL, 0);
	if (rc < 0) {
		pak_debug_log(mod, "Transaction failed");
		return -1;
	} else {
		struct Response *resp = (struct Response *)buffer;
		struct BatteryStat *stat = (struct BatteryStat *)resp->payload;
		for (int i = 0; i < stat->n_bats; i++) {
			int percent = (int)(stat->batteries[i].status & 0x7f);
			switch (stat->batteries[i].id) {
			case 4:
				pak_rt_set_session_property_int(mod, PAK_PROP_BATTERY_MAIN, percent);
				break;
			case 2:
				pak_rt_set_session_property_int(mod, PAK_PROP_BATTERY_LEFT, percent);
				break;
			case 3:
				pak_rt_set_session_property_int(mod, PAK_PROP_BATTERY_RIGHT, percent);
				break;
			}
			//pak_debug_log(mod, "Battery: %d: %u%%\n", stat->batteries[i].id, stat->batteries[i].status & 0x7f);
		}
	}
	return 0;
}

static int init(struct PakModule *mod) {
	pak_rt_set_tick_interval(mod, 1000 * 1000);
	mod->priv = calloc(1, sizeof(struct ModulePriv));

	pak_rt_set_dashboard_pane(mod, &(struct PakWidget) {
		.name = "lowlagmode",
		.title = "Low Lag Mode",
		.type = PAK_BOOLEAN,
		.u.boolv.v = 0,
	});

	const char *options[] = {"Low", "Mid", "High", "Adaptive", "Transparency Mode", "Noise cancellation", "Off", NULL};

	pak_rt_set_dashboard_pane(mod, &(struct PakWidget) {
		.name = "noisecancellation",
		.title = "Noise Cancellation",
		.type = PAK_DROPDOWN,
		.u.dropdownv = {
			.index_value = 0,
			.list = options
		}
	});

	pak_rt_set_dashboard_pane(mod, &(struct PakWidget) {
		.name = "in-ear-detection",
		.title = "In-ear detection",
		.type = PAK_BOOLEAN,
		.u.boolv.v = 0,
	});

	pak_rt_set_dashboard_pane(mod, &(struct PakWidget) {
		.name = "ultrabass",
		.title = "Ultra bass",
		.type = PAK_BOOLEAN,
		.u.boolv.v = 0,
	});

	return 0;
}

static int on_try_connect_bluetooth(struct PakModule *mod, struct PakBtDevice *dev, struct PakSavedConnection *saved, int job) {
	if (!dev->is_bonded) {
		int rc = pak_bt_device_connect(mod->bt, dev);
		if (rc) {
			pak_debug_log(mod, "Failed to connect");
			return rc;
		}

		rc = pak_bt_device_create_bond(mod->bt, dev);
		if (rc) {
			pak_debug_log(mod, "Failed to create bond");
			return rc;
		}
	}

	struct PakBtSocket *conn = pak_bt_connect_to_service_channel(mod->bt, dev, "aeac4a03-dff5-498f-843a-34487cf133eb");
	if (conn == NULL) {
		pak_debug_log(mod, "pak_bt_connect_to_service_channel");
		return -1;
	}
	mod->priv->conn = conn;
	mod->priv->dev = dev;

	update_battery(mod);

	pak_rt_set_session_property(mod, PAK_PROP_NAME, dev->name);
	pak_rt_set_session_property(mod, PAK_PROP_FW_VER, "1.0.1.7.4");
	
	return 0;
}

static int on_idle_tick(struct PakModule *mod, unsigned int us_since_last_tick) {
	pak_bt_device_update(mod->bt, mod->priv->dev);
	if (!mod->priv->dev->is_connected) pak_rt_fatal_error(mod, "Disconnected");
	return 0;
}

static int on_disconnect(struct PakModule *mod) {
	pak_bt_close_socket(mod->priv->conn);
	pak_bt_unref_device(mod->bt, mod->priv->dev);
	return 0;
}

static int on_switch_screen(struct PakModule *mod, int old_screen, int new_screen, int job) {
	return 0;
}

static int on_prop_changed(struct PakModule *mod, int job, struct PakWidget *prop) {
	char buf[64];
	if (!strcmp(prop->name, "ultrabass")) {
		transaction(mod->priv, buf, sizeof(buf), SET_ULTRA_BASS, (uint8_t[]){
			prop->u.boolv.v ? 1 : 0, // enabled
			0x6 // bass level
		}, 2);
	}
	pak_global_log("on_prop_changed %s", prop->name);
	return 0;
}

int get_module(struct PakModule *mod) {
	mod->init = init;
	mod->on_try_connect_bluetooth = on_try_connect_bluetooth;
	mod->on_idle_tick = on_idle_tick;
	mod->on_disconnect = on_disconnect;
	mod->on_switch_screen = on_switch_screen;
	mod->on_setting_changed = on_prop_changed;
	return 0;
}
