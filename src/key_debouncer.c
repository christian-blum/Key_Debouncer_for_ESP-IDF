/*

Key debouncer library for ESP-IDF. Should work with all kinds of ESP32 variants.
Check out key_debouncer.h (the only header file you need to include) for documentation.
For every question that file doesn't answer for you, check out this one.

This library is published under the WTFPL license version 2. If you don't know
what that means, check out https://www.wtfpl.net/

While you are permitted to do with my code whatever the fuck you like, I would
still appreciate if you mention who wrote it originally and where you found it. :-)
But you don't have to. In fact it's pretty simple and very basic code; a bunch of
carefully trained monkeys or even a mechanical engineer could write it, but both
wouldn't understand why they need it.

Christian Blum, c@pf-b.de, Summer 2025

*/


#include "key_debouncer.h"
#include "key_debouncer_internals.h"

#include <esp_intr_alloc.h>
#include <esp_timer.h>
#include <esp_err.h>
#include <esp_log.h>

#include <string.h>

static const char *TAG = "KEY_DEBOUNCER";

static esp_event_loop_handle_t event_loop;
static key_debouncer_t key[GPIO_NUM_MAX];
static esp_timer_handle_t timer;

#define IS_PRESSED(key) ((gpio_get_level((key)->pin) != 0) ^ ((key)->active_low))

ESP_EVENT_DEFINE_BASE(KEY_DEBOUNCER_EVENT);

void reschedule_timer(time_t now) {
  time_t next_timeout = LONG_LONG_MAX;
  key_debouncer_t *k = &key[0];
  for (int i = 0; i < GPIO_NUM_MAX; i++, k++) {
    if (k->debounce_timeout && k->debounce_timeout < next_timeout) next_timeout = k->debounce_timeout;
    if (k->longpress_timeout && k->longpress_timeout < next_timeout) next_timeout = k->longpress_timeout;
    if (k->repeat_timeout && k->repeat_timeout < next_timeout) next_timeout = k->repeat_timeout;
  }
  time_t next_scheduled_timeout = esp_timer_get_next_alarm();
  if (next_scheduled_timeout == 0 || next_scheduled_timeout > next_timeout) {
    time_t fire_in = next_timeout - now;
    esp_timer_stop(timer);
    esp_timer_start_once(timer, fire_in);
  }
}

void key_debouncer_timer_callback(void *args) {
  time_t now = esp_timer_get_time();
  key_debouncer_t *k = &key[0];
  for (int i = 0; i < GPIO_NUM_MAX; i++, k++) {
    if (k->debounce_timeout && key->debounce_timeout <= now) {
      bool state = IS_PRESSED(k);
      k->last_state = state;
      k->longpress_timeout = 0;
      k->repeat_timeout = 0;
      if (state == true) {
        esp_event_post_to(event_loop, KEY_DEBOUNCER_EVENT, KEY_DEBOUNCER_KEY_PRESSED, &k->marker, sizeof(&k->marker), 10);
        if (k->long_pressed_us) k->longpress_timeout = now + k->long_pressed_us;
      }
      else {
        esp_event_post_to(event_loop, KEY_DEBOUNCER_EVENT, KEY_DEBOUNCER_KEY_RELEASED, &k->marker, sizeof(&k->marker), 10);
      }
      k->debounce_timeout = 0;
    }
    if (k->longpress_timeout && k->longpress_timeout <= now) {
      k->longpress_timeout = 0;
      esp_event_post_to(event_loop, KEY_DEBOUNCER_EVENT, KEY_DEBOUNCER_KEY_LONG_PRESSED, &k->marker, sizeof(&k->marker), 10);
      if (k->repeat_us) k->repeat_timeout = now + k->repeat_us;
    }
    if (k->repeat_timeout && k->repeat_timeout <= now) {
      k->repeat_timeout += k->repeat_us;
      esp_event_post_to(event_loop, KEY_DEBOUNCER_EVENT, KEY_DEBOUNCER_KEY_REPEATED, &k->marker, sizeof(&k->marker), 10);
    }
  }
  reschedule_timer(now);
}

esp_err_t key_debouncer_init(esp_event_loop_handle_t *event_loop_handle_pointer) {
  esp_event_loop_args_t args = {
    .queue_size = 16,
    .task_name = "key debouncer",
    .task_priority = 5,
    .task_stack_size = 3072
  };
  esp_err_t err = esp_event_loop_create(&args, &event_loop);
  if (err) return err;
  err = gpio_install_isr_service(ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_SHARED); // if it is installed, this will simply return an error and that's fine
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Could not install GPIO ISR service, code %4.4x", err);
  }
  if (event_loop_handle_pointer) {
    *event_loop_handle_pointer = event_loop;
  }
  esp_timer_create_args_t timer_args = {
    .callback = key_debouncer_timer_callback,
    .name = "key debouncer",
    .skip_unhandled_events = true
  };
  esp_timer_create(&timer_args, &timer);
  return ESP_OK;
}

