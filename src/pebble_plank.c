#include <pebble.h>

#define NUM_MENU_SECTIONS 3
#define NUM_CTRL_MENU_ITEMS 1
#define NUM_CFG_MENU_ITEMS 5
#define NUM_OTHER_MENU_ITEMS 2

#define NUM_PERSIST_SETS 1
#define NUM_PERSIST_TIME 2
#define NUM_PERSIST_REST 3
#define NUM_PERSIST_TIC  4


#define NUM_MAX_SETS 10
#define NUM_MAX_DO_TIME 180
#define NUM_MAX_REST_TIME 30

#define SET_STEP_READY 0
#define SET_STEP_REGULAR 1
#define SET_STEP_REST_LEFT 2
#define SET_STEP_SIDE_LEFT 3
#define SET_STEP_REST_RIGHT 4
#define SET_STEP_SIDE_RIGHT 5
#define SET_STEP_REST_NEXT_SET 6
#define SET_STEP_MAX 7

static Window *window;
static Window *window_go;
static Window *window_log;
static Window *window_about;

static int cfg_sets = 1;
static int cfg_time = 5;
static int cfg_rest = 5;
static int cfg_tic = 0;
static int go_set_step = 0;
static int go_set_crt = 0;
static int step_timer = 0;

static TextLayer *text_layer;
static TextLayer *text_layer_timer;
static MenuLayer *menu_layer;

static BitmapLayer *image_layer;
static GBitmap *image;

static AppTimer *timer;

static void window_go_load(Window *window);
static void window_go_unload(Window *window);
static void window_log_load(Window *window);
static void window_log_unload(Window *window);
static void window_about_load(Window *window);
static void window_about_unload(Window *window);


//static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Select");
//}

//static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Up");
//}

//static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Down");
//}

//static void click_config_provider(void *context) {
  //window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  //window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  //window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
//}

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

  char num[10] = "";
  // Determine which section we're going to draw in
  switch (cell_index->section) {
    case 0:
      switch (cell_index->row) {
        case 0:
          // There is title draw for something more simple than a basic menu item
          menu_cell_title_draw(ctx, cell_layer, "GO");
          break;
      }
      break;
    case 1:
      // Use the row to specify which item we'll draw
      switch (cell_index->row) {
        case 0:
          snprintf(num, 10, "%d", cfg_sets);
          menu_cell_basic_draw(ctx, cell_layer, "Set(s)", num, NULL);
          break;

        case 1:
          snprintf(num, 10, "%d", cfg_time);
          menu_cell_basic_draw(ctx, cell_layer, "Time(sec.)", num, NULL);
          break;

        case 2:
          snprintf(num, 10, "%d", cfg_rest);
          menu_cell_basic_draw(ctx, cell_layer, "Pause Time(sec.)", num, NULL);
          break;

        case 3:
          if(cfg_tic == 0){
            menu_cell_basic_draw(ctx, cell_layer, "With tic", "OFF", NULL);
          }
          else{
            menu_cell_basic_draw(ctx, cell_layer, "With tic", "ON", NULL);
          }
          break;

        case 4:
          snprintf(num, 10, "%d", (cfg_rest+cfg_time)*cfg_sets);
          menu_cell_basic_draw(ctx, cell_layer, "Estimed time(s)", num, NULL);
          break;
      }
      break;

    case 2:
      switch (cell_index->row) {
        case 0:
          // There is title draw for something more simple than a basic menu item
          menu_cell_title_draw(ctx, cell_layer, "LOG");
          break;
        case 1:
          // There is title draw for something more simple than a basic menu item
          menu_cell_title_draw(ctx, cell_layer, "ABOUT");
          break;
      }
      break;
  }
}

