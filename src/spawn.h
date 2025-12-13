#pragma once
#include "world.h"

#define BASE_WAVE_TIMER 1000

void spawnStateTick(World *world);

void runWave(World *world);

void spawnEnemy(World *world, int x, int y, int cooldown);
