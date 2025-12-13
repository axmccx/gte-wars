#include "controller.h"
#include "world.h"
#include "render.h"
#include "stdlib.h"
#include "ps1/trig.h"
#include "rng.h"
#include "math.h"
#include "spawn.h"

void worldInit(World *world, const GameState state) {
    world->state = state;
    world->frameCount = 0;
    world->nextFreeBullet = 0;
    world->nextFreeEnemy = 0;
    world->nextFreeParticle = 0;
    world->score = 0;
    world->lives = 3;
    world->respawnTimer = 0;
    world->polycount = 0;
    world->baseSpawnRate = 80;
    world->nextWaveTimer = BASE_WAVE_TIMER;
    world->lastButtons = 0;
    world->camera.x = 0;
    world->camera.y = 0;
    world->player.x = 0;
    world->player.y = 0;
    world->player.rot = 0;
    world->player.dir = 0;
    world->player.alive = 1;

    for (int i = 0; i < MAX_BULLETS; i++) {
        world->bullets[i].alive = 0;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        world->enemies[i].alive = 0;
    }

    for (int i = 0; i < MAX_PARTICLES; i++) {
        world->particles[i].lifetime = 0;
    }

    world->models.player = malloc(sizeof *world->models.player);
    loadObjModel(world->models.player, playerShipObj);

    world->models.bullet = malloc(sizeof *world->models.bullet);
    loadObjModel(world->models.bullet, bulletObj);

    world->models.enemy = malloc(sizeof *world->models.enemy);
    loadObjModel(world->models.enemy, octahedronObj);

    world->models.smallParticle = malloc(sizeof *world->models.smallParticle);
    generateParticle(world->models.smallParticle, SMALL_PARTICLE, COLOR_RED);

    world->models.mediumParticle = malloc(sizeof *world->models.mediumParticle);
    generateParticle(world->models.mediumParticle, MEDIUM_PARTICLE, COLOR_GREEN);

    world->models.largeParticle = malloc(sizeof *world->models.largeParticle);
    generateParticle(world->models.largeParticle, LARGE_PARTICLE, COLOR_CYAN);
}

static const int dpad_to_angle[3][3] = {
    { 3584, 3072, 2560 },
    { 0, -1, 2048 },
    { 512, 1024, 1536 },
};

void updatePlayer(World *world, const ControllerResponse controller_response) {
    if (!world->player.alive) {
        if (world->respawnTimer > 0) {
            world->respawnTimer--;
        } else if (world->lives > 0) {
            world->lives--;
            world->player.alive = 1;
        }
        return;
    }

    const bool right = (controller_response.buttons & BUTTON_RIGHT) != 0;
    const bool left = (controller_response.buttons & BUTTON_LEFT) != 0;
    const bool down = (controller_response.buttons & BUTTON_DOWN) != 0;
    const bool up = (controller_response.buttons & BUTTON_UP) != 0;

    const int dx = right ? 1 : (left ? -1 : 0);
    const int dy = down ? 1 : (up ? -1: 0);

    const uint8_t stick_x_raw = controller_response.left_joystick & 0xFF;
    const uint8_t stick_y_raw = (controller_response.left_joystick >> 8) & 0xFF;
    const int stick_x = stick_x_raw - 128;
    const int stick_y = stick_y_raw - 128;

    const int abs_x = (stick_x < 0) ? -stick_x : stick_x;
    const int abs_y = (stick_y < 0) ? -stick_y : stick_y;
    const int magnitude = abs_x + abs_y;

    if (magnitude > JOYSTICK_DEAD_ZONE) {
        world->player.x += (stick_x * MAX_PLAYER_SPEED) / 128;
        world->player.y += (stick_y * MAX_PLAYER_SPEED) / 128;
        world->player.dir = atan2(stick_y, stick_x);
    } else {
        world->player.x += dx * MAX_PLAYER_SPEED;
        world->player.y += dy * MAX_PLAYER_SPEED;

        const int dir_angle = dpad_to_angle[dx+1][dy+1];
        if (dir_angle >= 0) {
            world->player.dir = dir_angle;
        }
    }

    clamp_axis(&world->player.x, PLAYFIELD_HALF_WIDTH - 100);
    clamp_axis(&world->player.y, PLAYFIELD_HALF_HEIGHT - 100);
    world->player.rot += 32;
}

void detectPlayerEnemyCollisions(World *world) {
    Player *player = &world->player;
    const int r = ENEMY_HIT_RADIUS + PLAYER_HIT_RADIUS;

    for (int e = 0; e < MAX_ENEMIES; e++) {
        Enemy *enemy = &world->enemies[e];
        if (!enemy->alive) continue;

        if (isWithinRange(enemy->x, enemy->y, player->x, player->y, r)) {
            enemy->alive = 0;
            player->alive = 0;
            world->respawnTimer = 50;
            emitParticles(world, MEDIUM_PARTICLE, 60, 50, 16, enemy->x, enemy->y);
            emitParticles(world, LARGE_PARTICLE, 30, 50, 32, enemy->x, enemy->y);
            emitParticles(world, LARGE_PARTICLE, 20, 50, 64, enemy->x, enemy->y);
            break;
        }
    }
}