esp_err_t key_debouncer_unregister_key(gpio_num_t pin) {
  if (pin <= GPIO_NUM_NC || pin >= GPIO_NUM_MAX) return ESP_ERR_INVALID_ARG;
  key_debouncer_t *k = &key[pin];
  if (!k->pin) return ESP_ERR_INVALID_ARG; // key is not configured
  gpio_intr_disable(pin);
  gpio_isr_handler_remove(pin);
  gpio_reset_pin(pin);
  bzero(k, sizeof(k));
  return ESP_OK;
}

esp_err_t key_debouncer_set_long_pressed(gpio_num_t pin, time_t microseconds) {
  if (pin <= GPIO_NUM_NC || pin >= GPIO_NUM_MAX) return ESP_ERR_INVALID_ARG;
  if (microseconds < 0) return ESP_ERR_INVALID_ARG;
  key_debouncer_t *k = &key[pin];
  if (!k->pin) return ESP_ERR_INVALID_STATE;
  k->long_pressed_us = microseconds;
  return ESP_OK;
}

esp_err_t key_debouncer_set_repeat(gpio_num_t pin, time_t microseconds) {
  if (pin <= GPIO_NUM_NC || pin >= GPIO_NUM_MAX) return ESP_ERR_INVALID_ARG;
  if (microseconds < 0) return ESP_ERR_INVALID_ARG;
  key_debouncer_t *k = &key[pin];
  if (!k->pin) return ESP_ERR_INVALID_STATE;
  if (!k->long_pressed_us && microseconds) return ESP_ERR_INVALID_STATE;
  k->repeat_us = microseconds;
  return ESP_OK;
}

int8_t key_debouncer_get_state(gpio_num_t pin) {
  if (pin <= GPIO_NUM_NC || pin >= GPIO_NUM_MAX) return -1;
  key_debouncer_t *k = &key[pin];
  return k->last_state ? 1 : 0;
}

esp_err_t key_debouncer_deinit() {
  esp_err_t err = esp_event_loop_delete(event_loop);
  if (err == ESP_OK) {
    event_loop = NULL;
  }
  for (gpio_num_t i = 0; i < GPIO_NUM_MAX; i++) {
    key_debouncer_unregister_key(i);
  }
  return err;
}

static void key_debouncer_isr_service_handler(void *args) {
  key_debouncer_t *key = (key_debouncer_t *)args;
  time_t now = esp_timer_get_time();
  time_t timeout = now + key->debounce_us;
  key->debounce_timeout = timeout;
  key->longpress_timeout = 0;
  key->repeat_timeout = 0;
  reschedule_timer(now);
}

 esp_err_t key_debouncer_register_key(gpio_num_t pin, int32_t marker, bool active_low, time_t debounce_us, time_t long_pressed_us, time_t repeat_us) {
  if (pin <= GPIO_NUM_NC || pin >= GPIO_NUM_MAX) return ESP_ERR_INVALID_ARG;
  if (debounce_us < 0) return ESP_ERR_INVALID_ARG;
  if (long_pressed_us < 0) return ESP_ERR_INVALID_ARG;
  if (repeat_us < 0 || (repeat_us > 0 && long_pressed_us == 0)) return ESP_ERR_INVALID_ARG;
  bzero(&key[pin], sizeof(key[pin]));
  key[pin].pin = pin;
  key[pin].marker = marker;
  key[pin].active_low = active_low;
  key[pin].debounce_us = debounce_us;
  key[pin].long_pressed_us = long_pressed_us;
  key[pin].repeat_us = repeat_us;

  // set up the GPIO
  gpio_reset_pin(pin);
  gpio_set_direction(pin, GPIO_MODE_INPUT);
  gpio_input_enable(pin);
  gpio_set_pull_mode(pin, active_low ? GPIO_PULLUP_ONLY : GPIO_PULLDOWN_ONLY);
  key[pin].last_state = IS_PRESSED(&key[pin]);
  // set up the interrupt
  esp_err_t err;
  err = gpio_isr_handler_add(pin, key_debouncer_isr_service_handler, &key[pin]);
  if (err) return err;
  gpio_set_intr_type(pin, GPIO_INTR_ANYEDGE);
  gpio_intr_enable(pin);
  return err;
}
