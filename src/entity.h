#pragma once
#include "model.h"

typedef struct {
    int x, y, rot, dir, alive;
} Player;

typedef struct {
    int x, y, vx, vy, dir, alive;
} Bullet;

typedef struct {
    int x, y, rot, vx, vy, alive;
    ObjModel *model;
} Enemy;

typedef struct {
    int x, y, vx, vy, lifetime;
} Particle;
