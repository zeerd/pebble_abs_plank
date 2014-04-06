#include <pebble.h>

#include "plank.h"

#define NUM_MENU_SECTIONS 3
#define NUM_CTRL_MENU_ITEMS 1
#define NUM_CFG_MENU_ITEMS 3
#define NUM_ADV_CFG_MENU_ITEMS 5
#define NUM_OTHER_MENU_ITEMS 5

#define NUM_MAX_SETS 10
#define NUM_MAX_DO_TIME 180
#define NUM_MAX_REST_TIME 30

#define CFG_NUM_ID_SETS 0
#define CFG_NUM_ID_TIME1 1
#define CFG_NUM_ID_TIME2 2
#define CFG_NUM_ID_TIME3 3
#define CFG_NUM_ID_REST 4

static Window *window;
static NumberWindow *num_window;

int cfg_sets = 1;
int cfg_time[3] = {30, 30, 30};
int cfg_rest = 30;
bool cfg_tic = false;
bool cfg_adv = false;

static MenuLayer *menu_layer;

static void window_load(Window *window);
static void window_unload(Window *window);

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
      if(cfg_adv){
        return NUM_ADV_CFG_MENU_ITEMS;
      }
      else{
        return NUM_CFG_MENU_ITEMS;
      }

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
static void show_cfg_menu(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data)
{
  char num[30] = "";
  if(cfg_adv){
    switch (cell_index->row) {
      case 0:
        snprintf(num, 30, "%d", cfg_sets);
        menu_cell_basic_draw(ctx, cell_layer, "Set(s)", num, NULL);
        break;

      case 1:
        snprintf(num, 30, "%dsec", cfg_time[0]);
        menu_cell_basic_draw(ctx, cell_layer, "Regular Time", num, NULL);
        break;

      case 2:
        snprintf(num, 30, "%dsec", cfg_time[1]);
        menu_cell_basic_draw(ctx, cell_layer, "Left Side Time", num, NULL);
        break;

      case 3:
        snprintf(num, 30, "%dsec", cfg_time[2]);
        menu_cell_basic_draw(ctx, cell_layer, "Right Side Time", num, NULL);
        break;

      case 4:
        snprintf(num, 30, "%dsec", cfg_rest);
        menu_cell_basic_draw(ctx, cell_layer, "Rest Time", num, NULL);
        break;
    }
  }
  else{
    switch (cell_index->row) {
      case 0:
        snprintf(num, 30, "%d", cfg_sets);
        menu_cell_basic_draw(ctx, cell_layer, "Set(s)", num, NULL);
        break;

      case 1:
        snprintf(num, 30, "%ds", cfg_time[0]);
        menu_cell_basic_draw(ctx, cell_layer, "Time(sec.)", num, NULL);
        break;

      case 2:
        snprintf(num, 30, "%ds", cfg_rest);
        menu_cell_basic_draw(ctx, cell_layer, "Rest Time(sec.)", num, NULL);
        break;
    }
  }

}

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
      show_cfg_menu(ctx, cell_layer, cell_index, data);
      break;

    case 2:
      switch (cell_index->row) {
        case 0:
          if(cfg_adv){
            total = (cfg_rest*3+cfg_time[0]+cfg_time[1]+cfg_time[2])*cfg_sets;
          }
          else{
            total = (cfg_rest+cfg_time[0])*3*cfg_sets;
          }
          snprintf(num, 30, "%2dm%2ds", total/60, total%60);
          menu_cell_basic_draw(ctx, cell_layer, "Estimated time", num, NULL);
          break;
        case 1:
          if(cfg_adv){
            menu_cell_basic_draw(ctx, cell_layer, "Advanced set", "ON", NULL);
          }
          else{
            menu_cell_basic_draw(ctx, cell_layer, "Advanced set", "OFF", NULL);
          }
          break;
        case 2:
          if(cfg_tic){
            menu_cell_basic_draw(ctx, cell_layer, "With tic", "ON", NULL);
          }
          else{
            menu_cell_basic_draw(ctx, cell_layer, "With tic", "OFF", NULL);
          }
          break;
        case 3:
          // There is title draw for something more simple than a basic menu item
          menu_cell_basic_draw(ctx, cell_layer, "Log", "Records", NULL);
          break;
        case 4:
          // There is title draw for something more simple than a basic menu item
          menu_cell_basic_draw(ctx, cell_layer, "About", "...", NULL);
          break;
      }
      break;
  }
}

void num_window_sel(struct NumberWindow *number_window, void *context)
{
  if(context == CFG_NUM_ID_SETS){
    cfg_sets = number_window_get_value(number_window);
  }
  else if((int)context == CFG_NUM_ID_TIME1){
    cfg_time[0] = number_window_get_value(number_window);
    if(!cfg_adv){
      cfg_time[1] = number_window_get_value(number_window);
      cfg_time[2] = number_window_get_value(number_window);
    }
  }
  else if((int)context == CFG_NUM_ID_TIME2){
    cfg_time[1] = number_window_get_value(number_window);
  }
  else if((int)context == CFG_NUM_ID_TIME3){
    cfg_time[2] = number_window_get_value(number_window);
  }
  else if((int)context == CFG_NUM_ID_REST){
    cfg_rest = number_window_get_value(number_window);
  }
  layer_mark_dirty(menu_layer_get_layer(menu_layer));
  window_stack_pop((Window*)num_window);
}

