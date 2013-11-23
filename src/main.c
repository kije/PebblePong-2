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




void handle_init(void) {
	game_init();
}

void handle_deinit(void) {
	game_deinit();
}


int main(void) {
    srand(time(NULL));
	handle_init();
    /*PebbleAppHandlers handlers = {
        .init_handler = &handle_init,
        .timer_handler = &handle_timeout
    };*/
    app_event_loop();
	handle_deinit();
}
