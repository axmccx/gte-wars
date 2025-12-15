#pragma once
#include "model.h"

#define ENEMY_COUNT 1

typedef struct Enemy Enemy;
typedef struct World World;

typedef enum {
    ENEMY_WANDERER,
    // ENEMY_EVADER,
    // ENEMY_CHASER,
} EnemyType;

typedef void (*EnemyUpdateFn)(Enemy *, World *);

typedef struct {
    EnemyType type;
    ObjModel *model;
    EnemyUpdateFn updateFn;
} EnemyDefinition;

typedef struct Enemy {
    int x, y, rot, vx, vy, alive, cooldown;
    EnemyDefinition *def;
} Enemy;

extern EnemyDefinition enemyDefs[ENEMY_COUNT];


void initEnemyDefinitions(World *world);

void updateEnemies(World *world);

void updateWander(Enemy *enemy, World *world);
