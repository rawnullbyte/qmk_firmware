// !!! DO NOT ADD #pragma once !!! //

RGB_MATRIX_EFFECT(keypress_splash)
RGB_MATRIX_EFFECT(rain_drops)
RGB_MATRIX_EFFECT(flame_breath)
RGB_MATRIX_EFFECT(cosmic_swirl)
RGB_MATRIX_EFFECT(pulse_wave)
RGB_MATRIX_EFFECT(starfield)
RGB_MATRIX_EFFECT(plasma_cloud)
RGB_MATRIX_EFFECT(keypress_flame)
RGB_MATRIX_EFFECT(breathe_cycle)
RGB_MATRIX_EFFECT(ripple_pool)

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

// Rain drops effect
#define MAX_DROPS 8
typedef struct {
    uint8_t x;
    float y;
    float speed;
    bool active;
    HSV color;
} rain_drop_t;

static rain_drop_t rain_drops[MAX_DROPS];
static uint32_t last_rain_spawn = 0;

// Flame breath effect
static float flame_time = 0;
static uint8_t flame_base_hue = 20; // Orange base

// Cosmic swirl effect
static float cosmic_angle = 0;

// Pulse wave effect  
static float pulse_wave_time = 0;

// Starfield effect
#define MAX_STARS 20
typedef struct {
    uint8_t x, y;
    float brightness;
    float speed;
} star_t;

static star_t stars[MAX_STARS];

// Plasma cloud effect
static float plasma_time = 0;

// Keypress flame effect
#define MAX_FLAMES 10
typedef struct {
    uint8_t center;
    uint32_t start_time;
    bool active;
    uint8_t intensity;
} flame_t;

static flame_t flames[MAX_FLAMES];

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

