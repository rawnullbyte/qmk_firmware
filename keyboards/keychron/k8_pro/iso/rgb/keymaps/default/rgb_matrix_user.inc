// !!! DO NOT ADD #pragma once !!! //

RGB_MATRIX_EFFECT(keypress_splash)
RGB_MATRIX_EFFECT(rain_drops_effect)
RGB_MATRIX_EFFECT(golden_breath)
RGB_MATRIX_EFFECT(cosmic_swirl)
RGB_MATRIX_EFFECT(pulse_wave)
RGB_MATRIX_EFFECT(thunder_storm)
RGB_MATRIX_EFFECT(plasma_cloud)
RGB_MATRIX_EFFECT(keypress_flame)
RGB_MATRIX_EFFECT(energy_pulse)
RGB_MATRIX_EFFECT(ripple_pool)
RGB_MATRIX_EFFECT(keypress_ripple)
RGB_MATRIX_EFFECT(keypress_sparkle)
RGB_MATRIX_EFFECT(keypress_trail)
RGB_MATRIX_EFFECT(keypress_sonar)
RGB_MATRIX_EFFECT(keypress_thunder)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#include "rgb_matrix.h"
#include "math.h"

// Existing keypress_splash effect
#define MAX_SPLASHES 15
#define SPLASH_DURATION 500
#define LED_FADE_TIME 300

typedef struct {
    uint8_t  center;
    uint32_t start_time;
    bool     active;
    uint16_t max_radius;
} splash_t;

static splash_t splashes[MAX_SPLASHES];

// Rain drops effect - FIXED
#define MAX_DROPS 5
typedef struct {
    uint8_t x, y;
    float progress;
    float speed;
    bool active;
    uint8_t size;
} rain_drop_t;

static rain_drop_t rain_drops_array[MAX_DROPS];
static uint32_t last_rain_spawn = 0;

// Golden breath effect (renamed from flame_breath)
static float golden_time = 0;

// Cosmic swirl effect
static float cosmic_angle = 0;

// Pulse wave effect  
static float pulse_wave_time = 0;

// Thunder storm effect (replaces starfield)
#define MAX_THUNDERS 3
typedef struct {
    uint8_t start_x, end_x;
    uint8_t start_y, end_y;
    uint32_t start_time;
    bool active;
    uint8_t intensity;
} thunder_t;

static thunder_t thunders[MAX_THUNDERS];
static uint32_t last_thunder_spawn = 0;

// Plasma cloud effect
static float plasma_time = 0;

// Keypress flame effect - FIXED
#define MAX_FLAMES 10
typedef struct {
    uint8_t center;
    uint32_t start_time;
    uint32_t last_update;
    bool active;
    uint8_t intensity;
    uint8_t flicker_offset;
} flame_t;

static flame_t flames[MAX_FLAMES];

// Energy pulse effect (replaces breathe_cycle)
static float energy_time = 0;

// Ripple pool effect  
#define MAX_RIPPLES 5
typedef struct {
    uint8_t center;
    uint32_t start_time;
    bool active;
    float max_radius;
} ripple_t;

static ripple_t ripples[MAX_RIPPLES];
static uint32_t last_auto_ripple = 0;

// New keypress effects
#define MAX_RIPPLES_KEY 8
typedef struct {
    uint8_t center;
    uint32_t start_time;
    bool active;
    HSV color;
} key_ripple_t;

static key_ripple_t key_ripples[MAX_RIPPLES_KEY];

#define MAX_SPARKLES 12
typedef struct {
    uint8_t center;
    uint32_t start_time;
    bool active;
    uint8_t lifetime;
} sparkle_t;

static sparkle_t sparkles[MAX_SPARKLES];

#define MAX_TRAILS 6
typedef struct {
    uint8_t center;
    uint32_t start_time;
    bool active;
    uint8_t direction;
} trail_t;

static trail_t trails[MAX_TRAILS];

