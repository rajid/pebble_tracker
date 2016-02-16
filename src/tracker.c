/*
 * Tracker
 * A simple program to keep track of time used on multiple projects.
 */

#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static BitmapLayer *my_bt_layer;
static GBitmap *my_bluetooth_image;

#define NUM_PROJECTS	10

static time_t proj_time[NUM_PROJECTS];		/* always in accumulated seconds */
static time_t proj_start[NUM_PROJECTS];
static bool proj_running[NUM_PROJECTS];
static char *proj_name[NUM_PROJECTS];

#define MAX_PROJ_NAME_LEN	40

static bool bluetooth_connected;
static bool proj_change_written;

static GFont note_font;
static char string[80];

int cur_p;				/* current project displayed */

/*
 * These numbers must correspond to the appKeys numbers in appinfo.json
 */
unsigned char proj_time_val[NUM_PROJECTS]={0, 3, 6, 9, 12, 30, 33, 36, 39, 42};
unsigned char proj_run_val[NUM_PROJECTS]={1, 4, 7, 10, 13, 31, 34, 37, 40, 43};
unsigned char proj_start_val[NUM_PROJECTS]={2, 5, 8, 11, 14, 32, 35, 38, 41, 44};
unsigned char proj_name_val[NUM_PROJECTS]={15, 16, 17, 18, 19, 45, 46, 47, 48, 49};
unsigned char proj_hour_val[NUM_PROJECTS]={20, 21, 22, 23, 24, 50, 51, 52, 53, 54};
unsigned char proj_min_val[NUM_PROJECTS]={25, 26, 27, 28, 29, 55, 56, 57, 58, 59};

bool autostop;
#define AUTOSTOP	60              /* appkey # */

time_t	now;

int
hours (time_t timeval) 
{

    return ((timeval / 60) / 60);
}

int
minutes (time_t timeval) 
{

    return ((timeval - (hours(timeval) * 60 * 60)) / 60);
}


void
write_proj_change_info (void) 
{
    status_t stat;
    int i;
    uint32_t h, m;
    char string[10];

    DictionaryIterator *iterator;

    app_message_outbox_begin(&iterator);

    for (i = 0 ; i < NUM_PROJECTS ; i++) {
	stat = persist_write_int(proj_time_val[i], (uint32_t)proj_time[i]);
	stat = persist_write_int(proj_run_val[i], (uint32_t)proj_running[i]);
	stat = persist_write_int(proj_start_val[i], (uint32_t)proj_start[i]);
	if (proj_name[i] && *proj_name[i]) {
	    stat = persist_write_string(proj_name_val[i], proj_name[i]);
	    dict_write_cstring(iterator, proj_name_val[i], proj_name[i]);
	} else {
	    stat = persist_write_string(proj_name_val[i], "");
	    dict_write_cstring(iterator, proj_name_val[i], proj_name[i]);
	}
	app_log(APP_LOG_LEVEL_WARNING,
		__FILE__,
		__LINE__,
		"Proj %d time info written as %d:%d", i, hours(proj_time[i]), minutes(proj_time[i]));
	h = (uint32_t)hours(proj_time[i]);
	stat = persist_write_int(proj_hour_val[i], h);
	m = (uint32_t)minutes(proj_time[i]);
	stat = persist_write_int(proj_min_val[i], m);

	snprintf(string, sizeof(string), "%u", (uint)h);
	dict_write_cstring(iterator, proj_hour_val[i], string);

	snprintf(string, sizeof(string), "%u", (uint)m);
	dict_write_cstring(iterator, proj_min_val[i], string);
    }

    app_log(APP_LOG_LEVEL_WARNING,
            __FILE__,
            __LINE__,
            "Writing autostop as %d", (int)autostop);
    stat = persist_write_int(AUTOSTOP, (uint32_t)autostop);
    snprintf(string, sizeof(string), "%u", (uint)autostop);
    dict_write_cstring(iterator, AUTOSTOP, string);

    app_message_outbox_send();

    if (bluetooth_connected == true && stat == 4) {
	proj_change_written = true;
    } else {
	app_log(APP_LOG_LEVEL_WARNING,
		__FILE__,
		__LINE__,
		"Proj time info written");
	proj_change_written = false;
    }
}


