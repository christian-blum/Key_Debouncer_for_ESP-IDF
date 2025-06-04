#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"

struct key_debouncer_s {
  bool configured;
  gpio_num_t pin; // may seem silly but the ISR needs to know
  int32_t marker;
  bool active_low;
  time_t debounce_us;
  time_t long_pressed_us;
  time_t repeat_us;
  gpio_isr_handle_t isr_handle;
  bool last_state; // true = pressed, false = not pressed
  time_t debounce_timeout;
  time_t longpress_timeout;
  time_t repeat_timeout;
};

typedef struct key_debouncer_s key_debouncer_t;