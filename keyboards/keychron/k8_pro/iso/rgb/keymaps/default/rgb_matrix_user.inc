// !!! DO NOT ADD #pragma once !!! //

// Declare custom effects
RGB_MATRIX_EFFECT(key_splash)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#include "rgb_matrix.h"
#include "lib/lib8tion/lib8tion.h"

// Key splash effect - creates a ripple around pressed keys
static uint8_t splash_positions[RGB_MATRIX_LED_COUNT];
static uint32_t splash_timers[RGB_MATRIX_LED_COUNT];

static bool key_splash(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    // Process any new key presses
    if (params->init) {
        memset(splash_positions, 0, sizeof(splash_positions));
        memset(splash_timers, 0, sizeof(splash_timers));
    }
    
    // Check for key presses and activate splashes
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t i = led_min; i < led_max; i++) {
        // Check if this LED corresponds to a pressed key
        if (g_led_config.flags[i] & LED_FLAG_KEYLIGHT) {
            if (rgb_matrix_get_flags() & LED_FLAG_KEYLIGHT) {
                // Activate splash for this key
                splash_positions[i] = 1;
                splash_timers[i] = timer_read32();
            }
        }
    }
    
    // Update and render all active splashes
    for (uint8_t i = led_min; i < led_max; i++) {
        if (splash_positions[i] > 0) {
            // Calculate splash progress (0-255)
            uint32_t elapsed = timer_elapsed32(splash_timers[i]);
            uint8_t progress = scale16by8(elapsed, rgb_matrix_config.speed / 8);
            
            if (progress < 255) {
                // Expand splash radius based on progress
                uint8_t radius = scale8(progress, 10); // Max radius of 10 LEDs
                
                // Calculate splash intensity (fades out)
                uint8_t intensity = 255 - progress;
                
                // Color the center key
                rgb_matrix_set_color(i, intensity, intensity >> 2, 0x00); // Orange center
                
                // Color surrounding LEDs based on distance
                for (uint8_t j = led_min; j < led_max; j++) {
                    if (j == i) continue; // Skip center (already colored)
                    
                    // Calculate distance between LEDs (simplified)
                    int16_t dx = (int16_t)g_led_config.point[i].x - (int16_t)g_led_config.point[j].x;
                    int16_t dy = (int16_t)g_led_config.point[i].y - (int16_t)g_led_config.point[j].y;
                    uint8_t distance = sqrt16(dx * dx + dy * dy) / 8; // Scale distance
                    
                    if (distance <= radius && distance > 0) {
                        // Calculate intensity based on distance from center
                        uint8_t splash_intensity = intensity - (distance * 25);
                        if (splash_intensity > 0) {
                            rgb_matrix_set_color(j, splash_intensity >> 2, splash_intensity >> 1, splash_intensity);
                        }
                    }
                }
                
                splash_positions[i] = radius;
            } else {
                // Splash completed
                splash_positions[i] = 0;
            }
        }
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
