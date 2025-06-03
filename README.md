# Key debouncer library for ESP-IDF

Debounce mechanical keys connected to ESP32/ESP8266 pins and either VCC or GND entirely in software - no additional components needed.

## How to include into your projects

If you use PlatformIO, just add the GIT repository to your list of `lib_deps`. In all other cases, it's probably easiest if
you just copy the two header files and the one C file that contains the whole code.

## How to use

The library supports as many keys as you can connect, and while you can poll the key states you should probably use the event loop instead. It's really
simple; here's an example.

```
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
```
Check out `key_debouncer.h` for detailed documentation, and check out the ESP-IDF reference manual on how to use the
event loop. But it's really so simple that you should get away with just modifying the above example.

## Legal stuff

This library is totally free of borrowed stuff. It is published under the
WTFPL license version 2. If you don't know what that means, check out https://www.wtfpl.net/

While you are permitted to do with my code whatever the fuck you like, I would
still appreciate if you mention who wrote it originally and where you found it. :-)
But you don't have to. In fact it's pretty simple and very basic code; a bunch of
carefully trained monkeys or even a mechanical engineer could write it, but both
wouldn't understand why they need it.

## About the author

I am an electrical engineer who actually worked in the trade - during my studies.
After that, I found it more remunerative to do networking and software projects.
So remunerative in fact that I can now spend time on one of my hobbies, playing with
microcontrollers and all kinds of home-grown electronics. I am also a football
referee (that's what perverts call soccer).

If you want to get in touch, just write to c@pf-b.de.

Summer 2025

Chris Blum