#define MAX_SONARS 8
typedef struct {
    uint8_t center;
    uint32_t start_time;
    bool active;
    uint8_t hue;
} sonar_t;

static sonar_t sonars[MAX_SONARS];

#define MAX_THUNDERS_KEY 4
typedef struct {
    uint8_t start_x, start_y;
    uint8_t end_x, end_y;
    uint32_t start_time;
    bool active;
} key_thunder_t;

static key_thunder_t key_thunders[MAX_THUNDERS_KEY];

// ===== FIXED RAIN DROPS EFFECT =====
static bool rain_drops_effect(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    HSV hsv = rgb_matrix_config.hsv;
    uint32_t time = timer_elapsed32(0);
    
    // Spawn new drops occasionally
    if (time - last_rain_spawn > 500 + (255 - rgb_matrix_config.speed) * 3) {
        last_rain_spawn = time;
        for (uint8_t i = 0; i < MAX_DROPS; i++) {
            if (!rain_drops_array[i].active) {
                rain_drops_array[i].x = 20 + (rand() % 184);
                rain_drops_array[i].y = 0;
                rain_drops_array[i].progress = 0.0f;
                rain_drops_array[i].speed = 0.3f + (rand() % 70) * 0.01f;
                rain_drops_array[i].active = true;
                rain_drops_array[i].size = 3 + (rand() % 4);
                break;
            }
        }
    }
    
    // Update drops
    for (uint8_t d = 0; d < MAX_DROPS; d++) {
        if (rain_drops_array[d].active) {
            rain_drops_array[d].progress += rain_drops_array[d].speed * 0.02f;
            if (rain_drops_array[d].progress > 1.0f) {
                rain_drops_array[d].active = false;
            }
        }
    }
    
    // Render drops as circles with ripples
    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t brightness = 0;
        
        for (uint8_t d = 0; d < MAX_DROPS; d++) {
            if (!rain_drops_array[d].active) continue;
            
            uint8_t drop_y = (uint8_t)(rain_drops_array[d].progress * 128);
            int16_t dx = g_led_config.point[i].x - rain_drops_array[d].x;
            int16_t dy = g_led_config.point[i].y - drop_y;
            float distance = sqrtf(dx * dx + dy * dy);
            
            // Main drop circle
            if (distance < rain_drops_array[d].size) {
                uint8_t drop_bright = (uint8_t)(255 * (1.0f - distance / rain_drops_array[d].size));
                brightness = MAX(brightness, drop_bright);
            }
            
            // Ripple effect when drop hits "ground" (near bottom)
            if (drop_y > 100 && drop_y < 110) {
                float ripple_progress = (drop_y - 100) / 10.0f;
                float ripple_radius = ripple_progress * 15.0f;
                float ripple_distance = fabs(distance - ripple_radius);
                
                if (ripple_distance < 2.0f) {
                    uint8_t ripple_bright = (uint8_t)(255 * (1.0f - ripple_progress) * (1.0f - ripple_distance / 2.0f));
                    brightness = MAX(brightness, ripple_bright);
                }
            }
        }
        
        if (brightness > 0) {
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, 
                (rgb.r * brightness) / 255,
                (rgb.g * brightness) / 255, 
                (rgb.b * brightness) / 255);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== GOLDEN BREATH EFFECT =====
static bool golden_breath(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    golden_time += 0.02;
    
    // Golden color palette
    uint8_t base_hue = 40; // Golden yellow
    
    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t x = g_led_config.point[i].x;
        uint8_t y = g_led_config.point[i].y;
        
        // Breathing effect
        float breath = (sin(golden_time) + 1.0) * 0.5;
        
        // Subtle shimmer effect
        float shimmer = sin(x * 0.05 + golden_time * 1.5) * 0.2 + 0.8;
        float pulse = cos(y * 0.03 + golden_time * 2.0) * 0.1 + 0.9;
        
        uint8_t brightness = (uint8_t)(breath * shimmer * pulse * rgb_matrix_config.hsv.v);
        uint8_t hue = (base_hue + (y / 8)) % 255;
        
        HSV hsv = {hue, 200 + (uint8_t)(sin(golden_time + x * 0.02) * 55), brightness};
        RGB rgb = hsv_to_rgb(hsv);
        
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== THUNDER STORM EFFECT =====
static bool thunder_storm(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    HSV hsv = rgb_matrix_config.hsv;
    uint32_t time = timer_elapsed32(0);
    
    // Spawn new thunders occasionally
    if (time - last_thunder_spawn > 2000 + (rand() % 5000)) {
        last_thunder_spawn = time;
        for (uint8_t i = 0; i < MAX_THUNDERS; i++) {
            if (!thunders[i].active) {
                thunders[i].start_x = rand() % 224;
                thunders[i].start_y = 0;
                thunders[i].end_x = thunders[i].start_x + (rand() % 40) - 20;
                thunders[i].end_y = 128;
                thunders[i].start_time = time;
                thunders[i].active = true;
                thunders[i].intensity = 200 + rand() % 55;
                break;
            }
        }
    }
    
    // Clear all LEDs first
    for (uint8_t i = led_min; i < led_max; i++) {
        rgb_matrix_set_color(i, 0, 0, 0);
    }
    
    // Render thunders
    for (uint8_t t = 0; t < MAX_THUNDERS; t++) {
        if (!thunders[t].active) continue;
        
        uint32_t elapsed = time - thunders[t].start_time;
        if (elapsed > 300) { // Thunder lasts 300ms
            thunders[t].active = false;
            continue;
        }
        
        // Thunder flickers
        uint8_t brightness = (elapsed < 100 || (elapsed > 150 && elapsed < 180) || (elapsed > 220 && elapsed < 240)) ? thunders[t].intensity : 0;
        
        if (brightness > 0) {
            // Draw thunder bolt as a line with branches
            int16_t dx = thunders[t].end_x - thunders[t].start_x;
            int16_t dy = thunders[t].end_y - thunders[t].start_y;
            float steps = fmax(fabs(dx), fabs(dy));
            
            for (uint8_t step = 0; step < steps; step++) {
                float t_val = step / steps;
                uint8_t x = thunders[t].start_x + (uint8_t)(dx * t_val);
                uint8_t y = thunders[t].start_y + (uint8_t)(dy * t_val);
                
                // Add some randomness to make it look like lightning
                x += (rand() % 5) - 2;
                y += (rand() % 3) - 1;
                
                // Find closest LED to this point
                for (uint8_t i = led_min; i < led_max; i++) {
                    int16_t led_dx = abs(g_led_config.point[i].x - x);
                    int16_t led_dy = abs(g_led_config.point[i].y - y);
                    
                    if (led_dx < 3 && led_dy < 3) {
                        RGB rgb = hsv_to_rgb(hsv);
                        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
                    }
                }
            }
        }
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== FIXED KEYPRESS FLAME EFFECT =====
static bool keypress_flame(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    uint32_t current_time = timer_elapsed32(0);
    HSV hsv = rgb_matrix_config.hsv;
    
    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t total_brightness = 0;
        
        for (uint8_t f = 0; f < MAX_FLAMES; f++) {
            if (!flames[f].active) continue;
            
            uint32_t elapsed = current_time - flames[f].start_time;
            
            // Update flicker for active flames
            if (current_time - flames[f].last_update > 50) {
                flames[f].flicker_offset = rand() % 30;
                flames[f].last_update = current_time;
            }
            
            // Base flame properties
            float base_radius = 12.0f;
            float flicker = 1.0f + sin(flames[f].flicker_offset * 0.1f) * 0.3f;
            float radius = base_radius * flicker;
            
            int16_t dx = g_led_config.point[i].x - g_led_config.point[flames[f].center].x;
            int16_t dy = g_led_config.point[i].y - g_led_config.point[flames[f].center].y;
            float distance = sqrtf(dx * dx + dy * dy);
            
            if (distance <= radius) {
                // Flame intensity decreases with distance and has flickering
                float intensity = 1.0f - (distance / radius);
                float flame_flicker = 0.7f + (sin(flames[f].flicker_offset + distance * 0.5f) * 0.3f);
                uint8_t brightness = (uint8_t)(255 * intensity * flame_flicker);
                total_brightness = MIN(255, total_brightness + brightness);
            }
        }
        
        if (total_brightness > 0) {
            // Orange-red flame colors that follow user hue
            uint8_t flame_hue = (hsv.h + 10) % 255; // Slightly shifted for fire effect
            HSV flame_hsv = {flame_hue, hsv.s, total_brightness};
            RGB rgb = hsv_to_rgb(flame_hsv);
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== ENERGY PULSE EFFECT =====
static bool energy_pulse(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    energy_time += 0.03;
    HSV hsv = rgb_matrix_config.hsv;
    
    for (uint8_t i = led_min; i < led_max; i++) {
        float x = (g_led_config.point[i].x - 112) / 112.0f;
        float y = (g_led_config.point[i].y - 64) / 64.0f;
        float distance_from_center = sqrtf(x * x + y * y);
        
        // Pulsing energy waves
        float pulse1 = sin(distance_from_center * 8.0f - energy_time * 3.0f) * 0.5f + 0.5f;
        float pulse2 = cos(distance_from_center * 6.0f + energy_time * 2.0f) * 0.3f + 0.7f;
        float pulse3 = sin(energy_time * 4.0f) * 0.2f + 0.8f;
        
        uint8_t brightness = (uint8_t)(pulse1 * pulse2 * pulse3 * hsv.v);
        uint8_t hue = (hsv.h + (uint8_t)(distance_from_center * 30) + (uint8_t)(energy_time * 20)) % 255;
        
        HSV energy_hsv = {hue, hsv.s, brightness};
        RGB rgb = hsv_to_rgb(energy_hsv);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== ENHANCED RIPPLE POOL EFFECT =====
static bool ripple_pool(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    uint16_t ripple_duration = 1800 + (rgb_matrix_config.speed * 6);
    uint32_t time = timer_elapsed32(0);
    HSV hsv = rgb_matrix_config.hsv;
    
    // Auto-spawn ripples occasionally
    if (time - last_auto_ripple > 2500 - (rgb_matrix_config.speed * 8)) {
        last_auto_ripple = time;
        for (uint8_t r = 0; r < MAX_RIPPLES; r++) {
            if (!ripples[r].active) {
                ripples[r].center = rand() % RGB_MATRIX_LED_COUNT;
                ripples[r].start_time = time;
                ripples[r].active = true;
                ripples[r].max_radius = 35 + rand() % 25;
                break;
            }
        }
    }
    
    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t brightness = 0;
        
        for (uint8_t r = 0; r < MAX_RIPPLES; r++) {
            if (!ripples[r].active) continue;
            
            uint32_t elapsed = time - ripples[r].start_time;
            if (elapsed > ripple_duration) {
                ripples[r].active = false;
                continue;
            }
            
            float progress = (float)elapsed / ripple_duration;
            float radius = progress * ripples[r].max_radius;
            
            int16_t dx = g_led_config.point[i].x - g_led_config.point[ripples[r].center].x;
            int16_t dy = g_led_config.point[i].y - g_led_config.point[ripples[r].center].y;
            float distance = sqrtf(dx * dx + dy * dy);
            
            // Multiple ripple waves
            for (uint8_t wave = 0; wave < 3; wave++) {
                float wave_offset = wave * 0.2f;
                float wave_radius = radius * (1.0f - wave_offset);
                float wave_width = 3.0f;
                
                if (fabs(distance - wave_radius) < wave_width && progress > wave_offset) {
                    float wave_progress = (progress - wave_offset) / (1.0f - wave_offset);
                    float ripple_intensity = sin(wave_progress * M_PI) * (1.0f - fabs(distance - wave_radius) / wave_width);
                    brightness = MAX(brightness, (uint8_t)(ripple_intensity * 255));
                }
            }
        }
        
        if (brightness > 0) {
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, 
                (rgb.r * brightness) / 255,
                (rgb.g * brightness) / 255,
                (rgb.b * brightness) / 255);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== KEYPRESS THUNDER EFFECT =====
static bool keypress_thunder(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    uint32_t time = timer_elapsed32(0);
    HSV hsv = rgb_matrix_config.hsv;
    
    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t brightness = 0;
        
        for (uint8_t t = 0; t < MAX_THUNDERS_KEY; t++) {
            if (!key_thunders[t].active) continue;
            
            uint32_t elapsed = time - key_thunders[t].start_time;
            if (elapsed > 400) {
                key_thunders[t].active = false;
                continue;
            }
            
            // Thunder flickers
            if ((elapsed < 80) || (elapsed > 120 && elapsed < 150) || (elapsed > 200 && elapsed < 220)) {
                int16_t dx = g_led_config.point[i].x - key_thunders[t].start_x;
                int16_t dy = g_led_config.point[i].y - key_thunders[t].start_y;
                int16_t end_dx = g_led_config.point[i].x - key_thunders[t].end_x;
                int16_t end_dy = g_led_config.point[i].y - key_thunders[t].end_y;
                
                // Check if LED is near the thunder line
                float distance_to_line = fabs((key_thunders[t].end_x - key_thunders[t].start_x) * (key_thunders[t].start_y - g_led_config.point[i].y) - 
                                            (key_thunders[t].start_x - g_led_config.point[i].x) * (key_thunders[t].end_y - key_thunders[t].start_y)) / 
                                        sqrtf(pow(key_thunders[t].end_x - key_thunders[t].start_x, 2) + pow(key_thunders[t].end_y - key_thunders[t].start_y, 2));
                
                if (distance_to_line < 2.0f) {
                    brightness = 255;
                }
            }
        }
        
        if (brightness > 0) {
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// [Keep all the other existing effects exactly as they were: cosmic_swirl, pulse_wave, plasma_cloud, 
// keypress_ripple, keypress_sparkle, keypress_trail, keypress_sonar, and the original keypress_splash]

// ===== COSMIC SWIRL EFFECT =====
static bool cosmic_swirl(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    cosmic_angle += 0.03;
    HSV hsv = rgb_matrix_config.hsv;
    
    for (uint8_t i = led_min; i < led_max; i++) {
        float x = (g_led_config.point[i].x - 112) / 112.0;
        float y = (g_led_config.point[i].y - 64) / 64.0;
        
        float angle = atan2(y, x) + cosmic_angle;
        float radius = sqrt(x * x + y * y);
        
        float swirl = sin(radius * 20 - cosmic_angle * 3) * 0.5 + 0.5;
        uint8_t hue = (hsv.h + (uint8_t)((angle / (2 * M_PI) + radius + cosmic_angle) * 64)) % 255;
        uint8_t bright = (uint8_t)(swirl * hsv.v);
        
        HSV swirl_hsv = {hue, hsv.s, bright};
        RGB rgb = hsv_to_rgb(swirl_hsv);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== PULSE WAVE EFFECT =====
static bool pulse_wave(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    pulse_wave_time += 0.05;
    HSV hsv = rgb_matrix_config.hsv;
    float wave = (sin(pulse_wave_time) + 1.0) * 0.5;
    
    for (uint8_t i = led_min; i < led_max; i++) {
        float x = g_led_config.point[i].x / 224.0;
        float distance = fabs(x - wave);
        
        uint8_t brightness = (uint8_t)((1.0 - distance * 2) * 255);
        if (brightness > 10) {
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, 
                (rgb.r * brightness) / 255,
                (rgb.g * brightness) / 255,
                (rgb.b * brightness) / 255);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== PLASMA CLOUD EFFECT =====
static bool plasma_cloud(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    plasma_time += 0.02;
    HSV hsv = rgb_matrix_config.hsv;
    
    for (uint8_t i = led_min; i < led_max; i++) {
        float x = g_led_config.point[i].x / 224.0 * 4.0;
        float y = g_led_config.point[i].y / 128.0 * 4.0;
        
        float v1 = sin(x + plasma_time);
        float v2 = sin((x + y) * 0.5 + plasma_time);
        float v3 = sin(sqrt(x*x + y*y) + plasma_time);
        
        float plasma = (v1 + v2 + v3) / 3.0;
        uint8_t hue = (hsv.h + (uint8_t)((plasma + 1.0) * 63)) % 255;
        uint8_t bright = (uint8_t)((plasma * 0.5 + 0.5) * hsv.v);
        
        HSV plasma_hsv = {hue, hsv.s, bright};
        RGB rgb = hsv_to_rgb(plasma_hsv);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== KEYPRESS RIPPLE EFFECT =====
static bool keypress_ripple(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    uint16_t ripple_duration = 1200 + (rgb_matrix_config.speed * 4);
    HSV hsv = rgb_matrix_config.hsv;
    
    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t brightness = 0;
        
        for (uint8_t r = 0; r < MAX_RIPPLES_KEY; r++) {
            if (!key_ripples[r].active) continue;
            
            uint32_t elapsed = timer_elapsed32(key_ripples[r].start_time);
            if (elapsed > ripple_duration) {
                key_ripples[r].active = false;
                continue;
            }
            
            float progress = (float)elapsed / ripple_duration;
            float radius = progress * 40;
            
            int16_t dx = g_led_config.point[i].x - g_led_config.point[key_ripples[r].center].x;
            int16_t dy = g_led_config.point[i].y - g_led_config.point[key_ripples[r].center].y;
            float distance = sqrtf(dx * dx + dy * dy);
            
            if (fabs(distance - radius) < 4) {
                float ripple_intensity = sin(progress * M_PI) * (1.0 - fabs(distance - radius) / 4);
                uint8_t ripple_bright = (uint8_t)(ripple_intensity * 255);
                brightness = MAX(brightness, ripple_bright);
            }
        }
        
        if (brightness > 0) {
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, 
                (rgb.r * brightness) / 255,
                (rgb.g * brightness) / 255,
                (rgb.b * brightness) / 255);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== KEYPRESS SPARKLE EFFECT =====
static bool keypress_sparkle(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    uint16_t sparkle_duration = 600 + (rgb_matrix_config.speed * 3);
    HSV hsv = rgb_matrix_config.hsv;
    
    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t brightness = 0;
        
        for (uint8_t s = 0; s < MAX_SPARKLES; s++) {
            if (!sparkles[s].active) continue;
            
            uint32_t elapsed = timer_elapsed32(sparkles[s].start_time);
            if (elapsed > sparkle_duration) {
                sparkles[s].active = false;
                continue;
            }
            
            float progress = (float)elapsed / sparkle_duration;
            
            int16_t dx = abs(g_led_config.point[i].x - g_led_config.point[sparkles[s].center].x);
            int16_t dy = abs(g_led_config.point[i].y - g_led_config.point[sparkles[s].center].y);
            
            if (dx <= 8 && dy <= 8) {
                float distance = sqrtf(dx * dx + dy * dy);
                if (distance < 6) {
                    float sparkle = sin(progress * M_PI * 8) * (1.0 - progress) * (1.0 - distance / 6);
                    brightness = MAX(brightness, (uint8_t)(sparkle * 255));
                }
            }
        }
        
        if (brightness > 0) {
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, 
                (rgb.r * brightness) / 255,
                (rgb.g * brightness) / 255,
                (rgb.b * brightness) / 255);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== KEYPRESS TRAIL EFFECT =====
static bool keypress_trail(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    uint16_t trail_duration = 800 + (rgb_matrix_config.speed * 3);
    HSV hsv = rgb_matrix_config.hsv;
    
    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t brightness = 0;
        
        for (uint8_t t = 0; t < MAX_TRAILS; t++) {
            if (!trails[t].active) continue;
            
            uint32_t elapsed = timer_elapsed32(trails[t].start_time);
            if (elapsed > trail_duration) {
                trails[t].active = false;
                continue;
            }
            
            float progress = (float)elapsed / trail_duration;
            
            int16_t dx = g_led_config.point[i].x - g_led_config.point[trails[t].center].x;
            int16_t dy = g_led_config.point[i].y - g_led_config.point[trails[t].center].y;
            
            float distance = 0;
            switch (trails[t].direction) {
                case 0: distance = abs(dx); break;
                case 1: distance = abs(dy); break;
                case 2: distance = (abs(dx) + abs(dy)) * 0.7; break;
                default: distance = sqrtf(dx * dx + dy * dy); break;
            }
            
            if (distance < 20) {
                float trail_intensity = (1.0 - progress) * (1.0 - distance / 20);
                brightness = MAX(brightness, (uint8_t)(trail_intensity * 255));
            }
        }
        
        if (brightness > 0) {
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, 
                (rgb.r * brightness) / 255,
                (rgb.g * brightness) / 255,
                (rgb.b * brightness) / 255);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== KEYPRESS SONAR EFFECT =====
static bool keypress_sonar(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    uint16_t sonar_duration = 1500 + (rgb_matrix_config.speed * 5);
    HSV hsv = rgb_matrix_config.hsv;
    
    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t brightness = 0;
        
        for (uint8_t s = 0; s < MAX_SONARS; s++) {
            if (!sonars[s].active) continue;
            
            uint32_t elapsed = timer_elapsed32(sonars[s].start_time);
            if (elapsed > sonar_duration) {
                sonars[s].active = false;
                continue;
            }
            
            float progress = (float)elapsed / sonar_duration;
            float radius = progress * 60;
            
            int16_t dx = g_led_config.point[i].x - g_led_config.point[sonars[s].center].x;
            int16_t dy = g_led_config.point[i].y - g_led_config.point[sonars[s].center].y;
            float distance = sqrtf(dx * dx + dy * dy);
            
            if (fabs(distance - radius) < 2) {
                float pulse_width = 2.0;
                float sonar_intensity = (1.0 - progress) * (1.0 - fabs(distance - radius) / pulse_width);
                brightness = MAX(brightness, (uint8_t)(sonar_intensity * 255));
            }
        }
        
        if (brightness > 0) {
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, 
                (rgb.r * brightness) / 255,
                (rgb.g * brightness) / 255,
                (rgb.b * brightness) / 255);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== ORIGINAL KEYPRESS SPLASH EFFECT =====
static bool keypress_splash(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    HSV hsv = rgb_matrix_config.hsv;
    RGB rgb = hsv_to_rgb(hsv);
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

            float progress = (float)elapsed / splash_duration;
            float radius   = progress * splashes[s].max_radius;

            int16_t dx = g_led_config.point[i].x - g_led_config.point[splashes[s].center].x;
            int16_t dy = g_led_config.point[i].y - g_led_config.point[splashes[s].center].y;
            float distance = sqrtf((float)(dx * dx + dy * dy));

            if (distance <= radius) {
                float led_elapsed = elapsed - (distance / splashes[s].max_radius) * splash_duration;
                if (led_elapsed >= 0 && led_elapsed <= LED_FADE_TIME) {
                    float fade_progress = 1.0f - (led_elapsed / LED_FADE_TIME);
                    uint8_t brightness = (uint8_t)(255 * fade_progress);
                    total_brightness = MIN(255, total_brightness + brightness);
                }
            }
        }

        rgb_matrix_set_color(i,
            (rgb.r * total_brightness) / 255,
            (rgb.g * total_brightness) / 255,
            (rgb.b * total_brightness) / 255);
    }

    return rgb_matrix_check_finished_leds(led_max);
}

// ===== KEYPRESS HANDLERS =====
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        uint8_t led_index = g_led_config.matrix_co[record->event.key.row][record->event.key.col];
        if (led_index != NO_LED) {
            // Original splash effect
            uint16_t max_dist = 0;
            for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
                int16_t dx = g_led_config.point[i].x - g_led_config.point[led_index].x;
                int16_t dy = g_led_config.point[i].y - g_led_config.point[led_index].y;
                uint16_t dist = sqrt16(dx * dx + dy * dy);
                if (dist > max_dist) max_dist = dist;
            }
            max_dist += 10;

            for (uint8_t s = 0; s < MAX_SPLASHES; s++) {
                if (!splashes[s].active) {
                    splashes[s].center     = led_index;
                    splashes[s].start_time = timer_read32();
                    splashes[s].active     = true;
                    splashes[s].max_radius = max_dist;
                    break;
                }
            }
            
            // Keypress flame effect - stays active while key is held
            for (uint8_t f = 0; f < MAX_FLAMES; f++) {
                if (!flames[f].active) {
                    flames[f].center = led_index;
                    flames[f].start_time = timer_read32();
                    flames[f].last_update = timer_read32();
                    flames[f].active = true;
                    flames[f].intensity = 255;
                    flames[f].flicker_offset = rand() % 100;
                    break;
                }
            }
            
            // Keypress thunder effect
            for (uint8_t t = 0; t < MAX_THUNDERS_KEY; t++) {
                if (!key_thunders[t].active) {
                    key_thunders[t].start_x = g_led_config.point[led_index].x;
                    key_thunders[t].start_y = g_led_config.point[led_index].y;
                    key_thunders[t].end_x = key_thunders[t].start_x + (rand() % 30) - 15;
                    key_thunders[t].end_y = key_thunders[t].start_y + 20 + (rand() % 20);
                    key_thunders[t].start_time = timer_read32();
                    key_thunders[t].active = true;
                    break;
                }
            }
            
            // Other keypress effects
            for (uint8_t r = 0; r < MAX_RIPPLES_KEY; r++) {
                if (!key_ripples[r].active) {
                    key_ripples[r].center = led_index;
                    key_ripples[r].start_time = timer_read32();
                    key_ripples[r].active = true;
                    key_ripples[r].color = rgb_matrix_config.hsv;
                    break;
                }
            }
            
            for (uint8_t s = 0; s < MAX_SPARKLES; s++) {
                if (!sparkles[s].active) {
                    sparkles[s].center = led_index;
                    sparkles[s].start_time = timer_read32();
                    sparkles[s].active = true;
                    sparkles[s].lifetime = 100;
                    break;
                }
            }
            
            for (uint8_t t = 0; t < MAX_TRAILS; t++) {
                if (!trails[t].active) {
                    trails[t].center = led_index;
                    trails[t].start_time = timer_read32();
                    trails[t].active = true;
                    trails[t].direction = rand() % 4;
                    break;
                }
            }
            
            for (uint8_t s = 0; s < MAX_SONARS; s++) {
                if (!sonars[s].active) {
                    sonars[s].center = led_index;
                    sonars[s].start_time = timer_read32();
                    sonars[s].active = true;
                    sonars[s].hue = (rgb_matrix_config.hsv.h + (rand() % 60)) % 255;
                    break;
                }
            }
            
            // Ripple effect for keypress
            for (uint8_t r = 0; r < MAX_RIPPLES; r++) {
                if (!ripples[r].active) {
                    ripples[r].center = led_index;
                    ripples[r].start_time = timer_read32();
                    ripples[r].active = true;
                    ripples[r].max_radius = 40;
                    break;
                }
            }
        }
    } else {
        // Key release - deactivate flames for that key
        uint8_t led_index = g_led_config.matrix_co[record->event.key.row][record->event.key.col];
        if (led_index != NO_LED) {
            for (uint8_t f = 0; f < MAX_FLAMES; f++) {
                if (flames[f].active && flames[f].center == led_index) {
                    flames[f].active = false;
                }
            }
        }
    }
    return true;
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
