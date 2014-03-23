#include <pebble.h>

#include "plank.h"

#define NUM_MENU_SECTIONS 3
#define NUM_CTRL_MENU_ITEMS 1
#define NUM_CFG_MENU_ITEMS 4
#define NUM_OTHER_MENU_ITEMS 3

#define NUM_MAX_SETS 10
#define NUM_MAX_DO_TIME 180
#define NUM_MAX_REST_TIME 30


static Window *window;

int cfg_sets = 1;
int cfg_time = 30;
int cfg_rest = 30;
bool cfg_tic = false;

static MenuLayer *menu_layer;

// A callback is used to specify the amount of sections of menu items
// With this, you can dynamically add and remove sections
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

// Each section has a number of items;  we use a callback to specify this
// You can also dynamically add and remove items using this
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return NUM_CTRL_MENU_ITEMS;

    case 1:
      return NUM_CFG_MENU_ITEMS;

    case 2:
      return NUM_OTHER_MENU_ITEMS;

    default:
      return 0;
  }
}

// A callback is used to specify the height of the section header
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  // This is a define provided in pebble.h that you may use for the default height
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

// Here we draw what each header is
static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  switch (section_index) {
    case 0:
      menu_cell_basic_header_draw(ctx, cell_layer, "Control");
      break;

    case 1:
      // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, "Config");
      break;

    case 2:
      menu_cell_basic_header_draw(ctx, cell_layer, "Others");
      break;
  }
}

// This is the menu item draw callback where you specify what each item should look like
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {

  int total = 0;
  char num[30] = "";
  // Determine which section we're going to draw in
  switch (cell_index->section) {
    case 0:
      switch (cell_index->row) {
        case 0:
          // There is title draw for something more simple than a basic menu item
          menu_cell_title_draw(ctx, cell_layer, "Start");
          break;
      }
      break;
    case 1:
      // Use the row to specify which item we'll draw
      switch (cell_index->row) {
        case 0:
          snprintf(num, 30, "%d", cfg_sets);
          menu_cell_basic_draw(ctx, cell_layer, "Set(s)", num, NULL);
          break;

        case 1:
          snprintf(num, 30, "%ds", cfg_time);
          menu_cell_basic_draw(ctx, cell_layer, "Time(sec.)", num, NULL);
          break;

        case 2:
          snprintf(num, 20, "%ds", cfg_rest);
          menu_cell_basic_draw(ctx, cell_layer, "Pause Time(sec.)", num, NULL);
          break;

        case 3:
          if(cfg_tic){
            menu_cell_basic_draw(ctx, cell_layer, "With tic", "ON", NULL);
          }
          else{
            menu_cell_basic_draw(ctx, cell_layer, "With tic", "OFF", NULL);
          }
          break;
      }
      break;

    case 2:
      switch (cell_index->row) {
        case 0:
          total = (cfg_rest+cfg_time)*cfg_sets*3;
          snprintf(num, 30, "%2dm%2ds", total/60, total%60);
          menu_cell_basic_draw(ctx, cell_layer, "Estimated time", num, NULL);
          break;
        case 1:
          // There is title draw for something more simple than a basic menu item
          menu_cell_title_draw(ctx, cell_layer, "Log");
          break;
        case 2:
          // There is title draw for something more simple than a basic menu item
          menu_cell_title_draw(ctx, cell_layer, "About");
          break;
      }
      break;
  }
}

// Here we capture when a user selects a menu item
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {

  APP_LOG(APP_LOG_LEVEL_DEBUG, "menu_select_callback : sel=%d, row=%d", cell_index->section, cell_index->section);
  // Use the row to specify which item will receive the select action
  switch (cell_index->section){
    case 0:
      switch (cell_index->section) {
        case 0:
          // GO
          persist_write_int(NUM_PERSIST_SETS, cfg_sets);
          persist_write_int(NUM_PERSIST_TIME, cfg_time);
          persist_write_int(NUM_PERSIST_REST, cfg_rest);
          persist_write_bool(NUM_PERSIST_TIC,  cfg_tic);

          open_go_window_layer();
          break;
      }
    break;

    case 1:
      switch (cell_index->row) {
        case 0:
          cfg_sets %= NUM_MAX_SETS;
          cfg_sets += 1;
          break;
        case 1:
          cfg_time %= NUM_MAX_DO_TIME;
          cfg_time += 5;
          break;
        case 2:
          cfg_rest %= NUM_MAX_REST_TIME;
          cfg_rest += 5;
          break;
        case 3:
          cfg_tic = !cfg_tic;
          break;
      }
      // After changing , mark the layer to have it updated
      layer_mark_dirty(menu_layer_get_layer(menu_layer));
    break;

    case 2:
      switch (cell_index->row) {
        case 1:
          // LOG
          open_log_window_layer();
          break;
        case 2:
          // ABOUT
          open_about_window_layer();
          break;
      }
    break;
  }

}

void menu_select_long_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {

  APP_LOG(APP_LOG_LEVEL_DEBUG, "menu_select_long_callback : sel=%d, row=%d", cell_index->section, cell_index->section);
  // Use the row to specify which item will receive the select action
  switch (cell_index->section){
    case 1:
      switch (cell_index->row) {
        case 0:
          if(cfg_sets > 1){
            cfg_sets--;
          }
          else{
            cfg_sets = NUM_MAX_SETS;
          }
          break;
        case 1:
          if(cfg_time > 5){
            cfg_time -= 5;
          }
          else{
            cfg_time = NUM_MAX_DO_TIME;
          }
          break;
        case 2:
          if(cfg_rest > 5){
            cfg_rest -= 5;
          }
          else{
            cfg_rest = NUM_MAX_REST_TIME;
          }
          break;
        case 3:
          cfg_tic = !cfg_tic;
          break;
      }
      // After changing , mark the layer to have it updated
      layer_mark_dirty(menu_layer_get_layer(menu_layer));
    break;

  }

}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the menu layer
  menu_layer = menu_layer_create(bounds);

  // Set all the callbacks for the menu layer
  menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
    .select_long_click = menu_select_long_callback,
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(menu_layer, window);

  // Add it to the window for display
  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

static void window_unload(Window *window) {
  //text_layer_destroy(text_layer);

  // Destroy the menu layer
  menu_layer_destroy(menu_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  cfg_sets = persist_exists(NUM_PERSIST_SETS) ? persist_read_int(NUM_PERSIST_SETS) : 1;
  cfg_time = persist_exists(NUM_PERSIST_TIME) ? persist_read_int(NUM_PERSIST_TIME) : 30;
  cfg_rest = persist_exists(NUM_PERSIST_REST) ? persist_read_int(NUM_PERSIST_REST) : 30;
  cfg_tic  = persist_exists(NUM_PERSIST_TIC)  ? persist_read_bool(NUM_PERSIST_TIC)  : false;

  const bool animated = true;
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
