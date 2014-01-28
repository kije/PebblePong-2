// Communication and settings
// Started by ron64

#ifndef COMM_H_INCLUDED
#define COMM_H_INCLUDED

#define CONFIG_KEY_BG		7
#define CONFIG_KEY_NAME		8
#define CONFIG_KEY_LEFT		9
#define CONFIG_KEY_RIGHT	10
#define CONFIG_KEYS   		789	

typedef struct __attribute__((__packed__)){
	bool invert; 	//invert status
    uint8_t left; 	//player on left side, default AI
    uint8_t right; 	//player on right side, default Hyuman
    uint8_t reserved1; 	//reserved
    uint8_t reserved2; 	//reserved
    uint8_t reserved3; 	//reserved
    uint8_t reserved4; 	//reserved
    char name[10];  // Name of player to be displayed on the screen
}option_t;

option_t option;
	
bool got_remote(float *rem_y, float *rem_vel);
void send_local(float local_y, float local_vel);

bool got_ball(float *x,float *y, float *vx,float *vy, uint16_t *score);
void send_ball(float x,float y, float vx,float vy, uint16_t score);

void comm_deinit(void);
void comm_init();	
	
#endif //COMM_H_INCLUDED