// Here we capture when a user selects a menu item
static void show_num_window(const char* text, int id, int min, int max, int step, int crt)
{
  NumberWindowCallbacks nwcb;
  nwcb.selected = num_window_sel;
  nwcb.decremented = NULL;
  nwcb.incremented = NULL;

  num_window = number_window_create(text, nwcb, (void*)id);

  number_window_set_min(num_window, min);
  number_window_set_max(num_window, max);
  number_window_set_step_size(num_window, step);
  number_window_set_value(num_window, crt);

  window_stack_push((Window*)num_window, true);
}
static void sel_cfg_menu(MenuLayer *menu_layer, MenuIndex *cell_index, void *data)
{

  if(num_window != NULL){
    number_window_destroy(num_window);
  }

  if(cfg_adv){
    switch (cell_index->row) {
      case 0:
        show_num_window("Set(s)", CFG_NUM_ID_SETS, 1, NUM_MAX_SETS, 1, cfg_sets);
        break;
      case 1:
        show_num_window("Regular Planks Time(sec.)", CFG_NUM_ID_TIME1, 5, NUM_MAX_DO_TIME, 5, cfg_time[0]);
        break;
      case 2:
        show_num_window("Left Side Planks Time(sec.)", CFG_NUM_ID_TIME2, 5, NUM_MAX_DO_TIME, 5, cfg_time[1]);
        break;
      case 3:
        show_num_window("Right Side Planks Time(sec.)", CFG_NUM_ID_TIME3, 5, NUM_MAX_DO_TIME, 5, cfg_time[2]);
        break;
      case 4:
        show_num_window("Rest Time(sec.)", CFG_NUM_ID_REST, 5, NUM_MAX_REST_TIME, 5, cfg_rest);
        break;
    }
  }
  else{
    switch (cell_index->row) {
      case 0:
        show_num_window("Set(s)", CFG_NUM_ID_SETS, 1, NUM_MAX_SETS, 1, cfg_sets);
        break;
      case 1:
        show_num_window("Time(sec.)", CFG_NUM_ID_TIME1, 5, NUM_MAX_DO_TIME, 5, cfg_time[0]);
        break;
      case 2:
        show_num_window("Rest Time(sec.)", CFG_NUM_ID_REST, 5, NUM_MAX_REST_TIME, 5, cfg_rest);
        break;
    }
  }
}

void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "menu_select_callback : sel=%d, row=%d", cell_index->section, cell_index->section);
  // Use the row to specify which item will receive the select action
  switch (cell_index->section){
    case 0:
      switch (cell_index->section) {
        case 0:
          // GO
          persist_write_int(NUM_PERSIST_SETS, cfg_sets);
          persist_write_int(NUM_PERSIST_TIME1, cfg_time[0]);
          persist_write_int(NUM_PERSIST_TIME2, cfg_time[1]);
          persist_write_int(NUM_PERSIST_TIME3, cfg_time[2]);
          persist_write_int(NUM_PERSIST_REST, cfg_rest);
          persist_write_bool(NUM_PERSIST_TIC,  cfg_tic);

          open_go_window_layer();
          break;
      }
    break;

    case 1:
      sel_cfg_menu(menu_layer, cell_index, data);
    break;

    case 2:
      switch (cell_index->row) {
        case 1:
          cfg_adv = !cfg_adv;
          persist_write_bool(NUM_PERSIST_ADV,  cfg_adv);
          window_unload(window);
          window_load(window);
          break;
        case 2:
          cfg_tic = !cfg_tic;
          break;
        case 3:
          // LOG
          open_log_window_layer();
          break;
        case 4:
          // ABOUT
          open_about_window_layer();
          break;
      }
    break;
  }
  layer_mark_dirty(menu_layer_get_layer(menu_layer));

}

static void window_load(Window *window) 
{
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
    .select_click = menu_select_callback
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(menu_layer, window);

  // Add it to the window for display
  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

static void window_unload(Window *window) 
{
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
  cfg_time[0] = persist_exists(NUM_PERSIST_TIME1) ? persist_read_int(NUM_PERSIST_TIME1) : 30;
  cfg_time[1] = persist_exists(NUM_PERSIST_TIME2) ? persist_read_int(NUM_PERSIST_TIME2) : cfg_time[0];
  cfg_time[2] = persist_exists(NUM_PERSIST_TIME3) ? persist_read_int(NUM_PERSIST_TIME3) : cfg_time[0];
  cfg_rest = persist_exists(NUM_PERSIST_REST) ? persist_read_int(NUM_PERSIST_REST) : 30;
  cfg_tic  = persist_exists(NUM_PERSIST_TIC)  ? persist_read_bool(NUM_PERSIST_TIC)  : false;
  cfg_adv  = persist_exists(NUM_PERSIST_ADV)  ? persist_read_bool(NUM_PERSIST_ADV)  : false;

  const bool animated = true;
  window_stack_push(window, animated);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
}

static void deinit(void) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done deinitializing, destroy window: %p", window);
  if(num_window != NULL){
    number_window_destroy(num_window);
  }
  window_destroy(window);
  close_go_window_layer();
  close_log_window_layer();
  close_about_window_layer();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
