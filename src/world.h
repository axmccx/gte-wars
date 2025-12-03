#pragma once
#include "entity.h"
#include "model.h"

#define MAX_ENEMIES 5
#define MAX_BULLETS 30
#define MAX_SPEED 30
#define BULLET_SPEED 50

typedef struct {
    int frameCount;
    int nextFreeBullet;
    Entity player;
    Bullet bullets[MAX_BULLETS];
    Entity enemies[MAX_ENEMIES];
    Models models;
} World;

void worldInit(World *world);

void updatePlayer(World *world, ControllerResponse controller_response);

void spawnBullets(World *world, ControllerResponse controller_response);

void updateBullets(World *world);
