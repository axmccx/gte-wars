#pragma once

typedef struct {
    int x, y, rot, dir;
} Entity;

typedef struct {
    int x, y, vx, vy, dir, lifetime, alive;
} Bullet;
