// !!! DO NOT ADD #pragma once !!! //

// Step 1: Declare your custom effects using RGB_MATRIX_EFFECT macro
RGB_MATRIX_EFFECT(my_cool_effect)
RGB_MATRIX_EFFECT(ripple_effect)
RGB_MATRIX_EFFECT(breathing_effect)

// Step 2: Define effects inside RGB_MATRIX_CUSTOM_EFFECT_IMPLS block
#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#include "rgb_matrix.h"

// Simple solid color effect
static bool my_cool_effect(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t i = led_min; i < led_max; i++) {
        // Set all LEDs to cyan color (R, G, B)
        rgb_matrix_set_color(i, 0x00, 0xFF, 0xFF);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

// Ripple effect - fixed version
static uint8_t ripple_pos = 0;
static uint32_t ripple_timer = 0;

static bool ripple_effect(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    // Update ripple position based on speed
    if (timer_elapsed32(ripple_timer) > (255 - rgb_matrix_config.speed)) {
        ripple_timer = timer_read32();
        ripple_pos++;
        // Use RGB_MATRIX_LED_COUNT instead of DRIVER_LED_TOTAL
        if (ripple_pos >= RGB_MATRIX_LED_COUNT) {
            ripple_pos = 0;
        }
    }
    
    for (uint8_t i = led_min; i < led_max; i++) {
        // Calculate distance from ripple center
        int16_t distance = abs((int16_t)i - (int16_t)ripple_pos);
        if (distance > RGB_MATRIX_LED_COUNT / 2) {
            distance = RGB_MATRIX_LED_COUNT - distance;
        }
        
        // Calculate brightness based on distance
        uint8_t brightness;
        if (distance < 10) {
            brightness = 255 - (distance * 25);
        } else {
            brightness = 0;
        }
        
        // Set color with ripple effect (blue in this case)
        rgb_matrix_set_color(i, 0x00, 0x00, brightness);
    }
    return rgb_matrix_check_finished_leds(led_max);
}
// Breathing effect - fixed version
static bool breathing_effect(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // Calculate breathing pattern based on timer
    uint16_t time = scale16by8(g_rgb_timer, rgb_matrix_config.speed / 8);

    // Use rgb_matrix_get_val() instead of rgb_matrix_config.val
    uint8_t val = scale8(abs8(sin8(time) - 128) * 2, rgb_matrix_get_val());

    for (uint8_t i = led_min; i < led_max; i++) {
        // Breathing purple effect
        rgb_matrix_set_color(i, scale8(val, 0xFF), 0x00, scale8(val, 0xFF));
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
