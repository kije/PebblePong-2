#include <pebble.h>
#include <math.h>
#include <comm.h>
#include "main.h"
#include "game.h"

Window *window;
TextLayer *titleLayer;
TextLayer *scoreLayer;
InverterLayer *inverter_layer;

AppTimer *timer_handle;

Layer *gameLayer;
Player pl1,pl2;
Ball ball;

GRect validBallField;
GRect validPaddleField;

int button_up_pressed;
int button_down_pressed;

uint16_t level;

AccelData calibratedAccelData, accelData;

PrecisePoint ball_history[16];
uint16_t current_ball_history_position = 0;
    
int16_t getAccelDelta() {
	accel_service_peek(&accelData);
	return (accelData.y - calibratedAccelData.y);
}

void ai(Player *self) {
    int32_t ball_vertical_position = ball_history[
        (current_ball_history_position-COUNT_OF(ball_history))%COUNT_OF(ball_history)
    ].y; // Face ball position -> Emulate Inertia
    int32_t paddle_vertical_position = (*self).paddle.bounds.origin.y;

    if (((*self).paddle.bounds.origin.y <= validPaddleField.size.h)) {

        if (ball_vertical_position > paddle_vertical_position) { // ball is above paddle
            (*self).paddle.bounds.origin.y += (*self).velocity;

        } 
    }

    if (((*self).paddle.bounds.origin.y > validPaddleField.origin.y)) {

        if (ball_vertical_position < paddle_vertical_position) { // ball is below paddle
                (*self).paddle.bounds.origin.y -= (*self).velocity;
        }
    }
}

void human(Player *self) {	
	if (button_up_pressed != 0 || button_down_pressed != 0) {
		if (button_up_pressed != 0 && (*self).paddle.bounds.origin.y > validPaddleField.origin.y) { 
			(*self).paddle.bounds.origin.y -= (*self).velocity;
		}
	
		if (button_down_pressed != 0 && (*self).paddle.bounds.origin.y <= validPaddleField.size.h) {
			(*self).paddle.bounds.origin.y += (*self).velocity;
		}
	} else {
		int16_t detlta = getAccelDelta();
		
		
		if (detlta > 0 && (*self).paddle.bounds.origin.y > validPaddleField.origin.y) { 
			(*self).paddle.bounds.origin.y -= abs(detlta)/80;
		}
	
		if (detlta < 0 && (*self).paddle.bounds.origin.y <= validPaddleField.size.h) {
			(*self).paddle.bounds.origin.y += abs(detlta)/80;
		}
	}
}

// TODO: Multiplayer
	//ToDo: 
    // 1. init_player: Init remote with link to other player data.
    // 2. send data of local player to remote.
    // 3. transfer control of remote collision to remote side, local is display only, by received data +keeping momentum
    // 4. consider adding additional header file to keep struct that are common to comm and game.

void multiplayer(Player *self) {
	float rem_y;
	float rem_vel;
	if (got_remote(&rem_y,&rem_vel))
	{
		(*self).paddle.bounds.origin.y=rem_y;
		(*self).velocity=rem_vel;
	}
	else {
		(*self).paddle.bounds.origin.y += (*self).velocity; //keep last direction
		//ToDo: prevent case of out of bounds;
	}
	//ToDo: Handle loss of remote player, revert to AI.
	
}

void reset_ball(Side side) {
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__ , __LINE__ , "Ball reset");
    GRect bounds = layer_get_bounds(gameLayer);
    ball.position = (PrecisePoint){(bounds.size.w/2)-(BALL_SIZE_WIDTH/2), (bounds.size.h/2)-(BALL_SIZE_HEIGHT/2)};
    if (side == pl1.side) {
        ball.vetctor = (MovementVector){1,1};
    } else {
        ball.vetctor = (MovementVector){-1,1};
    }
    send_ball(ball.position.x, ball.position.y, ball.vetctor.vx, ball.vetctor.vx, 0); 
	//ToDo: 
	//  1. send correct score as well
	//  1. Change mode of active player.

}


