#include <pebble.h>

Window *window;	

TextLayer *txt_hdr_top;
TextLayer *txt_hdr_left;
TextLayer *txt_data_mid;
TextLayer *txt_data_right;
TextLayer *txt_flag;

char chr_hdr_top[128];
char chr_data_mid[128];
char chr_data_right[128];
char chr_flag[32];

// Key values for AppMessage Dictionary
enum {
	CODE_KEY = 0,
	INIT_KEY = 101,
	RUNNAME_KEY = 256,
	FLAG_KEY = 257,
	LAPS_KEY = 258,
	TOD_KEY = 259, // time of day
	TR_KEY = 260 //time remaining in session
};

static void set_text(DictionaryIterator *iter, char *chr, TextLayer *txt) {
	for(int i=1; i<6; i++){
		Tuple *tpl = dict_find(iter, i);
		
		if(tpl){
			if(i==1){
				strcpy(chr, tpl->value->cstring);
			}
			
			if(i>1){
				strcat(chr, "\n");
				strcat(chr, tpl->value->cstring);
			}
		}
	}
	
	text_layer_set_text(txt, chr);
}

static int set_run_name(DictionaryIterator *rec) {
	Tuple *tpl_head = dict_find(rec, RUNNAME_KEY);
	
	if(tpl_head){
		strcpy(chr_hdr_top, tpl_head->value->cstring);
		text_layer_set_text(txt_hdr_top, chr_hdr_top);
		
		return 1;
	}
	
	return 0;
}

static int set_flag(DictionaryIterator *rec) {
	Tuple *tpl_flag = dict_find(rec, FLAG_KEY);
	
	if(tpl_flag){
		//if(strcmp(chr_flag, tpl_flag->value->cstring) != 0) {
		//	if((strcmp(tpl_flag->value->cstring, "Y") == 0) ||
		//	  (strcmp(chr_flag, " ") == 0)) {
		//		vibes_short_pulse();	
		//	}
		//}
		strcpy(chr_flag, tpl_flag->value->cstring);
		text_layer_set_text(txt_flag, chr_flag);
		
		return 1;
	}
	
	return 0;
}

static int set_add(DictionaryIterator *rec) {
	Tuple *tpl_code = dict_find(rec, CODE_KEY);
	
	if(tpl_code){
		switch(tpl_code->value->uint32){
			case 1: // results
				set_text(rec, chr_data_mid, txt_data_mid);
				break;
			case 2: // best times
				set_text(rec, chr_data_right, txt_data_right);
				break;
			case 3: // lap counts
				set_text(rec, chr_data_right, txt_data_right);
				break;
		}
		
		return 1;
	}
	
	return 0;
}

// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	/*
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Message: %s", tuple->value->cstring);
	*/
	
	if(set_run_name(received)){
		set_flag(received);
		return;
	}
	
	set_add(received);
}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {	
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	text_layer_set_text(txt_hdr_top, "Open Pebble App");
}

static void send_hello_message() {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	Tuplet msg_val = TupletCString(101, "init");
	dict_write_tuplet(iter, &msg_val);
	app_message_outbox_send();
}

void init(void) {
	window = window_create();
	window_stack_push(window, true);
	
	Layer *window_layer = window_get_root_layer(window);
	
	txt_hdr_top = text_layer_create(GRect(0, 0, 124, 20));
	text_layer_set_font(txt_hdr_top, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
	//text_layer_set_text_alignment(txt_hdr_top, GTextAlignmentCenter);
	text_layer_set_background_color(txt_hdr_top, GColorBlack);
	text_layer_set_text_color(txt_hdr_top, GColorWhite);
	text_layer_set_text(txt_hdr_top, "Loading...");
	layer_add_child(window_layer, text_layer_get_layer(txt_hdr_top));
	
	txt_flag = text_layer_create(GRect(124, 0, 20, 20));
	text_layer_set_font(txt_flag, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(txt_flag, GTextAlignmentCenter);
	text_layer_set_background_color(txt_flag, GColorBlack);
	text_layer_set_text_color(txt_flag, GColorWhite);
	text_layer_set_text(txt_flag, " ");
	layer_add_child(window_layer, text_layer_get_layer(txt_flag));
	
	txt_hdr_left = text_layer_create(GRect(3, 20, 25, 148));
	text_layer_set_font(txt_hdr_left, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	text_layer_set_text(txt_hdr_left, "1.\n2.\n3.\n4.\n5.");
	layer_add_child(window_layer, text_layer_get_layer(txt_hdr_left));
	
	txt_data_mid = text_layer_create(GRect(25, 20, 35, 148));
	text_layer_set_font(txt_data_mid, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(txt_data_mid, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(txt_data_mid));
	
	txt_data_right = text_layer_create(GRect(72, 20, 60, 148));
	text_layer_set_font(txt_data_right, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	layer_add_child(window_layer, text_layer_get_layer(txt_data_right));
	
	// Register AppMessage handlers
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
		
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
	strcpy(chr_flag, "o");
	
	send_hello_message();
}

void deinit(void) {
	app_message_deregister_callbacks();
	
	text_layer_destroy(txt_hdr_top);
	text_layer_destroy(txt_hdr_left);
	text_layer_destroy(txt_data_mid);
	text_layer_destroy(txt_data_right);
	
	window_destroy(window);
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}