void
proj_running_note(struct tm *tick_time, int p) 
{
    int hours, mins, accum;

    accum = ((now - proj_start[p]) + proj_time[p]) / 60;
    hours = accum / 60;		/* hours */
    mins = (accum - (hours * 60));	/* minutes */
    app_log(APP_LOG_LEVEL_WARNING,
	    __FILE__,
	    __LINE__,
	    "Running: time=%ul, start=%ul, accum = %d, hours = %d, mins = %d",
	    (unsigned int)time(0L), (unsigned int)proj_start[p],
	    accum, hours, mins);

    /* Display project info */
    if (proj_name[p] && *proj_name[p]) {
	snprintf(string, sizeof(string),
		 "%2u:%02u\n"
		 "%s\n"
		 "%s\n"
		 "%2u:%02u",
		 (uint)tick_time->tm_hour, (uint)tick_time->tm_min,
		 proj_name[p],
		 "running",
		 (uint)hours, (uint)mins);
    } else {
	snprintf(string, sizeof(string),
		 "%2u:%02u\n"
		 "Proj %u\n"
		 "%s\n"
		 "%2u:%02u",
		 (uint)tick_time->tm_hour, (uint)tick_time->tm_min,
		 (uint)p,
		 "running",
		 (uint)hours, (uint)mins);
    }
    
    app_log(APP_LOG_LEVEL_WARNING,
	    __FILE__,
	    __LINE__,
	    "proj_time[%d] is %d, seconds is %d, string is '%s', mins=%d",
	    p, (int)proj_time[p], accum, string, mins);
    
    text_layer_set_text(text_layer, string);
    layer_mark_dirty(text_layer_get_layer(text_layer));
}

void
proj_stopped_note(struct tm *tick_time, int p) 
{
    int hours, mins, accum;

    accum = proj_time[p] / 60;
    hours = accum / 60;		/* hours */
    mins = (accum - (hours * 60));	/* minutes */
    app_log(APP_LOG_LEVEL_WARNING,
	    __FILE__,
	    __LINE__,
	    "Proj[%d] Stopped: hours = %d, mins = %d",
	    p, hours, mins);

    if (proj_name[p] && *proj_name[p]) {
	snprintf(string, sizeof(string),
		 "%2u:%02u\n"
		 "%s\n"
		 "%s\n"
		 "%2u:%02u",
		 (uint)tick_time->tm_hour, (uint)tick_time->tm_min,
		 proj_name[p],
		 "stopped",
		 (uint)hours, (uint)mins);
    } else {
	snprintf(string, sizeof(string),
		 "%2u:%02u\n"
		 "Proj %u\n"
		 "%s\n"
		 "%2u:%02u",
		 (uint)tick_time->tm_hour, (uint)tick_time->tm_min,
		 (uint)p,
		 "stopped",
		 (uint)hours, (uint)mins);
    }

    text_layer_set_text(text_layer, string);
    layer_mark_dirty(text_layer_get_layer(text_layer));
}


void
display_note(void) 
{
    struct tm *tick_time;
    
    now = time(0L);
    tick_time = localtime(&now);
    if (proj_running[cur_p]) {
	proj_running_note(tick_time, cur_p);
    } else {
	proj_stopped_note(tick_time, cur_p);
    }
}

static void
long_click_handler(ClickRecognizerRef recognizer, void *context) {

    proj_time[cur_p] = 0;
    proj_start[cur_p] = 0;		/* init again */
    proj_running[cur_p] = false;
    proj_stopped_note(localtime(&now), cur_p);
    write_proj_change_info();

    display_note();
}

static void
select_click_handler(ClickRecognizerRef recognizer, void *context) {
    int i;

    now = time(0L);

    app_log(APP_LOG_LEVEL_WARNING,
	    __FILE__,
	    __LINE__,
	    "Middle button");

    if (proj_running[cur_p]) {
	proj_time[cur_p] += now - proj_start[cur_p];
	proj_start[cur_p] = 0;		/* init again */
	proj_running[cur_p] = false;
	proj_stopped_note(localtime(&now), cur_p);
	write_proj_change_info();
    } else {
	proj_running[cur_p] = true;
	time(&proj_start[cur_p]);
        /* If autostop is set, turn off previous project */
        if (autostop) {
            for (i = 0 ; i < NUM_PROJECTS ; i++) {
                if (i == cur_p) continue;
                proj_running[i] = false;
            }
        }
	proj_running_note(localtime(&now), cur_p);
	write_proj_change_info();
    }
}


