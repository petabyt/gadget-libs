// Implements Nothing CMF proprietary protocol
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <runtime.h>
#include <bluetooth.h>

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

int main(void) {
	struct PakBt *bt = pak_bt_get_context();
	struct PakBtAdapter adapter;
	pak_bt_get_adapter(bt, &adapter, 0);

	struct PakBtDevice device;
	if (pak_bt_get_paired_device(bt, &adapter, &device, 0) != 0) {
		printf("No paired device\n");
		return -1;
	}

	const char *nothing_service_uuid = "aeac4a03-dff5-498f-843a-34487cf133eb";

	struct PakBtSocket *conn;
	int rc = pak_bt_connect_to_service_channel(bt, &device, nothing_service_uuid, &conn);
	if (rc == 0) {
		struct Request *req = malloc(100);
		struct Response *resp = malloc(100);
		req->magic = 0x55;
		req->transaction_counter = 0x5120 | 0x40;	
		req->command = 0xc007;
		req->payload_length = 0;
		req->message_counter = 0x0;

		rc = pak_bt_write(conn, req, sizeof(struct Request));
		printf("out %d\n", rc);
		rc = pak_bt_read(conn, resp, 100);
		printf("in %d\n", rc);

		struct BatteryStat *stat = (struct BatteryStat *)resp->payload;
		for (int i = 0; i < stat->n_bats; i++) {
			printf("Battery: %d: %u%%\n", stat->batteries[i].id, stat->batteries[i].status & 0x7f);
		}

		free(resp); free(req);
	}
	pak_bt_close_socket(conn);

	pak_bt_unref_adapter(bt, &adapter);
	pak_bt_unref_device(bt, &device);

	return 0;
}

static int init(struct Module *mod) {
	pak_debug_log(mod, "cmf-nothing init");
	pak_rt_set_screen_supported(mod, SCREEN_FILE_GALLERY, 1);
	pak_rt_set_screen_supported(mod, SCREEN_FILE_VIEWER, 1);
	pak_rt_set_tick_interval(mod, 1000 * 1000);
	return 0;
}

static int on_find_connection(struct Module *mod, int job) {
	struct PakBtAdapter adapter;
	pak_bt_get_adapter(mod->bt, &adapter, 0);
	struct PakBtDevice device;
	pak_bt_get_saved_device(mod->bt, &adapter, &device, 1);

	pak_debug_log(mod, "'%s'", device.name);

	struct PakBtSocket *conn;
	pak_bt_connect_to_service_channel(mod->bt, &device, "uuid", &conn);

	pak_bt_unref_adapter(mod->bt, &adapter);
	return -1;
}

static int on_idle_tick(struct Module *mod, unsigned int us_since_last_tick) {
	return 0;
}

static int on_disconnect(struct Module *mod) {
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

