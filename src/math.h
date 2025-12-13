#pragma once

static inline bool isWithinRange(
    const int x1,
    const int y1,
    const int x2,
    const int y2,
    const int range
) {
    const int dx = x1 - x2;
    const int dy = y1 - y2;

    return dx*dx + dy*dy < range*range;
}