bool
project_in_use (int p_num)
{
    
    if (proj_time[p_num] != 0 || proj_start[p_num] ||
        proj_running[p_num] ||
        (proj_name[p_num] && *proj_name[p_num])) {
        /* This project is being used */
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__,
		"project %d is in use - proj_time=%d,proj_start=%d,proj_running=%d,proj_name=0x%x <%d>",
                p_num,
                (int)proj_time[p_num],
                (int)proj_start[p_num],
                (int)proj_running[p_num],
                (uint)proj_name[p_num],
                (int)*proj_name[p_num]
            );
        return (true);
    }
    app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__,
            "project %d is not use", p_num);
    return (false);
}


/*
 * increment or decrement project number, circling around
 */
int
next_project_num (int p_num, int direction)
{
    
    /* inc. or dec., then circle around */
    p_num = p_num + direction;
    if (direction == 1 && p_num >= NUM_PROJECTS)
        p_num = 0;
    if (direction == -1 && p_num < 0)
        p_num = NUM_PROJECTS-1;

    return (p_num);
}



/*
 * Find the next project which is in use
 */
int
next_project (int p_num, int direction) 
{
    int start_num = p_num;

    /*
     * If we're at the highest used project number,
     * then return the next number, even if it's not
     * in use.
     * This allows the user to incrementally add projects.
     */
    if (project_in_use(p_num))
        return (next_project_num(p_num, direction));

    /*
     * Else, let's find the next project in use.
     */
    for (;;) {
        p_num = next_project_num(p_num, direction);

        if (project_in_use(p_num)) {
            return (p_num);
        }

        /* If we've circled to where we started, we're done */
        if (p_num == start_num)
            return (p_num);             /* didn't find anything */
    }
}


static void
up_click_handler(ClickRecognizerRef recognizer, void *context) {

    cur_p = next_project(cur_p, 1);

    display_note();
}

static void
down_click_handler(ClickRecognizerRef recognizer, void *context) {

    cur_p = next_project(cur_p, -1);

    display_note();
}

static void
click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 1000, long_click_handler, NULL);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

void
handle_bluetooth(bool connected) 
{
    static bool last_state=true;
    
    if (connected == false) {
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__,
		"Bluetooth lost");
	bluetooth_connected = false;
	layer_set_hidden(bitmap_layer_get_layer(my_bt_layer), false);
    } else {
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__,
		"Bluetooth back");
	bluetooth_connected = true;
	layer_set_hidden(bitmap_layer_get_layer(my_bt_layer), true);
    }
    if (connected != last_state)
	vibes_double_pulse();

    last_state = connected;
}


static void
window_load(Window *window) {

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
#if defined(PBL_RECT)
    text_layer = text_layer_create((GRect) { .origin = { 0, 0 },
		.size = { bounds.size.w,
			  bounds.size.h} });
#elif defined(PBL_ROUND)
    text_layer = text_layer_create((GRect) { .origin = { 10, 0 },
		.size = { bounds.size.w-20,
			  bounds.size.h} });
#endif    
    text_layer_set_font(text_layer, note_font);
    text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
    
    display_note();
    
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(text_layer));

/*
 * Define a field for displaying when bluetooth connection is not there
 */
#if defined(PBL_RECT)
  my_bt_layer = bitmap_layer_create(GRect(0, 5, 30, 30));
#elif defined(PBL_ROUND)
  my_bt_layer = bitmap_layer_create(GRect(30, 5, 30, 30));
#endif    
  my_bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);
  bitmap_layer_set_bitmap(my_bt_layer, my_bluetooth_image);
  layer_add_child(text_layer_get_layer(text_layer),
		  bitmap_layer_get_layer(my_bt_layer));
  layer_set_hidden(bitmap_layer_get_layer(my_bt_layer), true); /* hide it initially */

  bluetooth_connection_service_subscribe(handle_bluetooth);
  /* Update immediate status */
  if (bluetooth_connection_service_peek()) {
      layer_set_hidden(bitmap_layer_get_layer(my_bt_layer), true);
  } else {
      layer_set_hidden(bitmap_layer_get_layer(my_bt_layer), false);
  }

}

static void
window_unload(Window *window) {
  text_layer_destroy(text_layer);
}


