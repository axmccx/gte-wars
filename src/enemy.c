#include "enemy.h"
#include "model.h"
#include "world.h"

EnemyDefinition enemyDefs[ENEMY_COUNT] = {};


void initEnemyDefinitions(World *world) {
    enemyDefs[ENEMY_WANDERER] = (EnemyDefinition){
        .type = ENEMY_WANDERER,
        .model = world->models.enemyWanderer,
        .updateFn = updateWander,
    };
    // enemyDefs[ENEMY_CHASER] = (EnemyDefinition){
    //     .type = ENEMY_CHASER,
    //     .model = world->models.enemyChaser,
    //     .updateFn = updateChaser,
    // };
    //
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

void updateWander(Enemy *enemy, World *world) {
    if (enemy->cooldown > 0) {
        enemy->cooldown--;
    } else {
        enemy->x += (enemy->vx * ENEMY_SPEED) >> 12;
        enemy->y += (enemy->vy * ENEMY_SPEED) >> 12;
        bounce_axis(&enemy->x, &enemy->vx, PLAYFIELD_HALF_WIDTH  - 64);
        bounce_axis(&enemy->y, &enemy->vy, PLAYFIELD_HALF_HEIGHT - 64);
    }

    enemy->rot += 32;
}
