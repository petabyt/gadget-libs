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

static int on_find_connection(struct PakModule *mod, int job) {
	pak_debug_log(mod, "Connection established");
	for (int i = 0; i < 10; i++) {
		if (pak_rt_is_job_cancelled(mod, job)) return PAK_ERR_CANCELLED;
		pak_rt_set_progress_bar(mod, job, i * 10);
		usleep(100000);
	}

	pak_rt_save_session_signature(mod, &(struct PakSavedConnection){
		.name = "DummyDevice",
		.unique_id = "10:20:30:40",
		.aux_data = NULL,
	});

	return 0;
}

static int init(struct PakModule *mod) {
	pak_debug_log(mod, "Hello from dummy module");
	mod->priv = (struct ModulePriv *)malloc(sizeof(struct ModulePriv));
	pak_rt_set_session_property(mod, PAK_PROP_NAME, "Dummy Device");
	pak_rt_set_session_property(mod, PAK_PROP_FW_VER, "v1.2.3");

	pak_rt_set_dashboard_pane(mod, &(struct PakWidget) {
			.name = "switch-wifi",
			.title = "Connect over WiFi",
			.type = PAK_BUTTON,
	});

	pak_rt_set_screen_supported(mod, PAK_SCREEN_DASHBOARD, 1);
	pak_rt_set_screen_supported(mod, PAK_SCREEN_FILE_GALLERY, 1);
	pak_rt_set_screen_supported(mod, PAK_SCREEN_FILE_VIEWER, 1);
	pak_rt_set_screen_supported(mod, 107, 1);

	pak_rt_set_storage_info(mod, "sdcard", 50, PAK_NEWEST_FIRST);
	pak_rt_set_tick_interval(mod, 1000 * 100);
	return 0;
}

static int on_try_connect_wifi(struct PakModule *mod, struct PakWiFiAdapter *handle, int job) {
	return 0;
}

static int on_idle_tick(struct PakModule *mod, unsigned int us_since_last_tick) {
	pak_rt_set_session_property_int(mod, PAK_PROP_BATTERY_MAIN, mod->priv->x);
	if ((mod->priv->x += 10) > 100) mod->priv->x = 0;
	return 0;
}

static int on_disconnect(struct PakModule *mod) {
	return 0;
}

static int on_switch_screen(struct PakModule *mod, int old_screen, int new_screen, int job) {
	pak_global_log("dummymod: Switching screen (%d -> %d)", old_screen, new_screen);
	if (new_screen == PAK_SCREEN_FILE_VIEWER) {
		for (int i = 0; i < 100; i++) {
			if (pak_rt_is_job_cancelled(mod, job)) return 0;
			pak_rt_set_progress_bar(mod, job, i);
			usleep(10000);
		}
	}
	return 0;
}

static int on_request_file_contents(struct PakModule *mod, int job, struct PakFileHandle *file) {
	for (int i = 0; i < 100; i++) {
		if (pak_rt_is_job_cancelled(mod, job)) return 0;
		pak_rt_set_progress_bar(mod, job, i);
		usleep(10000);
	}
	pak_rt_add_file_contents(mod, file, _dummy_jpeg_jpg, sizeof(_dummy_jpeg_jpg), 0);
	return 0;
}

static int on_request_thumbnail(struct PakModule *mod, int job, struct PakFileHandle *file) {
	struct ExifParser metadata;
	int rc = exif_start_raw(&metadata, _dummy_jpeg_jpg, sizeof(_dummy_jpeg_jpg), NULL, NULL);
	if (rc == 0 && metadata.thumb_of != 0) {
		pak_rt_add_file_thumbnail(mod, file, _dummy_jpeg_jpg + metadata.thumb_of, metadata.thumb_size);
		return 0;
	}
	return -1;
}

static int on_request_file_metadata(struct PakModule *mod, int job, struct PakFileHandle *file) {
	char name[32];
	sprintf(name, "DSCF%04u.JPG", file->index_in_view * 13);
	pak_rt_add_file_metadata(mod, file, &(struct PakFileMetadata){
		.filename = name,
		.file_size = 123,
		.mime_type = "image/jpeg",
	});
	return 0;
}

static int on_run_test(struct PakModule *mod, int screen, int job) {
	return 0;
}

static int on_custom_command(struct PakModule *mod, int job, int argc, const char * const *argv) {
	if (!strcmp(argv[0], "help")) {
		//pak_rt_disconnect(mod, "Done");
		pak_global_log("Foo fighters");
	}
	return 0;
}

static int on_prop_changed(struct PakModule *mod, int job, struct PakWidget *prop) {
	pak_global_log("on_prop_changed %s", prop->name);
	return 0;
}

int get_module_dummy(struct PakModule *mod) {
	mod->init = init;
	mod->on_request_file_thumbnail = on_request_thumbnail;
	mod->on_request_file_metadata = on_request_file_metadata;
	//mod->on_try_connect_wifi = on_try_connect_wifi;
	mod->on_request_file_contents = on_request_file_contents;
	mod->on_find_connection = on_find_connection;
	mod->on_idle_tick = on_idle_tick;
	mod->on_disconnect = on_disconnect;
	mod->on_switch_screen = on_switch_screen;
	mod->on_custom_command = on_custom_command;
	mod->on_setting_changed = on_prop_changed;
	return 0;
}
__attribute__((weak)) int get_module(struct PakModule *mod) { return get_module_dummy(mod); }
