#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <runtime.h>
#include <runtime_ext.h>
#include <wifi.h>

#include "dummyjpg.h"

struct ModulePriv {
	int x;
};

static int on_find_connection(struct Module *mod, int job) {
	pak_debug_log(mod, "Connection established");
	return 0;
}

static int init(struct Module *mod) {
	pak_debug_log(mod, "Hello from dummy module");
	mod->priv = (struct ModulePriv *)malloc(sizeof(struct ModulePriv));
	pak_rt_set_session_property(mod, PAK_PROP_NAME, "Dummy Device");
	pak_rt_set_session_property(mod, PAK_PROP_FW_VER, "v1.2.3");

	pak_rt_add_user_setting(mod, &(struct PakUserSetting){
		.name = "flipper",
		.title = "Flipper",
		.type = PAK_BOOLEAN,
		.u.boolv.v = 1,
	});

	pak_rt_set_screen_supported(mod, SCREEN_DASHBOARD, 1);
	pak_rt_set_screen_supported(mod, SCREEN_FILE_GALLERY, 1);
	pak_rt_set_screen_supported(mod, SCREEN_FILE_VIEWER, 1);

	pak_rt_set_storage_info(mod, "sdcard", 10, PAK_NEWEST_FIRST);

	struct ExifParser metadata;
	int rc = exif_start_raw(&metadata, _dummy_jpeg_jpg, sizeof(_dummy_jpeg_jpg), NULL, NULL);
	if (rc == 0 && metadata.thumb_of != 0) {
		pak_rt_add_file_thumbnail(mod, &(struct FileHandle){
			.storage_name = "sdcard",
			.index_in_view = 0,
			.filename = NULL,
		}, _dummy_jpeg_jpg + metadata.thumb_of, metadata.thumb_size);
	}
	pak_rt_add_file_metadata(mod, &(struct FileHandle){
		.storage_name = "sdcard",
		.index_in_view = 1,
		.filename = NULL,
	}, &(struct FileMetadata){
		.filename = "ASD.JPG",
		.mime_type = "image/jpeg",
	});

	return 0;
}

static int on_try_connect_wifi(struct Module *mod, struct PakWiFiAdapter *handle, int job) {
	return 0;
}

static int on_idle_tick(struct Module *mod, unsigned int us_since_last_tick) {
	pak_rt_set_tick_interval(mod, 1000 * 1000);
	return 0;
}

static int on_disconnect(struct Module *mod) {
	return 0;
}

static int on_switch_screen(struct Module *mod, int old_screen, int new_screen, int job) {
	pak_global_log("dummymod: Switching screen (%d -> %d)", old_screen, new_screen);
	for (int i = 0; i < 100; i++) {
		pak_rt_set_progress_bar(mod, job, i);
		usleep(1000);
	}
	return 0;
}

static int on_request_file_contents(struct Module *mod, int job, struct FileHandle *file) {
	pak_rt_add_file_contents(mod, file, _dummy_jpeg_jpg, sizeof(_dummy_jpeg_jpg));
	return 0;
}

static int on_request_thumbnail(struct Module *mod, int job, struct FileHandle *file) {
	return -1;
}

static int on_request_file_metadata(struct Module *mod, int job, struct FileHandle *file) {
	pak_rt_add_file_metadata(mod, file, &(struct FileMetadata){
		.filename = "DSC1234.JPG",
		.mime_type = "image/jpeg",
	});
	return 0;
}

static int on_run_test(struct Module *mod, int screen, int job) {
	return 0;
}

static int on_custom_command(struct Module *mod, const char *request) {
	return 0;
}

int get_module_dummy(struct Module *mod) {
	mod->init = init;
	mod->on_try_connect_wifi = on_try_connect_wifi;
	mod->on_request_file_contents = on_request_file_contents;
	mod->on_find_connection = on_find_connection;
	mod->on_idle_tick = on_idle_tick;
	mod->on_disconnect = on_disconnect;
	mod->on_switch_screen = on_switch_screen;
	return 0;
}
