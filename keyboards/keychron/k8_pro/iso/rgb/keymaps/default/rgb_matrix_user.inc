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
        if (splash_radius > RGB_MATRIX_LED_COUNT / 2) {
            splash_active = false; // stop splash when it fades out
        }
    }

    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t brightness = 0;
        if (splash_active) {
            int16_t distance = abs((int16_t)i - (int16_t)splash_center);
            if (abs(distance - splash_radius) < 3) {
                brightness = 255 - (abs(distance - splash_radius) * 80);
            }
        }
        // Cyan splash color
        rgb_matrix_set_color(i, 0x00, brightness, brightness);
    }

    return rgb_matrix_check_finished_leds(led_max);
}

// Hook into keypresses
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        // Start a new splash centered on the pressed key’s LED index
        splash_center = record->event.key.row * MATRIX_COLS + record->event.key.col;
        splash_radius = 0;
        splash_active = true;
        splash_timer  = timer_read32();
    }
    return true; // let QMK handle the key normally
}

// Indicators hook must return bool, not void
bool rgb_matrix_indicators_user(void) {
    // We don’t override indicators here, so return false
    return false;
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
