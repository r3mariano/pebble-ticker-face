#include <pebble.h>

enum {
  KEY_TEMPERATURE = 0,
  KEY_CONDITIONS = 1,
  KEY_SHOW_SECONDS = 2,
  KEY_DATE_FORMAT = 3,
  KEY_SHOW_AMPM_24H = 4
};

enum {
  PERSIST_KEY_SHOW_SECONDS = 20,
  PERSIST_KEY_DATE_FORMAT = 21,
  PERSIST_KEY_SHOW_AMPM_24H = 22
};

static Window *s_main_window;
static GFont s_custom_font_l;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static TextLayer *s_time_layer;
static TextLayer *s_day_layer;
static TextLayer *s_ampm_layer;
static TextLayer *s_date_layer;
static TextLayer *s_weather_layer;

static PropertyAnimation *s_weather_animation;

// show_seconds: 0 = no, 1 = yes
static int show_seconds = 1;
// date_format: 0 = dd/mm, 1 = mm/dd 
static int date_format = 0;
// show_ampm_24h: 0 = no, 1 = yes
static int show_ampm_24h = 0;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00:00";
  static char ampm_buffer[] = "--";
  static char day_buffer[] = "---";
  static char date_buffer[] = "--/--";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    if (show_seconds == 1)
      strftime(buffer, sizeof("00:00:00"), "%H:%M:%S", tick_time);
    else if (show_ampm_24h == 1)
      strftime(buffer, sizeof("00:00:00"), "%H:%M %p", tick_time);
    else
      strftime(buffer, sizeof("00:00:00"), "%H:%M   ", tick_time);
  } else {
    // Use 12 hour format
    if (show_seconds == 1)
      strftime(buffer, sizeof("00:00:00"), "%I:%M:%S", tick_time);
    else
      strftime(buffer, sizeof("00:00:00"), "%I:%M %p", tick_time);
  }

  if (show_seconds == 1 && 
      (show_ampm_24h == 1 || clock_is_24h_style() != true) )
    strftime(ampm_buffer, sizeof("--"), "%p", tick_time);
  else
    strftime(ampm_buffer, sizeof("--"), "  ", tick_time);
  
  strftime(day_buffer, sizeof("---"), "%a", tick_time);

  if (date_format == 0)
    strftime(date_buffer, sizeof("--/--"), "%e/%m", tick_time);
  else
    strftime(date_buffer, sizeof("--/--"), "%m/%e", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  text_layer_set_text(s_ampm_layer, ampm_buffer);
  text_layer_set_text(s_day_layer, day_buffer);
  text_layer_set_text(s_date_layer, date_buffer);
}

static void main_window_load(Window *window) {
  // Read persist keys if available
  if (persist_exists(PERSIST_KEY_SHOW_SECONDS)) {
    show_seconds = persist_read_int(PERSIST_KEY_SHOW_SECONDS);
  }
  if (persist_exists(PERSIST_KEY_DATE_FORMAT)) {
    date_format = persist_read_int(PERSIST_KEY_DATE_FORMAT);
  }
  if (persist_exists(PERSIST_KEY_SHOW_AMPM_24H)) {
    show_ampm_24h = persist_read_int(PERSIST_KEY_SHOW_AMPM_24H);
  }

  // Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DOTS);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));

  // Declare custom font
  s_custom_font_l = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LED_38));

  GColor text_color;
  #ifdef PBL_COLOR
    text_color = GColorChromeYellow;
  #else
    text_color = GColorWhite;
  #endif

  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(2, 0, 144, 42));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, text_color);
  text_layer_set_font(s_time_layer, s_custom_font_l);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  s_day_layer = text_layer_create(GRect(2, 36, 72, 42));
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_text_color(s_day_layer, text_color);
  text_layer_set_font(s_day_layer, s_custom_font_l);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_day_layer));

  s_ampm_layer = text_layer_create(GRect(74, 36, 70, 42));
  text_layer_set_background_color(s_ampm_layer, GColorClear);
  text_layer_set_text_color(s_ampm_layer, text_color);
  text_layer_set_font(s_ampm_layer, s_custom_font_l);
  text_layer_set_text_alignment(s_ampm_layer, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_ampm_layer));

  s_date_layer = text_layer_create(GRect(2, 72, 144, 42));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, text_color);
  text_layer_set_font(s_date_layer, s_custom_font_l);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));

  s_weather_layer = text_layer_create(GRect(2, 108, 240, 42));
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, text_color);
  text_layer_set_font(s_weather_layer, s_custom_font_l);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));

  update_time();
}

