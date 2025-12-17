#pragma once
#include "model.h"

#define ENEMY_COUNT 2
#define CHASER_TURN_RATE 256
#define MAX_CHASER_SPEED  4096

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
    EnemyUpdateFn updateFn;
} EnemyDefinition;

typedef struct Enemy {
    int x, y, rot, rot_dir, vx, vy, alive, cooldown;
    EnemyDefinition *def;
} Enemy;

extern EnemyDefinition enemyDefs[ENEMY_COUNT];


void initEnemyDefinitions(World *world);

void updateEnemies(World *world);

void updateWanderer(Enemy *enemy, World *world);

void updateChaser(Enemy *enemy, World *world);
