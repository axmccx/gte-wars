#pragma once
#include "model.h"

#define ENEMY_COUNT 2

typedef struct Enemy Enemy;
typedef struct World World;

typedef enum {
    ENEMY_WANDERER = 0,
    ENEMY_CHASER = 1,
    // ENEMY_EVADER,
} EnemyType;

typedef void (*EnemyUpdateFn)(Enemy *, World *);

typedef struct {
    EnemyType type;
    ObjModel *model;
    int speed;
    int vLimit;
    int turnRate;
    EnemyUpdateFn updateFn;
} EnemyDefinition;

typedef struct Enemy {
    int x, y, rot, rotDir;
    int vx, vy;
    int steerX, steerY;
    int alive, cooldown;
    EnemyDefinition *def;
} Enemy;

extern EnemyDefinition enemyDefs[ENEMY_COUNT];


void initEnemyDefinitions(World *world);

void updateEnemies(World *world);

void updateWanderer(Enemy *enemy, World *world);

void updateChaser(Enemy *enemy, World *world);
