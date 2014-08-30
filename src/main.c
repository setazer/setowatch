#include <pebble.h>
Window *window;  
TextLayer *time_layer, *date_layer, *wday_layer;
InverterLayer *inv_layer;
char timebuffer[] = "00:00";
char datebuffer[] = "00.00";
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
  //62
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
 
layer_add_child(window_get_root_layer(window), (Layer*) time_layer);
layer_add_child(window_get_root_layer(window), (Layer*) date_layer);
layer_add_child(window_get_root_layer(window), (Layer*) wday_layer);
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
  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(wday_layer);
  inverter_layer_destroy(inv_layer);
}  
  
void init()
{
  //Initialize the app elements here!
  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler);
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
window_destroy(window);  
}
  
int main(void)
{
  init();
  app_event_loop();
  deinit();
}