int is_colided_with_paddle(Paddle paddle) {

	// TODO: optimize -> we don't need to check, if all 4 sides are colided...
    int16_t ball_left = ball.position.x, ball_right = ball.position.x+ball.size.w,
             ball_top  = ball.position.y, ball_bottom = ball.position.y+ball.size.h;

    int16_t paddle_left = paddle.bounds.origin.x, paddle_right = paddle.bounds.origin.x+paddle.bounds.size.w,
             paddle_top  = paddle.bounds.origin.y, paddle_bottom = paddle.bounds.origin.y+paddle.bounds.size.h;


    if (
        ((ball_left >= paddle_right) ||
        (ball_right <= paddle_left)) ||
        ((ball_top >= paddle_bottom) ||
        (ball_bottom <= paddle_top))
    ) {
        return 0;
    } 


    return 1;
}

void ball_hit_paddle(Paddle paddle) {
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__ , __LINE__ , "Ball hit Paddle");
	//ToDo: Fix bug that ball get stuck to paddle(give it some round initial x)
	
    ball.vetctor.vx *= -1; // reverse Ball movements
	// ToDo: change controling side
	// ToDo: Send correct score
    send_ball(ball.position.x, ball.position.y, ball.vetctor.vx, ball.vetctor.vx, 0);
}

void register_ball_position() {
    ball_history[current_ball_history_position] = ball.position;
    current_ball_history_position = (current_ball_history_position+1)%COUNT_OF(ball_history);
}
void check_hit(Player pxx)
{ //now old code, two section will be combined, and check for remote(multyplayer) will be added
    // hit edge? 
    if (ball.position.x < validBallField.origin.x) {
        pl2.score++;
		app_log(APP_LOG_LEVEL_DEBUG, __FILE__ , __LINE__ , "Player 2 get point\nScore: P1: %d P2: %d", pl1.score, pl2.score);
        reset_ball(pl1.side);
    }

    if (ball.position.x > validBallField.size.w) {
		pl1.score++;
		app_log(APP_LOG_LEVEL_DEBUG, __FILE__ , __LINE__ , "Player 1 get point\nScore: P1: %d P2: %d", pl1.score, pl2.score);
        reset_ball(pl2.side);
    }

	// Ball colision check
    if (is_colided_with_paddle(pl1.paddle) != 0) {
        ball_hit_paddle(pl1.paddle);
    } else if (is_colided_with_paddle(pl2.paddle) != 0) {
        ball_hit_paddle(pl2.paddle);
    }

}

// Thanks to flightcrank(https://github.com/flightcrank) and his Pong(https://github.com/flightcrank/pong) for the idea how to move the ball :)
void move_ball() {
    // Move Ball 
    ball.position.x += ball.velocity*ball.vetctor.vx;
    ball.position.y += ball.velocity*ball.vetctor.vy;

     // if ball touches top or bottom of gameField -> revert vy
    if (ball.position.y < validBallField.origin.y || ball.position.y > validBallField.size.h) {
        ball.vetctor.vy = -ball.vetctor.vy; // reverse Ball movements
    }

    // check hit and collision? 
	check_hit(pl1);
	//check_hit(pl2);
	
	float r_x, r_y, r_vx, r_vy;
	uint16_t r_score;
	if (got_ball(&r_x, &r_y, &r_vx, &r_vy, &r_score))
	{
		//ToDo update on reset/hit from other side, and check score
	}
    // Store history
    register_ball_position();
}


void init_player(Player *player, Side side, Pl_type pl_type) {
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__ , __LINE__ , "Player init:\nSide: %s\n Is KI: %d", (side == WEST ? "West" : "EAST"), pl_type);
    GRect bounds = layer_get_bounds(gameLayer);
    Paddle paddle = (Paddle){
		(PreciseRect) {
			.origin = (PrecisePoint) {
				.x = (side == WEST ? 0+PADDLE_WIDTH : bounds.size.w-PADDLE_WIDTH-2 /* Why 2 px offset? */),
				.y = (bounds.size.h/2)-(PADDLE_HEIGHT/2)
			},
			.size = (PreciseSize) {
				.w = PADDLE_WIDTH,
				.h = PADDLE_HEIGHT
			}
		}};
    *player = (Player){
        .score = 0,
        .paddle = paddle, 
        .side = side, 
        .pl_type = pl_type, 
        .control_handler = (pl_type==AI ? &ai : &human), //ToDo: modify for remote
		.velocity = 0.6
    };
}

