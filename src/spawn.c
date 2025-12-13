#include "math.h"
#include "rng.h"
#include "world.h"
#include "spawn.h"

void spawnStateTick(World *world) {
    world->nextWaveTimer--;
    if (world->nextWaveTimer == 0) {
        world->nextWaveTimer = BASE_WAVE_TIMER;
        runWave(world);
    } else if (world->frameCount % world->baseSpawnRate == 0) {
        int x, y;
        do {
            x = rand_range(-(PLAYFIELD_HALF_WIDTH - 100), PLAYFIELD_HALF_WIDTH - 100);
            y = rand_range(-(PLAYFIELD_HALF_HEIGHT - 100), PLAYFIELD_HALF_HEIGHT - 100);
        } while (isWithinRange(x, y, world->player.x, world->player.y, 800));

        spawnEnemy(world, x, y, 0);
    }
}

void runWave(World *world) {
    int spawnCount = 50;

    static const int8_t cornerSigns[4][2] = {
        { -1,  1 },
        {  1,  1 },
        {  1, -1 },
        { -1, -1 }
    };

    while (spawnCount > 0) {
        const int corner = spawnCount % 4;
        const int xSign = cornerSigns[corner][0];
        const int ySign = cornerSigns[corner][1];

        const int rx = rand_range(0, 511);
        const int ry = rand_range(0, 511);

        const int x = xSign * (PLAYFIELD_HALF_WIDTH - rx);
        const int y = ySign * (PLAYFIELD_HALF_HEIGHT - ry);

        spawnEnemy(world, x, y, 50);
        spawnCount--;
    }
}

void spawnEnemy(World *world, const int x, const int y, const int cooldown) {
    int vx = rand_range(-128, 127);
    int vy = rand_range(-128, 127);
    normalize_direction(&vx, &vy);

    const Enemy newEnemy = {
        .x = x,
        .y = y,
        .rot = 0,
        .vx = vx,
        .vy = vy,
        .alive = 1,
        .cooldown = cooldown,
        .model = world->models.enemy,
    };

    const int start = world->nextFreeEnemy;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        const int idx = (start + i) % MAX_ENEMIES;

        if (!world->enemies[idx].alive) {
            world->nextFreeEnemy = (idx + 1) % MAX_ENEMIES;
            world->enemies[idx] = newEnemy;
            return;
        }
    }
}
