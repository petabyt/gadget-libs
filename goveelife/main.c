#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <runtime.h>
#include <bluetooth.h>

static int init(struct Module *mod) {
	//pak_debug_log(mod, "goveelife init");
	return 0;
	//pak_rt_set_tick_interval(mod, 1000 * 1000);

}
static int on_find_connection(struct Module *mod, int job) {
	return 0;
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

int get_pow(int x, int y);

int get_module_cmfnothingaudio(struct Module *mod) {
	pak_global_log("test: %s\n", "asd");
	mod->init = init;
	mod->on_find_connection = on_find_connection;
	mod->on_idle_tick = on_idle_tick;
	mod->on_disconnect = on_disconnect;
	mod->on_switch_screen = on_switch_screen;
	return 0;
}
__attribute__((weak)) int get_module(struct Module *mod) { return get_module_cmfnothingaudio(mod); }
