// Communication and settings
// Started by ron64
#include <pebble.h>
#include "comm.h"

bool got_remote(float *rem_y, float *rem_vel)
{
	//toDo: update when got data from remote
	return false;
}

void send_local(float local_y, float local_vel)
{
	//toDo: send update when local status changed
}

bool got_ball(float *x,float *y, float *vx,float *vy, uint16_t *score)
{
	//toDo: update when got ball collide or reset data from remote
	return false;
}

void send_ball(float x,float y, float vx,float vy, uint16_t score)
{
	//toDo: send update when ball colided or reset
}


void set_defaults()
{
	option.invert=false; //not inverted
	option.left=0; 
	option.right=0;
	option.reserved1= option.reserved2= option.reserved3= option.reserved4= 0;
	option.name[0]='\0';
}

void read_persistant(void)
{
	status_t status = 0;
	bool p_exsist;
	p_exsist = persist_exists(CONFIG_KEYS);
	if (p_exsist)
	{
		status = persist_read_data(CONFIG_KEYS, &option, sizeof(option));
		if (status<0) APP_LOG(APP_LOG_LEVEL_ERROR, "error in persist read: %d", (int)status);
	}
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Read:  Invert:%d, Left:%d, Right:%d, Name:%s", 
			option.invert, option.left, option.right, option.name);
}	
void write_persistant(void)
{
	status_t status = 0;
	status = persist_write_data(CONFIG_KEYS, &option, sizeof(option));
	if (status<0) APP_LOG(APP_LOG_LEVEL_ERROR, "error in persist write: %d", (int)status);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Wrote:  Invert:%d, Left:%d, Right:%d, Name:%s", 
			option.invert, option.left, option.right, option.name);
}	

void in_received_handler(DictionaryIterator *received, void *context)
{
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "msg recived");
	Tuple *BG_tuple 	= dict_find(received, CONFIG_KEY_BG);
    Tuple *right_tuple	= dict_find(received, CONFIG_KEY_RIGHT);
	Tuple *left_tuple 	= dict_find(received, CONFIG_KEY_LEFT);
	Tuple *name_tuple	= dict_find(received, CONFIG_KEY_NAME);

	bool key; int8_t num; //char str[20]; 
	static char msg[60] = "";
    if (BG_tuple)
    {
		key=(strcmp(BG_tuple->value->cstring, "white") == 0);
		option.invert = key;
		strcat(msg," BG=");
		strcat(msg, BG_tuple->value->cstring);
    }
	if (right_tuple){
		num= right_tuple->value->uint8;
		if ((num>='0')&&(num<='2'))
		option.right = num-48;
		strcat(msg," right=");strcat(msg,right_tuple->value->cstring);
	}
	if (left_tuple){
		num= left_tuple->value->uint8;
		if ((num>='0')&&(num<='2'))
		option.left = num-48;
		strcat(msg," left=");strcat(msg,left_tuple->value->cstring);
	}
    if (name_tuple)
    {
		//str=name_tuple->value->cstring;
		strncpy(option.name, name_tuple->value->cstring,9);
		strcat(msg," Name=");
		strcat(msg, name_tuple->value->cstring);
    }
	APP_LOG(APP_LOG_LEVEL_DEBUG, msg);
	write_persistant();
    //update_configuration(); //to be implemented
  
}

void in_dropped_handler(AppMessageResult reason, void *ctx)
{
    APP_LOG(APP_LOG_LEVEL_WARNING, "Message dropped, reason code 0x%02X", reason);
}

void comm_init()
{
	set_defaults();
	read_persistant();
	app_message_register_inbox_received(in_received_handler);
    app_message_register_inbox_dropped(in_dropped_handler);
    app_message_open(120, 120);	
		
}
void comm_deinit(void)
{
	app_message_deregister_callbacks();
	
}