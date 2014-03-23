#include <pebble.h>

#include "plank.h"
#include "../build/generated/appinfo.h"

static Window *window;

static TextLayer *text_layer;
static TextLayer *text_layer_url;
static TextLayer *text_layer_ver;

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  window_stack_pop(true);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, bounds.size.h } });
  text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text(text_layer, "This app helps you to make plank exercices to build your abs. Long press for decrease in config.");
  text_layer_set_text_alignment(text_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  text_layer_url = text_layer_create((GRect) { .origin = { 0, bounds.size.h-40 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer_url, "http://www.zeerd.com");
  text_layer_set_text_alignment(text_layer_url, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer_url));

  text_layer_ver = text_layer_create((GRect) { .origin = { 0, bounds.size.h-20 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer_ver, "Ver "VERSION_LABEL);
  text_layer_set_text_alignment(text_layer_ver, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer_ver));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  text_layer_destroy(text_layer_url);
  text_layer_destroy(text_layer_ver);
}

void open_about_window_layer(void)
{
  close_about_window_layer();

  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);
}

void close_about_window_layer(void)
{
  if (window != NULL){
    window_destroy(window);
  }
}