void
handle_msg_received (DictionaryIterator *received, void *context)
{

    Tuple *tuple;
    int i, len;
    int hour, min=0;

    for (i = 0 ; i < NUM_PROJECTS ; i++) {
	tuple = dict_find(received, proj_name_val[i]);
	if (tuple) {
	    if (proj_name[i])
		free(proj_name[i]);
            len = strlen(tuple->value->cstring)+1;
            if (len > MAX_PROJ_NAME_LEN)
                len = MAX_PROJ_NAME_LEN;
	    proj_name[i] = malloc(len);
	    strncpy(proj_name[i], tuple->value->cstring, len);
	    
	    app_log(APP_LOG_LEVEL_WARNING,
		    __FILE__,
		    __LINE__,
		    "Project[%d] name is %s",
		    i, proj_name[i]);
	}

	tuple = dict_find(received, proj_hour_val[i]);
	if (tuple) {
	    hour = atoi(tuple->value->cstring);
	    tuple = dict_find(received, proj_min_val[i]);
	    if (tuple) {
		min = atoi(tuple->value->cstring);
		proj_time[i] = ((hour * 60) + min) * 60; /* make into seconds */
	    app_log(APP_LOG_LEVEL_WARNING,
		    __FILE__,
		    __LINE__,
		    "Project[%d] time is %d:%d",
		    i, hour, min);
	    }
	}
    }

    tuple = dict_find(received, AUTOSTOP);
    if (tuple) {
        autostop = (bool)atoi(tuple->value->cstring);
        app_log(APP_LOG_LEVEL_WARNING,
                __FILE__,
                __LINE__,
                "Autostop conf'd as %d",
                (int)autostop);
    }
    
    proj_change_written = false;
    write_proj_change_info();
    display_note();
}

void
handle_msg_dropped (AppMessageResult reason, void *ctx)
{

    app_log(APP_LOG_LEVEL_WARNING,
	    __FILE__,
	    __LINE__,
	    "Message dropped, reason code %d",
	    reason);
}

void
handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {

    now = time(0L);
    if (proj_running[cur_p]) {
	proj_running_note(tick_time, cur_p);
    } else {
	proj_stopped_note(tick_time, cur_p);
    }
}


void
update_configuration (void) 
{
    int i;
    char string[40];

    for (i = 0 ; i < NUM_PROJECTS ; i++) {

	if (persist_exists(proj_time_val[i])) {
	    uint32_t val;
	
	    val = persist_read_int(proj_time_val[i]);
	    if (val) {
		proj_time[i] = (time_t)val;
		
		app_log(APP_LOG_LEVEL_WARNING,
			__FILE__,
			__LINE__,
			"Proj[%d] accumulated time read as %d",
			i, (int)proj_time[i]);
	    }
	}

	proj_running[i] = false;		/* init */
	if (persist_exists(proj_run_val[i])) {
	    uint32_t val;
	    
	    val = persist_read_int(proj_run_val[i]);
	    if (val) {
		proj_running[i] = (int)val;
	    }
	    app_log(APP_LOG_LEVEL_WARNING,
		    __FILE__,
		    __LINE__,
		    "Proj[%d]_running as %d",
		    i, (int)proj_running[i]);
	}

	proj_start[i] = (time_t)0;
	if (persist_exists(proj_start_val[i])) {
	    uint32_t val;
	    
	    val = persist_read_int(proj_start_val[i]);
	    if (val) {
		proj_start[i] = (time_t)val;
		
		app_log(APP_LOG_LEVEL_WARNING,
			__FILE__,
			__LINE__,
			"Proj[%d] start time read as %d",
			i, (int)proj_start[i]);
	    }
	}

	proj_name[i] = NULL;
	if (persist_exists(proj_name_val[i])) {
	    uint32_t val;
	    
	    val = persist_read_string(proj_name_val[i], string, sizeof(string));
	    if (val) {
		proj_name[i] = malloc(strlen(string)+1);
		strcpy(proj_name[i], string);
		app_log(APP_LOG_LEVEL_WARNING,
			__FILE__,
			__LINE__,
			"Proj[%d] name read as %s",
			i, proj_name[i]);
	    }
	}
    }

    autostop = false;;
    if (persist_exists(AUTOSTOP)) {
        int val;
	    
        val = persist_read_int(AUTOSTOP);
        if (val) {
            autostop = (bool)val;
		
            app_log(APP_LOG_LEVEL_WARNING,
                    __FILE__,
                    __LINE__,
                    "Autostop = %d",
                    (int)autostop);
        }
    }
}



