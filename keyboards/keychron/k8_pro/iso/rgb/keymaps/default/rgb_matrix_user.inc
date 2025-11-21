// !!! DO NOT ADD #pragma once !!! //

// Step 1: Declare your custom effect
RGB_MATRIX_EFFECT(keypress_splash)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#include "rgb_matrix.h"

#define MAX_SPLASHES 5
#define SPLASH_DURATION 500 // ms

typedef struct {
    uint8_t  center;
    uint32_t start_time;
    bool     active;
} splash_t;

static splash_t splashes[MAX_SPLASHES];

// The animation itself
static bool keypress_splash(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    for (uint8_t i = led_min; i < led_max; i++) {
        uint16_t total_brightness = 0;

        // Check all active splashes
        for (uint8_t s = 0; s < MAX_SPLASHES; s++) {
            if (!splashes[s].active) continue;

            uint32_t elapsed = timer_elapsed32(splashes[s].start_time);
            if (elapsed > SPLASH_DURATION) {
                splashes[s].active = false;
                continue;
            }

            // Progress 0 → 1
            float progress = (float)elapsed / SPLASH_DURATION;

            // Max radius: diagonal of keyboard area (~60 units typical)
            uint16_t max_radius = 60;
            float radius = progress * max_radius;

            // Fade brightness over time
            uint8_t base_brightness = (uint8_t)(255 * (1.0f - progress));

            // Distance from splash center
            int16_t dx = g_led_config.point[i].x - g_led_config.point[splashes[s].center].x;
            int16_t dy = g_led_config.point[i].y - g_led_config.point[splashes[s].center].y;
            float distance = sqrtf((float)(dx * dx + dy * dy));

            // Fill inside the radius
            if (distance <= radius) {
                // Stronger near center, weaker near edge
                uint8_t brightness = (uint8_t)(base_brightness * (1.0f - (distance / radius)));
                total_brightness = MIN(255, total_brightness + brightness);
            }
        }

        // Apply combined brightness (cyan splash)
        rgb_matrix_set_color(i, 0x00, total_brightness, total_brightness);
    }

    return rgb_matrix_check_finished_leds(led_max);
}

// Hook into keypresses to trigger splash
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        uint8_t led_index = g_led_config.matrix_co[record->event.key.row][record->event.key.col];
        if (led_index != NO_LED) {
            // Find a free slot
            for (uint8_t s = 0; s < MAX_SPLASHES; s++) {
                if (!splashes[s].active) {
                    splashes[s].center     = led_index;
                    splashes[s].start_time = timer_read32();
                    splashes[s].active     = true;
                    break;
                }
            }
        }
    }
    return true; // let QMK handle the key normally
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
