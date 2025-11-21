// !!! DO NOT ADD #pragma once !!! //

// Step 1: Declare your custom effect
RGB_MATRIX_EFFECT(keypress_splash)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#include "rgb_matrix.h"

typedef struct {
    uint8_t  center;
    uint32_t start_time;
    bool     active;
} splash_t;

static splash_t splash = {0, 0, false};

// Duration of splash in ms
#define SPLASH_DURATION 500

static bool keypress_splash(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    if (splash.active) {
        uint32_t elapsed = timer_elapsed32(splash.start_time);
        if (elapsed > SPLASH_DURATION) {
            splash.active = false;
        } else {
            // Radius grows with time
            float progress = (float)elapsed / SPLASH_DURATION;
            // Max radius: diagonal of keyboard area (~60 units typical)
            uint16_t max_radius = 60;
            float radius = progress * max_radius;

            for (uint8_t i = led_min; i < led_max; i++) {
                int16_t dx = g_led_config.point[i].x - g_led_config.point[splash.center].x;
                int16_t dy = g_led_config.point[i].y - g_led_config.point[splash.center].y;
                float distance = sqrtf((float)(dx * dx + dy * dy));

                // Brightness peak at wavefront, fade elsewhere
                float diff = fabsf(distance - radius);
                uint8_t brightness = 0;
                if (diff < 5) {
                    brightness = (uint8_t)(255 * (1.0f - (diff / 5.0f)));
                }

                rgb_matrix_set_color(i, 0x00, brightness, brightness); // cyan
            }
        }
    }

    return rgb_matrix_check_finished_leds(led_max);
}

// Hook into keypresses to trigger splash
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        uint8_t led_index = g_led_config.matrix_co[record->event.key.row][record->event.key.col];
        if (led_index != NO_LED) {
            splash.center     = led_index;
            splash.start_time = timer_read32();
            splash.active     = true;
        }
    }
    return true; // let QMK handle the key normally
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
