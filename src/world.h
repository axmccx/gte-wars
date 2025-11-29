#pragma once
#include "entity.h"

#define MAX_ENEMIES 5
#define MAX_BULLETS 100

typedef struct {
    Entity player;
    Entity enemies[MAX_ENEMIES];
    Entity bullets[MAX_BULLETS];
} World;

void worldInit(World *world);
void updatePlayerPosition(World *world, uint16_t buttons);
