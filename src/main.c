#if KEY_DEBOUNCER_STANDALONE

/*

This is just testing and example code. It won't make it into your project.

*/

#include <esp_log.h>
#include <esp_err.h>

#include "key_debouncer.h"

esp_event_loop_handle_t event_loop;

static const char *TAG = "MAIN";


void key_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  if (event_base != KEY_DEBOUNCER_EVENT) {
    ESP_LOGE(TAG, "wrong event base: %s", event_base);
    return;
  }
  int32_t marker = *((int32_t *) event_data);
  switch (event_id) {
    case KEY_DEBOUNCER_KEY_PRESSED:
      ESP_LOGI(TAG, "Key pressed: %ld", marker);
      break;
    case KEY_DEBOUNCER_KEY_RELEASED:
      ESP_LOGI(TAG, "Key released: %ld", marker);  
      break;
    case KEY_DEBOUNCER_KEY_LONG_PRESSED:
      ESP_LOGI(TAG, "Key long-pressed: %ld", marker);  
      break;
    case KEY_DEBOUNCER_KEY_REPEATED:
      ESP_LOGI(TAG, "Key repeated: %ld", marker);
      break;
    default:
      ESP_LOGI(TAG, "sporadic event id %ld", event_id);
  }
}

void app_main() {
  key_debouncer_init(&event_loop);
  key_debouncer_register_key(10, 10, true, 1000, 500000, 200000);
  key_debouncer_register_key(11, 11, true, 1000, 500000, 200000);
  key_debouncer_register_key(12, 12, true, 1000, 500000, 200000);
  key_debouncer_register_key(13, 13, true, 1000, 500000, 200000);
  key_debouncer_register_key(14, 14, true, 1000, 500000, 200000);
  esp_event_handler_instance_register_with(event_loop, KEY_DEBOUNCER_EVENT, ESP_EVENT_ANY_ID, key_handler, NULL, NULL);
}

#endif