void init_ball(Ball *b) {
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__ , __LINE__ , "Ball init");
    GRect bounds = layer_get_bounds(gameLayer);
    *b = (Ball){
        {(bounds.size.w/2)-(BALL_SIZE_WIDTH/2), (bounds.size.h/2)-(BALL_SIZE_HEIGHT/2)},
        GSize(BALL_SIZE_WIDTH, BALL_SIZE_HEIGHT),
        {1,1},
		0.6
    };
}

void draw_dotted_line(GContext *ctx, GPoint p1, GPoint p2, uint16_t space) {
    uint16_t start = p1.y;
    uint16_t stop = p2.y;

    for (uint16_t i = start; i < stop; i+=(space*2)) {
        graphics_draw_line(ctx, GPoint(p1.x,i), GPoint(p2.x,i+space));
    }
}

void draw_paddle(GContext *ctx, Paddle paddle) {
    graphics_fill_rect(ctx, GRect(paddle.bounds.origin.x, paddle.bounds.origin.y, paddle.bounds.size.w, paddle.bounds.size.h), 0, GCornerNone);
}

void draw_ball(GContext *ctx, Ball b) {
    graphics_fill_rect(ctx, GRect(b.position.x, b.position.y, b.size.w, b.size.h), 0, GCornerNone);
}

void draw_game_field(struct Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_stroke_color (ctx, FG_COLOR); 
    graphics_context_set_fill_color (ctx, FG_COLOR);   

    // Frame
    graphics_draw_rect(ctx, bounds);

    // Middle-line
    draw_dotted_line(ctx, GPoint(bounds.size.w/2,0), GPoint(bounds.size.w/2,bounds.size.h), 3); 


    draw_paddle(ctx,pl1.paddle);
    draw_paddle(ctx,pl2.paddle);


    draw_ball(ctx, ball); 

    // update score
    static char buffer[45] = "";

    snprintf(
        buffer, 
        sizeof(buffer),
        "%d | %d",
        pl1.score, pl2.score
    );
    text_layer_set_text(scoreLayer, buffer);
}

void up_up_handler(ClickRecognizerRef recognizer, Window *window) {
    button_up_pressed=0;
}

void up_down_handler(ClickRecognizerRef recognizer, Window *window) {
    button_up_pressed=1;
}


void down_up_handler(ClickRecognizerRef recognizer, Window *window) {
    button_down_pressed=0;
}

void down_down_handler(ClickRecognizerRef recognizer, Window *window) {
    button_down_pressed=1;
}

void select_down(ClickRecognizerRef recognizer, Window *window) {
}

void select_up(ClickRecognizerRef recognizer, Window *window) {
    accel_service_peek(&calibratedAccelData); //Calibrate data to initial position
}


void config_provider(void *context) {
    window_set_click_context(BUTTON_ID_UP, context);
    
    window_raw_click_subscribe(BUTTON_ID_UP, (ClickHandler)up_down_handler, (ClickHandler)up_up_handler, context);
    window_raw_click_subscribe(BUTTON_ID_DOWN, (ClickHandler)down_down_handler, (ClickHandler)down_up_handler, context);
	window_raw_click_subscribe(BUTTON_ID_SELECT, (ClickHandler)select_down, (ClickHandler)select_up, context);
	

}

void handle_accel(AccelData *accel_data, uint32_t num_samples) {
  // do nothing
}

