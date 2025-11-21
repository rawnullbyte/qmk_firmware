// !!! DO NOT ADD #pragma once !!! //

// Step 1: Declare your custom effect
RGB_MATRIX_EFFECT(keypress_splash)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#include "rgb_matrix.h"

static uint8_t  splash_radius  = 0;
static uint8_t  splash_center  = 0;
static bool     splash_active  = false;
static uint32_t splash_timer   = 0;

// The animation itself
static bool keypress_splash(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // Expand radius over time if splash is active
    if (splash_active && timer_elapsed32(splash_timer) > (255 - rgb_matrix_config.speed)) {
        splash_timer = timer_read32();
        splash_radius++;
        if (splash_radius > 40) { // max radius in LED coordinate space
            splash_active = false; // stop splash when it fades out
        }
    }

    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t brightness = 0;
        if (splash_active) {
            // Get LED coordinates
            int16_t dx = g_led_config.point[i].x - g_led_config.point[splash_center].x;
            int16_t dy = g_led_config.point[i].y - g_led_config.point[splash_center].y;
            uint16_t distance = sqrt16(dx * dx + dy * dy);

            // Light LEDs near the current radius
            if (abs(distance - splash_radius) < 3) {
                brightness = 255 - (abs(distance - splash_radius) * 80);
            }
        }
        // Cyan splash color
        rgb_matrix_set_color(i, 0x00, brightness, brightness);
    }

    return rgb_matrix_check_finished_leds(led_max);
}

// Hook into keypresses to trigger splash
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        // Find LED index for this key
        uint8_t led_index = led_index_at(record->event.key.row, record->event.key.col);
        if (led_index != NO_LED) {
            splash_center = led_index;
            splash_radius = 0;
            splash_active = true;
            splash_timer  = timer_read32();
        }
    }
    return true; // let QMK handle the key normally
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