// Here we capture when a user selects a menu item
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
  switch (cell_index->section){
    case 0:
      switch (cell_index->row) {
        case 0:
          // GO
          persist_write_int(NUM_PERSIST_SETS, cfg_sets);
          persist_write_int(NUM_PERSIST_TIME, cfg_time);
          persist_write_int(NUM_PERSIST_REST, cfg_rest);
          persist_write_int(NUM_PERSIST_TIC,  cfg_tic);

          window_go = window_create();
          //window_set_click_config_provider(window_go, click_config_provider);
          window_set_window_handlers(window_go, (WindowHandlers) {
            .load = window_go_load,
            .unload = window_go_unload,
          });
          window_stack_push(window_go, true);
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
          cfg_tic = (cfg_tic + 1) % 2;
          break;
      }
      // After changing , mark the layer to have it updated
      layer_mark_dirty(menu_layer_get_layer(menu_layer));
    break;

    case 2:
      switch (cell_index->row) {
        case 0:
          // LOG
          window_log = window_create();
          //window_set_click_config_provider(window_log, click_config_provider);
          window_set_window_handlers(window_log, (WindowHandlers) {
            .load = window_log_load,
            .unload = window_log_unload,
          });
          window_stack_push(window_log, true);
          break;
        case 1:
          // ABOUT
          window_about = window_create();
          //window_set_click_config_provider(window_about, click_config_provider);
          window_set_window_handlers(window_about, (WindowHandlers) {
            .load = window_about_load,
            .unload = window_about_unload,
          });
          window_stack_push(window_about, true);
          break;
      }
    break;
  }

}

static void timer_callback(void *context) {

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
        text_layer_set_text(text_layer, "Rest");
        image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_REGULAR);
        break;
      case SET_STEP_REGULAR:
        text_layer_set_text(text_layer, "Regular Planks");
        image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_REGULAR);
        break;
      case SET_STEP_REST_NEXT_SET:
        text_layer_set_text(text_layer, "Rest");
        image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_REGULAR);
        break;
      case SET_STEP_REST_LEFT:
        text_layer_set_text(text_layer, "Rest");
        image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SIDE_LEFT);
        break;
      case SET_STEP_SIDE_LEFT:
        text_layer_set_text(text_layer, "Left Side Planks");
        image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SIDE_LEFT);
        break;
      case SET_STEP_REST_RIGHT:
        text_layer_set_text(text_layer, "Rest");
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
  }
}

static void window_go_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 10 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "READY");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  //const GSize size={20,20};
  text_layer_timer = text_layer_create((GRect) { .origin = { 0, 110 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer_timer, "TIMER");
  //text_layer_set_size(text_layer_timer, size);
  text_layer_set_text_alignment(text_layer_timer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer_timer));

  // This needs to be deinited on app exit which is when the event loop ends
  image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_REGULAR);

  // The bitmap layer holds the image for display
  image_layer = bitmap_layer_create((GRect) { .origin = { 0, 30 }, .size = { bounds.size.w, 80 } });
  bitmap_layer_set_bitmap(image_layer, image);
  bitmap_layer_set_alignment(image_layer, GAlignCenter);
  layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));

  go_set_step = SET_STEP_READY;
  step_timer = 5;
  go_set_crt = 0;

  timer = app_timer_register(1000, timer_callback, NULL);
}

static void window_go_unload(Window *window) {
  text_layer_destroy(text_layer);
  text_layer_destroy(text_layer_timer);

  gbitmap_destroy(image);
  bitmap_layer_destroy(image_layer);

  app_timer_cancel(timer);
}

static void window_log_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Coming soon...");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_log_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void window_about_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, bounds.size.h } });
  text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text(text_layer, "http://www.zeerd.com Ver 0.1.0");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_about_unload(Window *window) {
  text_layer_destroy(text_layer);
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
    .select_long_click = menu_select_callback,
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
  //window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  cfg_sets = persist_exists(NUM_PERSIST_SETS) ? persist_read_int(NUM_PERSIST_SETS) : 1;
  cfg_time = persist_exists(NUM_PERSIST_TIME) ? persist_read_int(NUM_PERSIST_TIME) : 30;
  cfg_rest = persist_exists(NUM_PERSIST_REST) ? persist_read_int(NUM_PERSIST_REST) : 30;
  cfg_tic  = persist_exists(NUM_PERSIST_TIC)  ? persist_read_int(NUM_PERSIST_TIC)  : 0;

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
