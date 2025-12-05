#pragma once
#include "entity.h"
#include "controller.h"
#include "model.h"

#define MAX_ENEMIES 20
#define MAX_BULLETS 30
#define MAX_SPEED 30
#define BULLET_SPEED 50
#define BULLET_TIP_OFFSET 56
#define ENEMY_SPEED 10
#define ENEMY_HIT_RADIUS 80
#define CAMERA_PAN_FACTOR 2560
#define PLAYFIELD_HALF_WIDTH 2000
#define PLAYFIELD_HALF_HEIGHT 1500
#define JOYSTICK_DEAD_ZONE 64

typedef struct {
    int x, y;
} Camera;

typedef struct {
    int frameCount;
    int nextFreeBullet;
    int nextFreeEnemy;
    Camera camera;
    Player player;
    Bullet bullets[MAX_BULLETS];
    Enemy enemies[MAX_ENEMIES];
    Models models;
} World;

static inline void normalize_direction(int *vx, int *vy) {
    int x = *vx;
    const int y = *vy;

    if (x == 0 && y == 0)
        x = 1;

    const int ax = (x < 0) ? -x : x;
    const int ay = (y < 0) ? -y : y;

    int mag = ax + ay;
    if (mag == 0) mag = 1;

    *vx = (x << 12) / mag;
    *vy = (y << 12) / mag;
}

static inline int limit_axis(int *pos, int limit) {
    if (*pos > limit) {
        *pos = limit;
        return 1;
    }
    if (*pos < -limit) {
        *pos = -limit;
        return 1;
    }
    return 0;
}

static inline void clamp_axis(int *pos, const int limit) {
    (void)limit_axis(pos, limit);
}

static inline void bounce_axis(int *pos, int *vel, const int limit) {
    if (limit_axis(pos, limit)) *vel = -*vel;
}

void worldInit(World *world);

void updatePlayer(World *world, ControllerResponse controller_response);

void spawnBullets(World *world, ControllerResponse controller_response);

void updateBullets(World *world);

void spawnEnemies(World *world);

void detectBulletEnemyCollisions(World *world);

void updateEnemies(World *world);
