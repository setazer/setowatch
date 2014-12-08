#include <pebble.h>
#include <math.h>
  
#define KEY_LANG 0
#define KEY_BATTBAR 0

Window *window; 
//Слои
TextLayer *time_layer, *date_layer, *wday_layer;
//Картинки блютуза и батареи
GBitmap *bt_disconn, *batt_low, *batt_charg, *batt_full;
//Слои картинок
BitmapLayer *bt_layer, *batt_layer;
//static Layer *batp_layer;
//Инвертер (инвертирует пиксели в заданной области)
InverterLayer *inv_layer;
unsigned int now_sec, now_min, now_hour, battery_percent;
  
char timebuffer[] = "00:00";
char datebuffer[] = "00.00";
char *dow[7];
char cur_day[2];

static void in_recv_handler(DictionaryIterator *iterator, void *context)
{
  //Get Tuple
  Tuple *t = dict_read_first(iterator);
  if(t)
  {
    switch(t->key)
    {
    case KEY_LANG:
      if(strcmp(t->value->cstring, "ru") == 0)
      {
        dow[0]="ВС";
        dow[1]="ПН";
        dow[2]="ВТ";
        dow[3]="СР";
        dow[4]="ЧТ";
        dow[5]="ПТ";
        dow[6]="СБ";
        persist_write_bool(KEY_LANG, true);
      }
      else if(strcmp(t->value->cstring, "en") == 0)
      {
        dow[0]="SU";
        dow[1]="MO";
        dow[2]="TU";
        dow[3]="WE";
        dow[4]="TH";
        dow[5]="FR";
        dow[6]="SA"; 
        persist_write_bool(KEY_LANG, false);
      }
      break;
    }
  }
}


//static void batp_layer_update_callback(Layer *me, GContext *ctx) {
//  (void)me;
//  graphics_context_set_stroke_color(ctx,GColorBlack);
//  graphics_draw_line(ctx, GPoint(71-round(7*battery_percent),0), GPoint(72+round(7*battery_percent),0));
//}

//Обработка изменения состояния батареи (подключение/отключение зарядки, изменение заряда на 10%)
static void batt_handler(BatteryChargeState state)
{
  battery_percent = round(round(state.charge_percent)/10);
  //layer_mark_dirty(batp_layer);
  if ((state.is_charging == true)&&(state.charge_percent<100)){
    bitmap_layer_set_bitmap(batt_layer, batt_charg);
  }
  else if ((state.is_charging == true)&&(state.charge_percent == 100)){
    bitmap_layer_set_bitmap(batt_layer, batt_full);
    vibes_double_pulse();
  }
  else if (state.charge_percent<30){
      bitmap_layer_set_bitmap(batt_layer, batt_low);
  }
  else
  {
   bitmap_layer_set_bitmap(batt_layer, NULL); 
  }
}

//Обработка изменения состояния блютуза (подключен/отключен)
static void bt_handler(bool connected)
{
  if(connected == true){
    bitmap_layer_set_bitmap(bt_layer, NULL);
  }
  else {
    bitmap_layer_set_bitmap(bt_layer, bt_disconn);
      vibes_short_pulse();
  } 
}

//Обработка "тика". В данном коде срабатывает каждую минуту
void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  //Форматирование буферов даты/времени
  strftime(timebuffer, sizeof("00:00"), "%H:%M", tick_time);
  strftime(datebuffer, sizeof("00.00"), "%d.%m", tick_time);
  //Вывод даты/времени
  text_layer_set_text(time_layer, timebuffer);
  text_layer_set_text(date_layer, datebuffer);
  
  //Вывод дня недели
  text_layer_set_text(wday_layer,dow[tick_time->tm_wday]);

  //Уведомительная вибрация (начало раб дня, конец раб дня, обед, конец обеда, пора домой)
  now_sec = tick_time->tm_sec;
  now_min = tick_time->tm_min;
  now_hour = tick_time->tm_hour;
  if ((now_sec==0) && 
      (((now_min==0)&&((now_hour==8)||(now_hour==13)||(now_hour==17)))||
        ((now_min==25)&&(now_hour==11))||
        ((now_min==48)&&(now_hour==16))))
    {
    vibes_double_pulse();
    }
}

