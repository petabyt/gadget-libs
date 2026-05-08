// Implements Nothing CMF proprietary protocol
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <runtime.h>
#include <bluetooth.h>

struct ModulePriv {
	struct PakBtSocket *conn;
};

struct __attribute__((packed)) Request {
	uint8_t magic;
	uint16_t transaction_counter;
	uint16_t command;
	uint64_t payload_length;
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

void hexdump(void *data, unsigned int length) {
	for (unsigned int i = 0 ; i < length; i++) {
		printf("%02x ", ((uint8_t *)data)[i]);
	}
	printf("\n");
}

static int transaction(struct ModulePriv *priv, void *resp, unsigned int length, uint16_t cmd) {
	struct PakBtSocket *conn = priv->conn;
	char buffer[500];
	struct Request *req = (struct Request *)buffer;
	req->magic = 0x55;
	req->transaction_counter = 0x5120 | 0x40;
	req->command = cmd;
	req->payload_length = 0;
	req->message_counter = 0x0;

	int rc = pak_bt_write(conn, req, sizeof(struct Request));
	if (rc < 0) return -1;
	rc = pak_bt_read(conn, resp, length);
	if (rc < 0) return -1;
	return 0;
}

static int update_battery(struct Module *mod) {
	char buffer[500];
	int rc = transaction(mod->priv, buffer, sizeof(buffer), 0xc007);
	if (rc < 0) {
		pak_debug_log(mod, "Transaction failed");
		return -1;
	} else {
		struct Response *resp = (struct Response *)buffer;
		struct BatteryStat *stat = (struct BatteryStat *)resp->payload;
		for (int i = 0; i < stat->n_bats; i++) {
			char buf[16];
			sprintf(buf, "%u", stat->batteries[i].status & 0x7f);
			switch (stat->batteries[i].id) {
			case 4:
				pak_rt_set_session_property(mod, PAK_PROP_BATTERY_MAIN, buf);
				break;
			case 2:
				pak_rt_set_session_property(mod, PAK_PROP_BATTERY_LEFT, buf);
				break;
			case 3:
				pak_rt_set_session_property(mod, PAK_PROP_BATTERY_RIGHT, buf);
				break;
			}
			//pak_debug_log(mod, "Battery: %d: %u%%\n", stat->batteries[i].id, stat->batteries[i].status & 0x7f);
		}
	}
	return 0;
}

static int init(struct Module *mod) {
	pak_debug_log(mod, "cmf-nothing init");
	pak_rt_set_tick_interval(mod, 1000 * 1000);
	mod->priv = calloc(1, sizeof(struct ModulePriv));

	pak_rt_add_user_setting(mod, &(struct PakUserSetting){
		.name = "lowlagmode",
		.title = "Low Lag Mode",
		.type = PAK_BOOLEAN,
		.u.boolv.v = 0,
	});

	pak_rt_add_user_setting(mod, &(struct PakUserSetting){
		.name = "in-ear-detection",
		.title = "In-ear detection",
		.type = PAK_BOOLEAN,
		.u.boolv.v = 0,
	});

	pak_rt_add_user_setting(mod, &(struct PakUserSetting){
		.name = "ultrabass",
		.title = "Ultra bass",
		.type = PAK_BOOLEAN,
		.u.boolv.v = 0,
	});

	return 0;
}

static int on_find_connection(struct Module *mod, int job) {
	struct PakBtAdapter adapter;
	pak_bt_get_adapter(mod->bt, &adapter, 0);
	struct PakBtDevice device;
	int rc = pak_bt_get_device(mod->bt, &adapter, &device, 0, PAK_FILTER_CONNECTED);
	if (rc) return PAK_ERR_NO_CONNECTION;

	pak_debug_log(mod, "Connecting to '%s'", device.name);

	struct PakBtSocket *conn;
	rc = pak_bt_connect_to_service_channel(mod->bt, &device, "aeac4a03-dff5-498f-843a-34487cf133eb", &conn);
	if (rc) {
		pak_debug_log(mod, "pak_bt_connect_to_service_channel");
		return -1;
	}
	mod->priv->conn = conn;

	update_battery(mod);

	pak_rt_set_session_property(mod, PAK_PROP_NAME, "CMF Buds Pro 2");
	pak_rt_set_session_property(mod, PAK_PROP_FW_VER, "1.0.1.7.4");

	pak_bt_unref_adapter(mod->bt, &adapter);
	return 0;
}

static int on_idle_tick(struct Module *mod, unsigned int us_since_last_tick) {
	return 0;
}

static int on_disconnect(struct Module *mod) {
	pak_bt_close_socket(mod->priv->conn);
	return 0;
}

static int on_switch_screen(struct Module *mod, int old_screen, int new_screen, int job) {
	return 0;
}

int get_module_cmfnothingaudio(struct Module *mod) {
	mod->init = init;
	mod->on_find_connection = on_find_connection;
	mod->on_idle_tick = on_idle_tick;
	mod->on_disconnect = on_disconnect;
	mod->on_switch_screen = on_switch_screen;
	return 0;
}
__attribute__((weak)) int get_module(struct Module *mod) { return get_module_cmfnothingaudio(mod); }
