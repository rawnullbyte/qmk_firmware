// !!! DO NOT ADD #pragma once !!! //

// Step 1: Declare your custom effect
RGB_MATRIX_EFFECT(wave_splash)

// Step 2: Define effect inside RGB_MATRIX_CUSTOM_EFFECT_IMPLS block
#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#include "rgb_matrix.h"

// Wave splash effect
static uint32_t splash_timer = 0;
static uint8_t splash_radius = 0;

static bool wave_splash(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // Expand the splash radius over time
    if (timer_elapsed32(splash_timer) > (255 - rgb_matrix_config.speed)) {
        splash_timer = timer_read32();
        splash_radius++;
        if (splash_radius > RGB_MATRIX_LED_COUNT / 2) {
            splash_radius = 0; // restart splash
        }
    }

    // Center of splash (middle of keyboard)
    uint8_t center = RGB_MATRIX_LED_COUNT / 2;

    for (uint8_t i = led_min; i < led_max; i++) {
        // Distance from center
        int16_t distance = abs((int16_t)i - (int16_t)center);

        // Brightness based on how close distance is to current radius
        uint8_t brightness;
        if (abs(distance - splash_radius) < 3) {
            brightness = 255 - (abs(distance - splash_radius) * 80);
        } else {
            brightness = 0;
        }

        // Color: cyan splash
        rgb_matrix_set_color(i, 0x00, brightness, brightness);
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