// ===== RAIN DROPS EFFECT =====
static bool rain_drops(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    HSV hsv = rgb_matrix_config.hsv;
    uint16_t time = timer_elapsed32(0);
    
    // Spawn new drops occasionally
    if (time - last_rain_spawn > 200 + (255 - rgb_matrix_config.speed) * 2) {
        last_rain_spawn = time;
        for (uint8_t i = 0; i < MAX_DROPS; i++) {
            if (!rain_drops[i].active) {
                rain_drops[i].x = rand() % 224;
                rain_drops[i].y = 0;
                rain_drops[i].speed = 0.5 + (rand() % 100) * 0.01;
                rain_drops[i].active = true;
                rain_drops[i].color = (HSV){hsv.h + (rand() % 30) - 15, 200 + rand() % 55, hsv.v};
                break;
            }
        }
    }
    
    // Update and render drops
    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t brightness = 0;
        
        for (uint8_t d = 0; d < MAX_DROPS; d++) {
            if (!rain_drops[d].active) continue;
            
            // Update drop position
            rain_drops[d].y += rain_drops[d].speed;
            if (rain_drops[d].y > 128) {
                rain_drops[d].active = false;
                continue;
            }
            
            // Calculate distance to drop
            int16_t dx = abs(g_led_config.point[i].x - rain_drops[d].x);
            int16_t dy = abs(g_led_config.point[i].y - (int)rain_drops[d].y);
            float distance = sqrtf(dx * dx + dy * dy);
            
            if (distance < 8) {
                uint8_t drop_bright = (uint8_t)(255 * (1.0 - distance / 8));
                brightness = MAX(brightness, drop_bright);
            }
        }
        
        if (brightness > 0) {
            RGB rgb = hsv_to_rgb(rain_drops[0].color); // Use first drop's color
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

// ===== FLAME BREATH EFFECT (Dynamic) =====
static bool flame_breath(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    flame_time += 0.02;
    uint8_t breath = (sin(flame_time) + 1.0) * 127.5;
    
    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t x = g_led_config.point[i].x;
        uint8_t y = g_led_config.point[i].y;
        
        // Flame-like noise pattern
        float noise = sin(x * 0.1 + flame_time) * cos(y * 0.1 + flame_time);
        uint8_t base_bright = (uint8_t)(breath * (0.7 + noise * 0.3));
        
        // Orange to yellow gradient
        uint8_t hue = flame_base_hue + (y * 10) / 128;
        HSV hsv = {hue, 255, base_bright};
        RGB rgb = hsv_to_rgb(hsv);
        
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== COSMIC SWIRL EFFECT (Dynamic) =====
static bool cosmic_swirl(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    cosmic_angle += 0.03;
    float scale = 0.1 + (rgb_matrix_config.speed / 255.0) * 0.2;
    
    for (uint8_t i = led_min; i < led_max; i++) {
        float x = (g_led_config.point[i].x - 112) / 112.0;
        float y = (g_led_config.point[i].y - 64) / 64.0;
        
        float angle = atan2(y, x) + cosmic_angle;
        float radius = sqrt(x * x + y * y);
        
        float swirl = sin(radius * 20 - cosmic_angle * 3) * 0.5 + 0.5;
        uint8_t hue = (uint8_t)((angle / (2 * M_PI) + radius + cosmic_angle) * 128);
        uint8_t bright = (uint8_t)(swirl * rgb_matrix_config.hsv.v);
        
        HSV hsv = {hue, 255, bright};
        RGB rgb = hsv_to_rgb(hsv);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== PULSE WAVE EFFECT (Dynamic) =====
static bool pulse_wave(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    pulse_wave_time += 0.05;
    float wave = (sin(pulse_wave_time) + 1.0) * 0.5;
    
    for (uint8_t i = led_min; i < led_max; i++) {
        float x = g_led_config.point[i].x / 224.0;
        float distance = fabs(x - wave);
        
        uint8_t brightness = (uint8_t)((1.0 - distance * 2) * 255);
        if (brightness > 10) {
            HSV hsv = {rgb_matrix_config.hsv.h, rgb_matrix_config.hsv.s, brightness};
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== STARFIELD EFFECT (Dynamic) =====
static bool starfield(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    // Initialize stars
    static bool initialized = false;
    if (!initialized) {
        for (uint8_t s = 0; s < MAX_STARS; s++) {
            stars[s].x = rand() % 224;
            stars[s].y = rand() % 128;
            stars[s].brightness = (rand() % 100) * 0.01;
            stars[s].speed = 0.1 + (rand() % 50) * 0.01;
        }
        initialized = true;
    }
    
    // Update stars
    for (uint8_t s = 0; s < MAX_STARS; s++) {
        stars[s].brightness += stars[s].speed * 0.02;
        if (stars[s].brightness > 1.0) {
            stars[s].x = rand() % 224;
            stars[s].y = rand() % 128;
            stars[s].brightness = 0;
            stars[s].speed = 0.1 + (rand() % 50) * 0.01;
        }
    }
    
    // Render stars
    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t brightness = 0;
        
        for (uint8_t s = 0; s < MAX_STARS; s++) {
            int16_t dx = abs(g_led_config.point[i].x - stars[s].x);
            int16_t dy = abs(g_led_config.point[i].y - stars[s].y);
            
            if (dx < 3 && dy < 3) {
                float distance = sqrt(dx * dx + dy * dy);
                uint8_t star_bright = (uint8_t)(stars[s].brightness * 255 * (1.0 - distance / 3));
                brightness = MAX(brightness, star_bright);
            }
        }
        
        if (brightness > 0) {
            rgb_matrix_set_color(i, brightness, brightness, brightness);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== PLASMA CLOUD EFFECT (Dynamic) =====
static bool plasma_cloud(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    plasma_time += 0.02;
    
    for (uint8_t i = led_min; i < led_max; i++) {
        float x = g_led_config.point[i].x / 224.0 * 4.0;
        float y = g_led_config.point[i].y / 128.0 * 4.0;
        
        float v1 = sin(x + plasma_time);
        float v2 = sin((x + y) * 0.5 + plasma_time);
        float v3 = sin(sqrt(x*x + y*y) + plasma_time);
        
        float plasma = (v1 + v2 + v3) / 3.0;
        uint8_t hue = (uint8_t)((plasma + 1.0) * 127.5);
        uint8_t bright = (uint8_t)((plasma * 0.5 + 0.5) * rgb_matrix_config.hsv.v);
        
        HSV hsv = {hue, 255, bright};
        RGB rgb = hsv_to_rgb(hsv);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== KEYPRESS FLAME EFFECT =====
static bool keypress_flame(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    uint16_t flame_duration = 800 + (rgb_matrix_config.speed * 3);
    
    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t total_brightness = 0;
        
        for (uint8_t f = 0; f < MAX_FLAMES; f++) {
            if (!flames[f].active) continue;
            
            uint32_t elapsed = timer_elapsed32(flames[f].start_time);
            if (elapsed > flame_duration) {
                flames[f].active = false;
                continue;
            }
            
            float progress = (float)elapsed / flame_duration;
            float radius = progress * 15; // 3-key radius approx
            
            int16_t dx = g_led_config.point[i].x - g_led_config.point[flames[f].center].x;
            int16_t dy = g_led_config.point[i].y - g_led_config.point[flames[f].center].y;
            float distance = sqrtf(dx * dx + dy * dy);
            
            if (distance <= radius) {
                // Orange to yellow gradient based on height
                float intensity = 1.0 - progress;
                uint8_t brightness = (uint8_t)(255 * intensity * (1.0 - distance / radius));
                total_brightness = MIN(255, total_brightness + brightness);
            }
        }
        
        if (total_brightness > 0) {
            // Orange-yellow flame color
            uint8_t r = 255;
            uint8_t g = (total_brightness * 200) / 255;
            uint8_t b = (total_brightness * 50) / 255;
            rgb_matrix_set_color(i, r, g, b);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== BREATHE CYCLE EFFECT (Dynamic) =====
static bool breathe_cycle(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    static float breathe_time = 0;
    breathe_time += 0.01;
    
    uint8_t breath = (sin(breathe_time) + 1.0) * 127.5;
    
    HSV hsv = rgb_matrix_config.hsv;
    hsv.v = breath;
    RGB rgb = hsv_to_rgb(hsv);
    
    for (uint8_t i = led_min; i < led_max; i++) {
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}

// ===== RIPPLE POOL EFFECT (Mostly Dynamic) =====
static bool ripple_pool(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    uint16_t ripple_duration = 1500 + (rgb_matrix_config.speed * 6);
    
    // Auto-spawn ripples occasionally
    uint32_t time = timer_elapsed32(0);
    if (time - last_auto_ripple > 3000 - (rgb_matrix_config.speed * 10)) {
        last_auto_ripple = time;
        for (uint8_t r = 0; r < MAX_RIPPLES; r++) {
            if (!ripples[r].active) {
                ripples[r].center = rand() % RGB_MATRIX_LED_COUNT;
                ripples[r].start_time = time;
                ripples[r].active = true;
                ripples[r].max_radius = 50 + rand() % 30;
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
            
            if (fabs(distance - radius) < 3) {
                float ripple_bright = sin(progress * M_PI) * (1.0 - fabs(distance - radius) / 3);
                brightness = MAX(brightness, (uint8_t)(ripple_bright * 255));
            }
        }
        
        if (brightness > 0) {
            HSV hsv = {rgb_matrix_config.hsv.h, rgb_matrix_config.hsv.s, brightness};
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
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
            
            // Keypress flame effect
            for (uint8_t f = 0; f < MAX_FLAMES; f++) {
                if (!flames[f].active) {
                    flames[f].center = led_index;
                    flames[f].start_time = timer_read32();
                    flames[f].active = true;
                    flames[f].intensity = 255;
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
    }
    return true;
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
