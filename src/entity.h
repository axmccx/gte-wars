#pragma once
#include "model.h"

typedef struct {
    int x, y, rot, dir;
} Player;

typedef struct {
    int x, y, vx, vy, dir, alive;
} Bullet;

typedef struct {
    int x, y, rot, vx, vy, alive;
    ObjModel *model;
} Enemy;
