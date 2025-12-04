#pragma once
#include "entity.h"
#include "model.h"

#define MAX_ENEMIES 20
#define MAX_BULLETS 30
#define MAX_SPEED 30
#define BULLET_SPEED 50
#define ENEMY_SPEED 10

typedef struct {
    int frameCount;
    int nextFreeBullet;
    int nextFreeEnemy;
    Player player;
    Bullet bullets[MAX_BULLETS];
    Enemy enemies[MAX_ENEMIES];
    Models models;
} World;

static inline void normalize_direction(int *vx, int *vy) {
    int x = *vx;
    int y = *vy;

    if (x == 0 && y == 0)
        x = 1;

    int ax = (x < 0) ? -x : x;
    int ay = (y < 0) ? -y : y;

    int mag = ax + ay;
    if (mag == 0) mag = 1;

    *vx = (x << 12) / mag;
    *vy = (y << 12) / mag;
}

void worldInit(World *world);

void updatePlayer(World *world, ControllerResponse controller_response);

void spawnBullets(World *world, ControllerResponse controller_response);

void updateBullets(World *world);

void spawnEnemies(World *world);

void updateEnemies(World *world);
