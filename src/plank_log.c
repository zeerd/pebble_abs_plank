#include <pebble.h>

#include "plank.h"


static Window *window;

static TextLayer *text_layer[PERSIST_LOG_MAX];

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  window_stack_pop(true);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  int i;
  static char log[PERSIST_LOG_MAX][PERSIST_LOG_LEN]={""};
  for(i=0;i<PERSIST_LOG_MAX;i++){
    persist_read_string(NUM_PERSIST_LOG+i, log[i], PERSIST_LOG_LEN);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loading %s from %d", log[i], i);

    text_layer[i] = text_layer_create((GRect) { .origin = { 0, i*20 }, .size = { bounds.size.w, 20 } });
    text_layer_set_text(text_layer[i], log[i]);
    text_layer_set_text_alignment(text_layer[i], GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(text_layer[i]));
  }

}

static void window_unload(Window *window) {
  int i;
  for(i=0;i<PERSIST_LOG_MAX;i++){
    text_layer_destroy(text_layer[i]);
  }
}

void open_log_window_layer(void)
{
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);
}

