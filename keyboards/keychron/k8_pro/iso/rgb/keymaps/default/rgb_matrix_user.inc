// !!! DO NOT ADD #pragma once !!! //

// Step 1: Declare your custom effects using RGB_MATRIX_EFFECT macro
RGB_MATRIX_EFFECT(my_cool_effect)
RGB_MATRIX_EFFECT(ripple_effect)

// Step 2: Define effects inside RGB_MATRIX_CUSTOM_EFFECT_IMPLS block
#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// Simple solid color effect
static bool my_cool_effect(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t i = led_min; i < led_max; i++) {
        // Set all LEDs to cyan color (R, G, B)
        rgb_matrix_set_color(i, 0x00, 0xFF, 0xFF);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

// More complex ripple effect
static uint8_t ripple_pos = 0;
static uint32_t ripple_timer = 0;

static bool ripple_effect(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    // Update ripple position every 50ms
    if (timer_elapsed32(ripple_timer) > 50) {
        ripple_timer = timer_read32();
        ripple_pos++;
        if (ripple_pos >= DRIVER_LED_TOTAL) {
            ripple_pos = 0;
        }
    }
    
    for (uint8_t i = led_min; i < led_max; i++) {
        // Calculate distance from ripple center
        uint8_t distance = abs(i - ripple_pos);
        if (distance > DRIVER_LED_TOTAL / 2) {
            distance = DRIVER_LED_TOTAL - distance;
        }
        
        // Calculate brightness based on distance
        uint8_t brightness = 255 - (distance * 8);
        if (brightness > 255) brightness = 0;
        
        // Set color with ripple effect (purple in this case)
        rgb_matrix_set_color(i, brightness, 0x00, brightness);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