static void main_window_unload(Window *window) {
    // Destroy TextLayer
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_day_layer);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_weather_layer);

    fonts_unload_custom_font(s_custom_font_l);
}

void animation_started(Animation *animation, void *data) {

}

void animation_2_stopped(Animation *animation, bool finished, void *data) {
  #ifdef PBL_PLATFORM_APLITE
    property_animation_destroy((PropertyAnimation*)animation);
  #endif
}

void animation_stopped(Animation *animation, bool finished, void *data) {
  // Do second part of animation
  #ifdef PBL_PLATFORM_APLITE
    property_animation_destroy((PropertyAnimation*)animation);
  #endif

  // Set start and end
  GRect from_frame = GRect(160, 108, 240, 42);
  GRect to_frame = GRect(2, 108, 240, 42);

  // Create the animation
  s_weather_animation = property_animation_create_layer_frame((Layer *)s_weather_layer, &from_frame, &to_frame);

  animation_set_duration((Animation *) s_weather_animation, 3000);

  animation_set_handlers((Animation*) s_weather_animation, (AnimationHandlers) {
      .started = (AnimationStartedHandler) animation_started,
      .stopped = (AnimationStoppedHandler) animation_2_stopped,
    }, NULL);

  // Schedule to occur ASAP with default settings
  animation_schedule((Animation*) s_weather_animation);
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  // Scroll
  // Set start and end
  GRect from_frame = layer_get_frame((Layer *) s_weather_layer);
  GRect to_frame = GRect(-240, 108, 240, 42);

  // Create the animation
  s_weather_animation = property_animation_create_layer_frame((Layer *) s_weather_layer, &from_frame, &to_frame);

  animation_set_duration((Animation *) s_weather_animation, 3000);

  animation_set_handlers((Animation*) s_weather_animation, (AnimationHandlers) {
      .started = (AnimationStartedHandler) animation_started,
      .stopped = (AnimationStoppedHandler) animation_stopped,
    }, NULL);

  // Schedule to occur ASAP with default settings
  animation_schedule((Animation*) s_weather_animation);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();

  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0 && tick_time->tm_sec == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
 
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
 
    // Send the message!
    app_message_outbox_send();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  int update_weather = 0;
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);
 
  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%d", (int)t->value->int32);
      update_weather = 1;
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      update_weather = 1;
      break;
    case KEY_SHOW_SECONDS:
      show_seconds = t->value->int32;
      persist_write_int(PERSIST_KEY_SHOW_SECONDS, show_seconds);
      if (show_seconds == 0) {
        tick_timer_service_unsubscribe();
        tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
      } else {
        tick_timer_service_unsubscribe();
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
      }
      break;
    case KEY_DATE_FORMAT:
      date_format = t->value->int32;
      persist_write_int(PERSIST_KEY_DATE_FORMAT, date_format);
      break;
    case KEY_SHOW_AMPM_24H:
      show_ampm_24h = t->value->int32;
      persist_write_int(PERSIST_KEY_SHOW_AMPM_24H, show_ampm_24h);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }
 
    // Look for next item
    t = dict_read_next(iterator);
  }
  
  // Assemble full string and display
  if (update_weather == 1) {
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s/%s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
  } else {
    update_time();
  }
}
 
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}
 
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}
 
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  // Register with accel tap service
  accel_tap_service_subscribe(tap_handler);
}

static void deinit() {
    // Destroy Window
    window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}