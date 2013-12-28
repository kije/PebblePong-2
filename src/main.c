/*
PEBBLEPONG by kije (http://github.com/kije/)

TODO:
– Clean up code
– – Javadoc 
– Better AI
– Accelometer controlled Paddles (API not released yet :/)
– Settings-Menu
– Muliplayer over Bluetooth (or over httpebble -> protocol for communicate with other pebble implementations on multiple platforms)
– More realistic ball movement 
 */
/***** THIS IS THE SDK 2.0 VERSION OF PEBBLE PONG. THE OTHER VERSION IS DISCONTINUED */

#include <pebble.h>
#include "main.h"
#include "game.h"




static void init(void) {
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__ , __LINE__ , "Init PebblePong");
	game_init();
}

static void deinit(void) {
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__ , __LINE__ , "denit PebblePong");
	game_deinit();
}


int main(void) {
    srand(time(NULL));
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__ , __LINE__ , "Start");
	init();
    /*PebbleAppHandlers handlers = {
        .init_handler = &handle_init,
        .timer_handler = &handle_timeout
    };*/
    app_event_loop();
	deinit();
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__ , __LINE__ , "Done");
	return 0;
}
