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
    const uint32_t span = max - min + 1;

    uint32_t scaled;
    if ((span & (span - 1)) == 0) { // power-of-two span
        scaled = r & (span - 1);
    } else {
        scaled = (uint32_t)(((uint64_t)r * span) >> 32);
    }

    return (int)(min + scaled);
}

static inline int depth_jitter(const int index, const int baseDepth) {
    const int jitter = ((index * 37) & 0x1FF) - 256;

    int d = baseDepth + jitter;

    if (d < 512) d = 512;
    if (d > baseDepth + 512) d = baseDepth + 512;

    return d;
}
