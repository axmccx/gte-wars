#include "enemy.h"
#include "model.h"
#include "rng.h"
#include "stdlib.h"
#include "world.h"

EnemyDefinition enemyDefs[ENEMY_COUNT] = {};


void initEnemyDefinitions(World *world) {
    enemyDefs[ENEMY_WANDERER] = (EnemyDefinition){
        .type = ENEMY_WANDERER,
        .model = world->models.enemyWanderer,
        .speed = 8,
        .turnRate = 384,
        .vLimit = 4096,
        .updateFn = updateWanderer,
    };
    enemyDefs[ENEMY_CHASER] = (EnemyDefinition){
        .type = ENEMY_CHASER,
        .model = world->models.enemyChaser,
        .speed = 18,
        .turnRate = 256,
        .vLimit = 4096,
        .updateFn = updateChaser,
    };
    // enemyDefs[ENEMY_EVADER] = (EnemyDefinition){
    //     .type = ENEMY_EVADER,
    //     .model = world->models.enemyEvader,
    //     .updateFn = updateEvade,
    // };
}


void updateEnemies(World *world) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *enemy = &world->enemies[i];

        if (!enemy->alive) continue;

        enemy->def->updateFn(enemy, world);
    }
}

void updateWanderer(Enemy *enemy, World *world) {
    if (enemy->cooldown > 0) {
        enemy->cooldown--;
    } else if (world->state == GAMESTATE_PLAYING)  {
        if ((world->frameCount & 7) == 0) {
            enemy->steerX += rand_range(-512, 512);
            enemy->steerY += rand_range(-512, 512);
        }

        const int wanderLimit = 1024;
        const int wanderMag = abs(enemy->steerX) + abs(enemy->steerY);
        if (wanderMag > wanderLimit) {
            enemy->steerX = (enemy->steerX * wanderLimit) / wanderMag;
            enemy->steerY = (enemy->steerY * wanderLimit) / wanderMag;
        }

        enemy->vx += (enemy->steerX * enemy->def->turnRate) >> 12;
        enemy->vy += (enemy->steerY * enemy->def->turnRate) >> 12;

        const int mag = abs(enemy->vx) + abs(enemy->vy);
        if (mag > 0) {
            enemy->vx = (enemy->vx * enemy->def->vLimit) / mag;
            enemy->vy = (enemy->vy * enemy->def->vLimit) / mag;
        }

        enemy->x += (enemy->vx * enemy->def->speed) >> 12;
        enemy->y += (enemy->vy * enemy->def->speed) >> 12;

        bounce_axis(&enemy->x, &enemy->vx, PLAYFIELD_HALF_WIDTH  - 64);
        bounce_axis(&enemy->y, &enemy->vy, PLAYFIELD_HALF_HEIGHT - 64);
    }

    enemy->rot += 32;
}

void updateChaser(Enemy *enemy, World *world) {
    if (enemy->cooldown > 0) {
        enemy->cooldown--;
    } else if (world->state == GAMESTATE_PLAYING) {
        int dx = world->player.x - enemy->x;
        int dy = world->player.y - enemy->y;
        normalize_direction(&dx, &dy);

        enemy->vx += (dx * enemy->def->turnRate) >> 12;
        enemy->vy += (dy * enemy->def->turnRate) >> 12;

        const int mag = abs(enemy->vx) + abs(enemy->vy);
        if (mag > enemy->def->vLimit) {
            enemy->vx = (enemy->vx * enemy->def->vLimit) / mag;
            enemy->vy = (enemy->vy * enemy->def->vLimit) / mag;
        }

        enemy->x += (enemy->vx * enemy->def->speed) >> 12;
        enemy->y += (enemy->vy * enemy->def->speed) >> 12;

        bounce_axis(&enemy->x, &enemy->vx, PLAYFIELD_HALF_WIDTH  - 64);
        bounce_axis(&enemy->y, &enemy->vy, PLAYFIELD_HALF_HEIGHT - 64);
    }

    const int rot_step = 32;
    const int rot_limit = 512;

    enemy->rot += enemy->rotDir * rot_step;

    if (enemy->rot > rot_limit) {
        enemy->rot = rot_limit - (enemy->rot - rot_limit);
        enemy->rotDir = -1;
    }
    else if (enemy->rot < -rot_limit) {
        enemy->rot = -rot_limit - (enemy->rot + rot_limit);
        enemy->rotDir = 1;
    }
}
