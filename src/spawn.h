#pragma once
#include "enemy.h"

#define BASE_WAVE_TIMER 1000

void spawnStateTick(World *world);

void runWave(World *world);

void spawnEnemy(World *world, EnemyType type, int x, int y, int cooldown);
