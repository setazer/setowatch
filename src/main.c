#include <pebble.h>
Window *window;  
TextLayer *time_layer, *date_layer, *wday_layer;
GBitmap *bt_conn, *bt_disconn, *batt_low, *batt_charg;
BitmapLayer *bt_layer, *batt_layer;
InverterLayer *inv_layer;
char timebuffer[] = "00:00";
char datebuffer[] = "00.00";

static void batt_handler(BatteryChargeState state)
{
 if ((state.is_charging == true)&&(state.charge_percent<100)){
    bitmap_layer_set_bitmap(batt_layer, batt_charg);
  }
  else if (state.charge_percent<30){
      bitmap_layer_set_bitmap(batt_layer, batt_low);
  }
  else
  {
   bitmap_layer_set_bitmap(batt_layer, NULL); 
  }
}
static void bt_handler(bool connected)
{
  if(connected == true){
    bitmap_layer_set_bitmap(bt_layer, bt_conn);
  }
  else {
    bitmap_layer_set_bitmap(bt_layer, bt_disconn);
      vibes_short_pulse();
  } 
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  strftime(timebuffer, sizeof("00:00"), "%H:%M", tick_time);
  strftime(datebuffer, sizeof("00.00"), "%e.%m", tick_time);

  text_layer_set_text(time_layer, timebuffer);
  text_layer_set_text(date_layer, datebuffer);
  switch (tick_time->tm_wday) {
    case 0:
    text_layer_set_text(wday_layer,"ВС");
    break;
     case 1:
    text_layer_set_text(wday_layer,"ПН");
    break;
     case 2:
    text_layer_set_text(wday_layer,"ВТ");
    break;
     case 3:
    text_layer_set_text(wday_layer,"СР");
    break;
     case 4:
    text_layer_set_text(wday_layer,"ЧТ");
    break;
     case 5:
    text_layer_set_text(wday_layer,"ПТ");
    break;
     case 6:
    text_layer_set_text(wday_layer,"СБ");
    break;
    };
    
    
  if ((tick_time->tm_min==0) && (tick_time->tm_sec==0) && (tick_time->tm_hour>7) && (tick_time->tm_hour<18) )
    {
    vibes_double_pulse();
    }
}

void window_load(Window *window)
{
  ResHandle font_handle42 = resource_get_handle(RESOURCE_ID_IMAGINE_42);
  ResHandle fontv_handle42 = resource_get_handle(RESOURCE_ID_VISITOR_42);
  bt_conn = gbitmap_create_with_resource(RESOURCE_ID_BT_CONNECTED);
  bt_disconn = gbitmap_create_with_resource(RESOURCE_ID_BT_DISCONNECTED);
  batt_low = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_LOW);
  batt_charg = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGING);
    
  bt_layer = bitmap_layer_create(GRect(5,132,32,32));
  batt_layer = bitmap_layer_create(GRect(107,132,32,32));
  
    time_layer = text_layer_create(GRect(2, 6, 144, 162));
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, GColorBlack);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_font(time_layer,fonts_load_custom_font(font_handle42));
    
    date_layer = text_layer_create(GRect(2, 62, 144, 106));
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_color(date_layer, GColorBlack);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  text_layer_set_font(date_layer,fonts_load_custom_font(font_handle42));
    
    wday_layer = text_layer_create(GRect(2, 118, 144, 106));
  text_layer_set_background_color(wday_layer, GColorClear);
  text_layer_set_text_color(wday_layer, GColorBlack);
  text_layer_set_text_alignment(wday_layer, GTextAlignmentCenter);
  text_layer_set_font(wday_layer,fonts_load_custom_font(fontv_handle42));
  if(bluetooth_connection_service_peek()==true){
    bitmap_layer_set_bitmap(bt_layer, bt_conn);
  }
  else {
    bitmap_layer_set_bitmap(bt_layer, bt_disconn);
  }
  BatteryChargeState state = battery_state_service_peek();
  if ((state.is_charging == true)&&(state.charge_percent<100)){
    bitmap_layer_set_bitmap(batt_layer, batt_charg);
  }
  else if (state.charge_percent<30){
      bitmap_layer_set_bitmap(batt_layer , batt_low);
  }
  layer_add_child(window_get_root_layer(window), (Layer*) time_layer);
  layer_add_child(window_get_root_layer(window), (Layer*) date_layer);
  layer_add_child(window_get_root_layer(window), (Layer*) wday_layer);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(bt_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(batt_layer));
   //Get a time structure so that the face doesn't start blank
  struct tm *t;
  time_t temp;
  temp = time(NULL);
  t = localtime(&temp);
   
  //Manually call the tick handler when the window is loading
  tick_handler(t, MINUTE_UNIT);
    inv_layer = inverter_layer_create(GRect(0, 56, 144, 56));
  
  layer_add_child(window_get_root_layer(window), (Layer*) inv_layer);
}
 
void window_unload(Window *window)
{
  gbitmap_destroy(bt_conn);
  gbitmap_destroy(bt_disconn);
  gbitmap_destroy(batt_low);
  gbitmap_destroy(batt_charg);
  bitmap_layer_destroy(bt_layer);
  bitmap_layer_destroy(batt_layer);
  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(wday_layer);
  inverter_layer_destroy(inv_layer);
}  
  
void init()
{
  //Initialize the app elements here!
  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler);
  battery_state_service_subscribe(batt_handler);
  bluetooth_connection_service_subscribe(bt_handler);
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);
}
 
void deinit()
{
  //De-initialize elements here to save memory!
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  window_destroy(window);  
}
  
int main(void)
{
  init();
  app_event_loop();
  deinit();
}