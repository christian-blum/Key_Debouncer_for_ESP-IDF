#pragma once

/* This header file contains everything you need to use the library. */

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_event_base.h"
#include "esp_event.h"



ESP_EVENT_DECLARE_BASE(KEY_DEBOUNCER_EVENT);

typedef enum {
  KEY_DEBOUNCER_KEY_PRESSED = 1,
  KEY_DEBOUNCER_KEY_LONG_PRESSED = 2,
  KEY_DEBOUNCER_KEY_REPEATED = 3,
  KEY_DEBOUNCER_KEY_RELEASED = 4
} key_debouncer_event_t;


/**
 * @brief Initialize the key debouncer library.
 * 
 * Call this function to set everything up behind the scenes.
 * 
 * @param[in] event_loop_handle_pointer a pointer to an esp_event_loop_handle_t so you know where to register event handlers with, or NULL if you are not interested.
 *
 * @return
 *  - ESP_OK: Success
 *  - Others: Fail
 */

esp_err_t key_debouncer_init(esp_event_loop_handle_t *event_loop_handle_pointer);


/**
 * @brief Deinitialize the key debouncer library.
 * 
 * Call this function end key debouncing and free up all resources.
 * 
 * @return
 *  - ESP_OK: Success
 *  - Others: Fail
 */

esp_err_t key_debouncer_deinit();


/**
 * @brief Register a key connected to a pin.
 *
 * @param[in] pin The GPIO number the key is connected to
 * @param[in] marker An arbitrary number you get back when you receive events (probably use the same as the pin number)
 * @param[in] active_low Whether or not the input is active low, i.e. you've wired the key to GND instead of VCC
 * @param[in] debounce_us Number of microseconds the pin state needs to remain the same before it is reported
 *                        (10000 is a good value if you don't know what to use here; some fat switches may need more,
 *                        and with good switches you may get away with much less. Yes, you can use 0, but why are
 *                        you using this library then?!)
 * @param[in] long_press_us The number of microseconds after which a press is a long-press (and reported as such)
 * @param[in] repeat_us The number of microseconds after a long-press after which a repeat event is sent over and over again
 *                      until the key is released
 * 
 * It's totally fine to pass 0 as long_press_us and repeat_us, which turns off long-press detection.
 *
 * @return
 *  - ESP_OK: Success
 *  - ESP_ERR_INVALID_ARG: key debouncer was not properly initialized beforehand
 *  - Others: Fail
 */

 esp_err_t key_debouncer_register_key(gpio_num_t pin, int32_t marker, bool active_low, time_t debounce_us, time_t long_press_us, time_t repeat_us);


/**
 * @brief Unregister a key connected to a pin.
 *
 * @param[in] pin The GPIO number the key is connected to
 * 
 * @return
 *   - ESP_OK: Success
 */

esp_err_t key_debouncer_unregister_key(gpio_num_t pin);


/**
 * @brief Set the long-pressed delay.
 *  * 
 * @param[in] pin The key's pin number.
 * @param[in] microseconds Delay in microseconds, or 0 to turn off long-press detection.
 *
 * @return
 *  - ESP_OK: Success
 *  - ESP_ERR_INVALID_ARG: An argument has an illegal value
 *  - ESP_ERR_INVALID_STATE: The key is not configured
 */

esp_err_t key_debouncer_set_long_pressed(gpio_num_t pin, time_t microseconds);


/**
 * @brief Set the repetition interval.
 * 
 * Repetition only occurs after detection of long-presses. This means that you'll
 * first receive a long-press notification, and after that repetition notifications.
 * 
 * @param[in] pin The key's pin number.
 * @param[in] microseconds Repetition interval, or 0 to turn off repetition.
 *
 * @return
 *  - ESP_OK: Success
 *  - ESP_ERR_INVALID_ARG: An argument has an illegal value
 *  - ESP_ERR_INVALID_STATE: The key is not configured, or long-press is not set.
 */

esp_err_t key_debouncer_set_repeat(gpio_num_t pin, time_t microseconds);


/**
 * @brief Get the current key state in a polling fashion.
 * 
 * If you need this, something's weird about your code. What the fuck is wrong with you, little pervert?!
 * 
 * @param[in] pin The key's pin number.
 *
 * @return
 *  - 0: key is not pressed
 *  - 1: key is pressed
 *  - -1: key is not configured
 */

int8_t key_debouncer_get_state(gpio_num_t pin);