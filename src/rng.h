#pragma once

#include <stdint.h>
#include "controller.h"

extern uint32_t rng_state;

static inline void rng_mix(const ControllerResponse *controller_response, const int frameCount) {
    uint32_t mix = 0;

    mix ^= controller_response->buttons;
    mix ^= ((uint32_t)controller_response->right_joystick) << 16;
    mix ^= controller_response->left_joystick * 0x45d9f3u;
    mix ^= (uint32_t)frameCount * 0x9e3779b9u;

    rng_state ^= mix;
}

static inline uint32_t rng_next() {
    uint32_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng_state = x ? x : 0xA634716A;
    return x;
}

static inline int rand_range(const int min, const int max) {
    const uint32_t r = rng_next();
    const int span = max - min + 1;
    return min + (int)(r % span);
}
