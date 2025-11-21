// !!! DO NOT ADD #pragma once !!! //

RGB_MATRIX_EFFECT(keypress_splash)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#include "rgb_matrix.h"

#define MAX_SPLASHES 5
#define SPLASH_DURATION 500 // ms

typedef struct {
    uint8_t  center;
    uint32_t start_time;
    bool     active;
    uint16_t max_radius;
} splash_t;

static splash_t splashes[MAX_SPLASHES];

static bool keypress_splash(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    for (uint8_t i = led_min; i < led_max; i++) {
        uint16_t total_brightness = 0;

        for (uint8_t s = 0; s < MAX_SPLASHES; s++) {
            if (!splashes[s].active) continue;

            uint32_t elapsed = timer_elapsed32(splashes[s].start_time);
            if (elapsed > SPLASH_DURATION) {
                splashes[s].active = false;
                continue;
            }

            float progress = (float)elapsed / SPLASH_DURATION;
            float radius   = progress * splashes[s].max_radius;

            int16_t dx = g_led_config.point[i].x - g_led_config.point[splashes[s].center].x;
            int16_t dy = g_led_config.point[i].y - g_led_config.point[splashes[s].center].y;
            float distance = sqrtf((float)(dx * dx + dy * dy));

            if (distance <= radius) {
                // Fade brightness over time (strong at start, weaker at end)
                float time_fade = 1.0f - progress;
                // Spatial fade: stronger near center, weaker near edge
                float spatial_fade = 1.0f - (distance / radius);
                if (spatial_fade < 0) spatial_fade = 0;

                uint8_t brightness = (uint8_t)(255 * time_fade * spatial_fade);
                total_brightness = MIN(255, total_brightness + brightness);
            }
        }

        rgb_matrix_set_color(i, 0x00, total_brightness, total_brightness); // cyan
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
