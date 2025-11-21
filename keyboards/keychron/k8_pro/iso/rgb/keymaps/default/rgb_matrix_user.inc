RGB_MATRIX_EFFECT(RGB_COOL_SPLASH)
#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static hsv_t RGB_COOL_SPLASH_math(hsv_t hsv, uint8_t i, uint8_t time) {
    hsv.v = 255;
    hsv.s = 255;

    uint8_t t = (time + i * 6) % 256;
    switch (t / (256 / 4)) {
        case 0:
            hsv.h = 0;
            break;
        case 1:
            hsv.h = 85;
            break;
        case 2:
            hsv.h = 170;
            break;
        default:
            hsv.s = 0;
    }
    return hsv;
}

static bool RGB_COOL_SPLASH(effect_params_t* params) { return effect_runner_i(params, &RGB_COOL_SPLASH_math); }
#endif
