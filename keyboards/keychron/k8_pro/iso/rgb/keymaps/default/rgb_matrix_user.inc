// !!! DO NOT ADD #pragma once !!! //

RGB_MATRIX_EFFECT(keypress_splash)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#include "rgb_matrix.h"

#define MAX_SPLASHES 15
#define SPLASH_DURATION 500 // ms
#define LED_FADE_TIME 300   // ms each LED fades after being lit

typedef struct {
    uint8_t  center;
    uint32_t start_time;
    bool     active;
    uint16_t max_radius;
} splash_t;

static splash_t splashes[MAX_SPLASHES];

static bool keypress_splash(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // Get user-selected color from VIA/VIAL
    HSV hsv = rgb_matrix_config.hsv;
    RGB rgb = hsv_to_rgb(hsv);

    // Duration tied to speed slider (range ~200–1000 ms)
    uint16_t splash_duration = 200 + (rgb_matrix_config.speed * 4);

    for (uint8_t i = led_min; i < led_max; i++) {
        uint16_t total_brightness = 0;

        for (uint8_t s = 0; s < MAX_SPLASHES; s++) {
            if (!splashes[s].active) continue;

            uint32_t elapsed = timer_elapsed32(splashes[s].start_time);
            if (elapsed > splash_duration + LED_FADE_TIME) {
                splashes[s].active = false;
                continue;
            }

            // Current wavefront radius
            float progress = (float)elapsed / splash_duration;
            float radius   = progress * splashes[s].max_radius;

            // Distance from splash center
            int16_t dx = g_led_config.point[i].x - g_led_config.point[splashes[s].center].x;
            int16_t dy = g_led_config.point[i].y - g_led_config.point[splashes[s].center].y;
            float distance = sqrtf((float)(dx * dx + dy * dy));

            // If wavefront has passed this LED
            if (distance <= radius) {
                // Time since wavefront reached this LED
                float led_elapsed = elapsed - (distance / splashes[s].max_radius) * splash_duration;
                if (led_elapsed >= 0 && led_elapsed <= LED_FADE_TIME) {
                    // Fade brightness from full → 0 over LED_FADE_TIME
                    float fade_progress = 1.0f - (led_elapsed / LED_FADE_TIME);
                    uint8_t brightness = (uint8_t)(255 * fade_progress);
                    total_brightness = MIN(255, total_brightness + brightness);
                }
            }
        }

        // Apply combined brightness with user-selected color
        rgb_matrix_set_color(i,
            (rgb.r * total_brightness) / 255,
            (rgb.g * total_brightness) / 255,
            (rgb.b * total_brightness) / 255);
    }

    return rgb_matrix_check_finished_leds(led_max);
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        uint8_t led_index = g_led_config.matrix_co[record->event.key.row][record->event.key.col];
        if (led_index != NO_LED) {
            // Find farthest LED distance for dynamic max_radius
            uint16_t max_dist = 0;
            for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
                int16_t dx = g_led_config.point[i].x - g_led_config.point[led_index].x;
                int16_t dy = g_led_config.point[i].y - g_led_config.point[led_index].y;
                uint16_t dist = sqrt16(dx * dx + dy * dy);
                if (dist > max_dist) max_dist = dist;
            }
            max_dist += 10; // extend radius

            // Find a free slot
            for (uint8_t s = 0; s < MAX_SPLASHES; s++) {
                if (!splashes[s].active) {
                    splashes[s].center     = led_index;
                    splashes[s].start_time = timer_read32();
                    splashes[s].active     = true;
                    splashes[s].max_radius = max_dist;
                    break;
                }
            }
        }
    }
    return true;
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