void fireBullets(World *world, const ControllerResponse controller_response) {
    const uint8_t stick_x_raw = controller_response.right_joystick & 0xFF;
    const uint8_t stick_y_raw = (controller_response.right_joystick >> 8) & 0xFF;

    const int stick_x = stick_x_raw - 128;
    const int stick_y = stick_y_raw - 128;

    const int abs_x = (stick_x < 0) ? -stick_x : stick_x;
    const int abs_y = (stick_y < 0) ? -stick_y : stick_y;
    const int magnitude = abs_x + abs_y;

    if (magnitude > JOYSTICK_DEAD_ZONE && world->frameCount % 5 == 0) {
        int vx = stick_x;
        int vy = stick_y;
        normalize_direction(&vx, &vy);

        const int BULLET_OFFSET = 80;

        const int spawn_x = world->player.x + ((vx * BULLET_OFFSET) >> 12);
        const int spawn_y = world->player.y + ((vy * BULLET_OFFSET) >> 12);

        const Bullet newBullet = {
            .x = spawn_x,
            .y = spawn_y,
            .vx = vx,
            .vy = vy,
            .dir = atan2(stick_y, stick_x),
            .alive = 1,
        };

        const int start = world->nextFreeBullet;

        for (int i = 0; i < MAX_BULLETS; i++) {
            const int idx = (start + i) % MAX_BULLETS;

            if (!world->bullets[idx].alive) {
                world->nextFreeBullet = (idx + 1) % MAX_BULLETS;
                world->bullets[idx] = newBullet;
                return;
            }
        }
    }
}

void updateBullets(World *world) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet *bullet = &world->bullets[i];

        if (!bullet->alive) continue;

        const int off_x = limit_axis(&bullet->x, PLAYFIELD_HALF_WIDTH - 100);
        const int off_y = limit_axis(&bullet->y, PLAYFIELD_HALF_HEIGHT - 100);

        if (off_x || off_y) {
            bullet->alive = 0;
            continue;
        }

        bullet->x += (bullet->vx * BULLET_SPEED) >> 12;
        bullet->y += (bullet->vy * BULLET_SPEED) >> 12;
    }
}

static ObjModel* getParticleModel(const Models* m, ParticleType k) {
    switch (k) {
        case SMALL_PARTICLE: return m->smallParticle;
        case MEDIUM_PARTICLE: return m->mediumParticle;
        case LARGE_PARTICLE:  return m->largeParticle;
    }
    return m->smallParticle;
}

void emitParticles(
    World *world,
    const ParticleType type,
    const int count,
    const int lifetime,
    const int speedSeed,
    const int spawnX,
    const int spawnY
) {
    for (int i = 0; i < count; i++) {
        int vx = rand_range(-128, 127);
        int vy = rand_range(-128, 127);
        normalize_direction(&vx, &vy);
        int speed = rand_range(speedSeed/2, speedSeed*2);

        int axis = rand_range(0, 1);
        int r[2] = {0, 0};
        r[axis] = rand_range(0, 127);

        const Particle newParticle = {
            .x = rand_range(spawnX - 10, spawnX + 10),
            .y = rand_range(spawnY - 10, spawnY + 10),
            .rx = 0,
            .ry = 0,
            .rz = 0,
            .rdx = r[0],
            .rdy = rand_range(0, 127),
            .rdz = r[1],
            .vx = (vx * speed) >> 12,
            .vy = (vy * speed) >> 12,
            .lifetime = lifetime,
            .model = getParticleModel(&world->models, type),
        };

        const int start = world->nextFreeParticle;

        for (int j = 0; j < MAX_PARTICLES; j++) {
            const int idx = (start + j) % MAX_PARTICLES;

            if (world->particles[idx].lifetime == 0) {
                world->nextFreeParticle = (idx + 1) % MAX_PARTICLES;
                world->particles[idx] = newParticle;
                break;
            }
        }
    }
}

void detectBulletEnemyCollisions(World *world) {
    for (int b = 0; b < MAX_BULLETS; b++) {
        Bullet *bullet = &world->bullets[b];
        if (!bullet->alive) continue;

        const int s = isin(bullet->dir);
        const int c = icos(bullet->dir);

        const int tipX = bullet->x + ((c * BULLET_TIP_OFFSET) >> 12);
        const int tipY = bullet->y + ((s * BULLET_TIP_OFFSET) >> 12);

        for (int e = 0; e < MAX_ENEMIES; e++) {
            Enemy *enemy = &world->enemies[e];
            if (!enemy->alive) continue;

            if (isWithinRange(enemy->x, enemy->y, tipX, tipY, ENEMY_HIT_RADIUS)) {
                enemy->alive = 0;
                bullet->alive = 0;
                world->score += 25;
                emitParticles(world, SMALL_PARTICLE, 20, 25, 16, enemy->x, enemy->y);
                emitParticles(world, SMALL_PARTICLE, 10, 25, 32, enemy->x, enemy->y);
                break;
            }
        }
    }
}

void updateEnemies(World *world) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *enemy = &world->enemies[i];

        if (!enemy->alive) continue;

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
}

void updateParticles(World *world) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *particle = &world->particles[i];

        if (particle->lifetime == 0) continue;

        particle->x += particle->vx;
        particle->y += particle->vy;
        particle->rx += particle->rdx;
        particle->ry += particle->rdy;
        particle->rz += particle->rdz;
        particle->lifetime--;
    }
}

void togglePause(World *world, const ControllerResponse controller_response) {
    const uint16_t buttons = controller_response.buttons;
    const uint16_t last = world->lastButtons;

    const bool startDown = (buttons & BUTTON_START) != 0;
    const bool startWasDown = (last & BUTTON_START) != 0;

    if (startDown && !startWasDown) {
        if (world->state == GAMESTATE_PLAYING)
            world->state = GAMESTATE_PAUSED;
        else if (world->state == GAMESTATE_PAUSED)
            world->state = GAMESTATE_PLAYING;
    }

    world->lastButtons = buttons;
}
