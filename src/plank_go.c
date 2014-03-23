#include <pebble.h>

#include "plank.h"

#define SET_STEP_READY 0
#define SET_STEP_REGULAR 1
#define SET_STEP_REST_LEFT 2
#define SET_STEP_SIDE_LEFT 3
#define SET_STEP_REST_RIGHT 4
#define SET_STEP_SIDE_RIGHT 5
#define SET_STEP_REST_NEXT_SET 6
#define SET_STEP_MAX 7

static int go_set_step = 0;
static int go_set_crt = 0;
static int step_timer = 0;

static bool pause = false;

static Window *window;

static TextLayer *text_layer;
static TextLayer *text_layer_timer;

static BitmapLayer *image_layer;
static GBitmap *image;

static GBitmap *img_bar_sel;

static AppTimer *timer;
static ActionBarLayer *action_bar;

static void window_load(Window *window);
static void window_unload(Window *window);

static void draw_action_bar(void)
{
  if(pause){
    text_layer_set_text(text_layer_timer, "Paused...");
    if(img_bar_sel != NULL){
      gbitmap_destroy(img_bar_sel);
    }
    img_bar_sel = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PLAY);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, img_bar_sel);
  }
  else{
    if(img_bar_sel != NULL){
      gbitmap_destroy(img_bar_sel);
    }
    img_bar_sel = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PAUSE);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, img_bar_sel);
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  pause = !pause;
  draw_action_bar();
}

//static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Up");
//}

//static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Down");
//}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  //window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  //window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void timer_callback(void *context) {

  if(pause){
    timer = app_timer_register(1000, timer_callback, NULL);
    return;
  }

  step_timer--;
  if(step_timer == 0){
    go_set_step += 1;
    go_set_step %= SET_STEP_MAX;

    switch(go_set_step){
      case SET_STEP_REGULAR:
      case SET_STEP_SIDE_LEFT:
      case SET_STEP_SIDE_RIGHT:
        step_timer = cfg_time;
        break;
      case SET_STEP_REST_NEXT_SET:
        go_set_crt++;
        go_set_step = SET_STEP_READY;
        step_timer = cfg_rest;
        break;
      case SET_STEP_REST_LEFT:
      case SET_STEP_REST_RIGHT:
        step_timer = cfg_rest;
        break;
    }

    vibes_long_pulse();

    if(image != NULL){
      gbitmap_destroy(image);
    }

    switch(go_set_step){
      case SET_STEP_READY:
        text_layer_set_text(text_layer, "Rest & Prepare ...");
        image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_REGULAR);
        break;
      case SET_STEP_REGULAR:
        text_layer_set_text(text_layer, "Regular Planks");
        image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_REGULAR);
        break;
      case SET_STEP_REST_NEXT_SET:
        text_layer_set_text(text_layer, "Rest & Prepare ...");
        image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_REGULAR);
        break;
      case SET_STEP_REST_LEFT:
        text_layer_set_text(text_layer, "Rest & Prepare ...");
        image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SIDE_LEFT);
        break;
      case SET_STEP_SIDE_LEFT:
        text_layer_set_text(text_layer, "Left Side Planks");
        image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SIDE_LEFT);
        break;
      case SET_STEP_REST_RIGHT:
        text_layer_set_text(text_layer, "Rest & Prepare ...");
        image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SIDE_RIGHT);
        break;
      case SET_STEP_SIDE_RIGHT:
        text_layer_set_text(text_layer, "Right Side Planks");
        image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SIDE_RIGHT);
        break;
    }

    bitmap_layer_set_bitmap(image_layer, image);
    layer_mark_dirty(bitmap_layer_get_layer(image_layer));
  }
  else{
    if(cfg_tic != 0){
      vibes_short_pulse();
    }
  }

  if(go_set_crt < cfg_sets){
    static char t[10] = "";
    snprintf(t, 10, "%d", step_timer);
    text_layer_set_text(text_layer_timer, t);
    timer = app_timer_register(1000, timer_callback, NULL);
  }
  else{
    text_layer_set_text(text_layer, "WELL");
    text_layer_set_text(text_layer_timer, "DONE!");

    int next = persist_exists(NUM_PERSIST_LOG_CRT) ? persist_read_int(NUM_PERSIST_LOG_CRT)+1 : 0;
    next = next % PERSIST_LOG_MAX;

    char new[PERSIST_LOG_LEN];
    time_t t = time(NULL);
    struct tm *loc =  localtime(&t);
    snprintf(new, PERSIST_LOG_LEN, 
             "[%02d%02d %02d%02d]%2dx3x%3d(%2d)",
             loc->tm_mon+1, loc->tm_mday, loc->tm_hour, loc->tm_min,
             cfg_sets, cfg_time, cfg_rest);
    persist_write_string(NUM_PERSIST_LOG+next, new); 
    persist_write_int(NUM_PERSIST_LOG_CRT, next); 
    APP_LOG(APP_LOG_LEVEL_DEBUG, "saving %s into %d", new, next);
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 10 }, .size = { bounds.size.w-20, 20 } });
  text_layer_set_text(text_layer, "READY");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  text_layer_timer = text_layer_create((GRect) { .origin = { 0, 110 }, .size = { bounds.size.w-20, 30 } });
  text_layer_set_text(text_layer_timer, "TIMER");
  text_layer_set_font(text_layer_timer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(text_layer_timer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer_timer));

  // This needs to be deinited on app exit which is when the event loop ends
  image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_REGULAR);

  // The bitmap layer holds the image for display
  image_layer = bitmap_layer_create((GRect) { .origin = { 0, 30 }, .size = { bounds.size.w-20, 80 } });
  bitmap_layer_set_bitmap(image_layer, image);
  bitmap_layer_set_alignment(image_layer, GAlignCenter);
  layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));

  // Initialize the action bar:
  action_bar = action_bar_layer_create();
  // Associate the action bar with the window:
  action_bar_layer_add_to_window(action_bar, window);
  // Set the click config provider:
  action_bar_layer_set_click_config_provider(action_bar,
                                             click_config_provider);

  draw_action_bar();

  go_set_step = SET_STEP_READY;
  step_timer = 5;
  go_set_crt = 0;

  timer = app_timer_register(1000, timer_callback, NULL);
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  text_layer_destroy(text_layer_timer);

  gbitmap_destroy(image);
  bitmap_layer_destroy(image_layer);

  app_timer_cancel(timer);

  action_bar_layer_destroy(action_bar);
  gbitmap_destroy(img_bar_sel);
}

void open_go_window_layer(void)
{
  window = window_create();

  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);
}
