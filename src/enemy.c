#include "enemy.h"
#include "model.h"
#include "stdlib.h"
#include "world.h"

EnemyDefinition enemyDefs[ENEMY_COUNT] = {};


void initEnemyDefinitions(World *world) {
    enemyDefs[ENEMY_WANDERER] = (EnemyDefinition){
        .type = ENEMY_WANDERER,
        .model = world->models.enemyWanderer,
        .speed = 8,
        .updateFn = updateWanderer,
    };
    enemyDefs[ENEMY_CHASER] = (EnemyDefinition){
        .type = ENEMY_CHASER,
        .model = world->models.enemyChaser,
        .speed = 18,
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

        enemy->vx += (dx * CHASER_TURN_RATE) >> 12;
        enemy->vy += (dy * CHASER_TURN_RATE) >> 12;

        // speed clamp
        int mag = abs(enemy->vx) + abs(enemy->vy);
        if (mag > MAX_CHASER_SPEED) {
            enemy->vx = (enemy->vx * MAX_CHASER_SPEED) / mag;
            enemy->vy = (enemy->vy * MAX_CHASER_SPEED) / mag;
        }

        enemy->x += (enemy->vx * enemy->def->speed) >> 12;
        enemy->y += (enemy->vy * enemy->def->speed) >> 12;

        bounce_axis(&enemy->x, &enemy->vx, PLAYFIELD_HALF_WIDTH  - 64);
        bounce_axis(&enemy->y, &enemy->vy, PLAYFIELD_HALF_HEIGHT - 64);
    }

    const int rot_step = 32;
    const int rot_limit = 512;

    enemy->rot += enemy->rot_dir * rot_step;

    if (enemy->rot > rot_limit) {
        enemy->rot = rot_limit - (enemy->rot - rot_limit);
        enemy->rot_dir = -1;
    }
    else if (enemy->rot < -rot_limit) {
        enemy->rot = -rot_limit - (enemy->rot + rot_limit);
        enemy->rot_dir = 1;
    }
}