void handle_timer_timeout(void *data) {
    pl1.control_handler(&pl1);
    pl2.control_handler(&pl2);
    move_ball();
    layer_mark_dirty(gameLayer);
	
    timer_handle = app_timer_register(UPDATE_FREQUENCY, &handle_timer_timeout, NULL);
}

void game_init(void) {
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__ , __LINE__ , "Game Init");
	comm_init();

	app_log(APP_LOG_LEVEL_DEBUG_VERBOSE, __FILE__ , __LINE__ , "Settings: \nPaddle Size (w/h): %d / %d \nBall Size (w/h): %d / %d \nUpdate Frequency: every %d ms", PADDLE_WIDTH, PADDLE_HEIGHT, BALL_SIZE_WIDTH ,BALL_SIZE_HEIGHT, UPDATE_FREQUENCY);
	
	window = window_create();
	
    window_set_click_config_provider(window, (ClickConfigProvider) config_provider); // Only till the Accelometer-API is released

    window_set_background_color(window, BG_COLOR);
    window_stack_push(window, true /* Animated */);
	
	button_up_pressed=0;
    button_down_pressed=0;
	
	level = 1;
	
	// accelometer
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__ , __LINE__ , "accel init");
	accel_data_service_subscribe(0, handle_accel);
	accel_service_set_sampling_rate(ACCEL_SAMPLING_100HZ);
	accel_service_peek(&calibratedAccelData); //Calibrate data to initial position
	
    
    // Title Layer
    titleLayer = text_layer_create(GRect(0,0,144,25));
    text_layer_set_text_alignment(titleLayer, GTextAlignmentCenter);
    text_layer_set_text(titleLayer, "PebblePong");
    text_layer_set_font(titleLayer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ORBITRON_BOLD_15)));
    text_layer_set_background_color(titleLayer, BG_COLOR);
    text_layer_set_text_color(titleLayer, FG_COLOR);
    layer_add_child(window_get_root_layer(window), (Layer *)titleLayer);

    //Score Layer
    scoreLayer = text_layer_create(GRect(0,22,144,19));
    text_layer_set_text_alignment(scoreLayer, GTextAlignmentCenter);
    text_layer_set_text(scoreLayer, "0 | 0");
    text_layer_set_font(scoreLayer, fonts_get_system_font (FONT_KEY_GOTHIC_14));
    text_layer_set_background_color(scoreLayer, BG_COLOR);
    text_layer_set_text_color(scoreLayer, FG_COLOR);
    layer_add_child(window_get_root_layer(window), (Layer *)scoreLayer);


	
    // Game Field

    gameLayer = layer_create(GRect(5, 40, 134, 90));
    layer_set_update_proc(gameLayer, draw_game_field);
    layer_add_child(window_get_root_layer(window), (Layer *)gameLayer);

    validBallField = GRect(1,1,(layer_get_bounds(gameLayer).size.w-BALL_SIZE_WIDTH)-1,(layer_get_bounds(gameLayer).size.h-BALL_SIZE_HEIGHT)-1);
    validPaddleField = GRect(1,1,(layer_get_bounds(gameLayer).size.w-PADDLE_WIDTH)-2,(layer_get_bounds(gameLayer).size.h-PADDLE_HEIGHT)-2);

	
    inverter_layer = inverter_layer_create(GRect(0, 0, 144, 168));
    layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(inverter_layer));
	layer_set_hidden(inverter_layer_get_layer(inverter_layer), !option.invert);
	
    // Player 

    init_player(&pl1, WEST, AI);
    init_player(&pl2, EAST, HUMAN);

    // Ball 
    init_ball(&ball);

    // Setup timer
    timer_handle = app_timer_register(UPDATE_FREQUENCY, &handle_timer_timeout, NULL);
}

void game_deinit(void) {
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__ , __LINE__ , "Game deinit");
	comm_deinit();
	app_timer_cancel(timer_handle);
    text_layer_destroy(titleLayer);
    text_layer_destroy(scoreLayer);
	layer_destroy(gameLayer);
	inverter_layer_destroy(inverter_layer);
	window_destroy(window);
	accel_data_service_unsubscribe();
}