static void
init (void) {
    int dict_size;

  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  int i;

  cur_p = 0;				/* arbitrary */
  for (i = 0 ; i < NUM_PROJECTS ; i++) {
      proj_name[i] = NULL;
      proj_time[i] = 0;
      proj_running[i] = false;
      proj_start[i] = 0;
  }

  note_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);

  app_message_register_inbox_received(handle_msg_received);
  app_message_register_inbox_dropped(handle_msg_dropped);
  /* Calculate size of buffer needed */
  dict_size = dict_calc_buffer_size(1,
                                    sizeof(uint32_t), /* proj_time_val */
                                    sizeof(uint32_t), /* proj_time_val */
                                    sizeof(uint32_t), /* proj_time_val */
                                    sizeof(uint32_t), /* proj_time_val */
                                    sizeof(uint32_t), /* proj_time_val */
                                    sizeof(uint32_t), /* proj_time_val */
                                    sizeof(uint32_t), /* proj_time_val */
                                    sizeof(uint32_t), /* proj_time_val */
                                    sizeof(uint32_t), /* proj_time_val */
                                    sizeof(uint32_t), /* proj_time_val */
                                    sizeof(uint32_t), /* proj_run_val */
                                    sizeof(uint32_t), /* proj_run_val */
                                    sizeof(uint32_t), /* proj_run_val */
                                    sizeof(uint32_t), /* proj_run_val */
                                    sizeof(uint32_t), /* proj_run_val */
                                    sizeof(uint32_t), /* proj_run_val */
                                    sizeof(uint32_t), /* proj_run_val */
                                    sizeof(uint32_t), /* proj_run_val */
                                    sizeof(uint32_t), /* proj_run_val */
                                    sizeof(uint32_t), /* proj_run_val */
                                    sizeof(uint32_t), /* proj_start_val */
                                    sizeof(uint32_t), /* proj_start_val */
                                    sizeof(uint32_t), /* proj_start_val */
                                    sizeof(uint32_t), /* proj_start_val */
                                    sizeof(uint32_t), /* proj_start_val */
                                    sizeof(uint32_t), /* proj_start_val */
                                    sizeof(uint32_t), /* proj_start_val */
                                    sizeof(uint32_t), /* proj_start_val */
                                    sizeof(uint32_t), /* proj_start_val */
                                    sizeof(uint32_t), /* proj_start_val */
                                    MAX_PROJ_NAME_LEN, /* proj_names */
                                    MAX_PROJ_NAME_LEN, /* proj_names */
                                    MAX_PROJ_NAME_LEN, /* proj_names */
                                    MAX_PROJ_NAME_LEN, /* proj_names */
                                    MAX_PROJ_NAME_LEN, /* proj_names */
                                    MAX_PROJ_NAME_LEN, /* proj_names */
                                    MAX_PROJ_NAME_LEN, /* proj_names */
                                    MAX_PROJ_NAME_LEN, /* proj_names */
                                    MAX_PROJ_NAME_LEN, /* proj_names */
                                    MAX_PROJ_NAME_LEN, /* proj_names */
                                    sizeof(uint32_t), /* proj_hour_val */
                                    sizeof(uint32_t), /* proj_hour_val */
                                    sizeof(uint32_t), /* proj_hour_val */
                                    sizeof(uint32_t), /* proj_hour_val */
                                    sizeof(uint32_t), /* proj_hour_val */
                                    sizeof(uint32_t), /* proj_hour_val */
                                    sizeof(uint32_t), /* proj_hour_val */
                                    sizeof(uint32_t), /* proj_hour_val */
                                    sizeof(uint32_t), /* proj_hour_val */
                                    sizeof(uint32_t), /* proj_hour_val */
                                    sizeof(uint32_t), /* proj_min_val */
                                    sizeof(uint32_t), /* proj_min_val */
                                    sizeof(uint32_t), /* proj_min_val */
                                    sizeof(uint32_t), /* proj_min_val */
                                    sizeof(uint32_t), /* proj_min_val */
                                    sizeof(uint32_t), /* proj_min_val */
                                    sizeof(uint32_t), /* proj_min_val */
                                    sizeof(uint32_t), /* proj_min_val */
                                    sizeof(uint32_t), /* proj_min_val */
                                    sizeof(uint32_t), /* proj_min_val */
                                    sizeof(bool));    /* autostop */

    
    app_log(APP_LOG_LEVEL_WARNING,
	    __FILE__,
	    __LINE__,	 
	    "dict_size = %d\n", dict_size);

  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
//    app_message_open(dict_size, dict_size);

  update_configuration();

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