//Обработка загрузки "окна"
void window_load(Window *window)
{
  //Инициализация ресурсов (шрифты/картинки блютуза/батареи)
  ResHandle font_handle42 = resource_get_handle(RESOURCE_ID_IMAGINE_42);
  ResHandle fontv_handle42 = resource_get_handle(RESOURCE_ID_VISITOR_62);
  bt_disconn = gbitmap_create_with_resource(RESOURCE_ID_BT_DISCONNECTED);
  batt_low = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_LOW);
  batt_charg = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGING);
  batt_full = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGED);
  //Создание слоёв батареи и блютуза  
  bt_layer = bitmap_layer_create(GRect(5,124,32,32));
  batt_layer = bitmap_layer_create(GRect(107,124,32,32));
  //batp_layer = layer_create(GRect(2,110,140,1));
  //Слой времени
    time_layer = text_layer_create(GRect(4, 56, 144, 56));
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, GColorBlack);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_font(time_layer,fonts_load_custom_font(font_handle42));
  //Слой даты  
    date_layer = text_layer_create(GRect(4, 0, 144, 56));
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_color(date_layer, GColorBlack);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  text_layer_set_font(date_layer,fonts_load_custom_font(font_handle42));
  //Слой дня недели    
    wday_layer = text_layer_create(GRect(4, 100, 144, 56));
  text_layer_set_background_color(wday_layer, GColorClear);
  text_layer_set_text_color(wday_layer, GColorBlack);
  text_layer_set_text_alignment(wday_layer, GTextAlignmentCenter);
  text_layer_set_font(wday_layer,fonts_load_custom_font(fontv_handle42));
  //Начальное состояние блютуза
  bt_handler(bluetooth_connection_service_peek());
  //Начальное состояние батареи
  batt_handler(battery_state_service_peek());
  //layer_mark_dirty(batp_layer);
   //"Вывод" слоёв в окно
  layer_add_child(window_get_root_layer(window), (Layer*) time_layer);
  layer_add_child(window_get_root_layer(window), (Layer*) date_layer);
  layer_add_child(window_get_root_layer(window), (Layer*) wday_layer);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(bt_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(batt_layer));
  //layer_set_update_proc(batp_layer,batp_layer_update_callback);
  
  //layer_add_child(window_get_root_layer(window), batp_layer);
  
  
  
  bool lang_rus = persist_read_bool(KEY_LANG);
  //bool show_bb = persist_read_bool(KEY_BATTBAR);
  
  if (lang_rus==true) {
        dow[0]="ВС";
        dow[1]="ПН";
        dow[2]="ВТ";
        dow[3]="СР";
        dow[4]="ЧТ";
        dow[5]="ПТ";
        dow[6]="СБ";
  }
  else {
        dow[0]="SU";
        dow[1]="MO";
        dow[2]="TU";
        dow[3]="WE";
        dow[4]="TH";
        dow[5]="FR";
        dow[6]="SA"; 
  }
   //Ручное начальнок срабатывание "тика", чтобы время не было пустым при открытии
  struct tm *t;
  time_t temp;
  temp = time(NULL);
  t = localtime(&temp);
  tick_handler(t, MINUTE_UNIT);
  //Создание и вывод инвертера
  inv_layer = inverter_layer_create(GRect(0, 56, 144, 56));
  layer_add_child(window_get_root_layer(window), (Layer*) inv_layer);
}

//Обработка выгрузки окна 
void window_unload(Window *window)
{
  //Освобождение памяти от ресурсов
  gbitmap_destroy(bt_disconn);
  gbitmap_destroy(batt_low);
  gbitmap_destroy(batt_charg);
  gbitmap_destroy(batt_full);
  bitmap_layer_destroy(bt_layer);
  bitmap_layer_destroy(batt_layer);
  //layer_destroy(batp_layer);
  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(wday_layer);
  inverter_layer_destroy(inv_layer);
}  

//Инициализация
void init()
{
  //Подписка на события ежеминутного "тика" и смены состояний батареи/блютуза
  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler);
  battery_state_service_subscribe(batt_handler);
  bluetooth_connection_service_subscribe(bt_handler);
  //Создание окна и задание обработчиков
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  //Вывод окна
  window_stack_push(window, true);
  app_message_register_inbox_received((AppMessageInboxReceived) in_recv_handler);
app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

//Деинициализация
void deinit()
{
  //Отписка от событий и уничтожение окна
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  window_destroy(window);  
}

//Основная процедура
int main(void)
{
  init();
  app_event_loop();
  deinit();
}