#include "controller.h"
#include "world.h"
#include "stdlib.h"
#include "ps1/trig.h"
#include "rng.h"

void worldInit(World *world) {
    world->frameCount = 0;
    world->nextFreeBullet = 0;
    world->nextFreeEnemy = 0;
    world->camera.x = 0;
    world->camera.y = 0;
    world->player.x = 0;
    world->player.y = 0;
    world->player.rot = 0;
    world->player.dir = 0;

    for (int i = 0; i < MAX_BULLETS; i++) {
        world->bullets[i].alive = 0;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        world->enemies[i].alive = 0;
    }

    world->models.player = malloc(sizeof *world->models.player);
    loadObjModel(world->models.player, playerShipObj);

    world->models.bullet = malloc(sizeof *world->models.bullet);
    loadObjModel(world->models.bullet, bulletObj);

    world->models.enemy = malloc(sizeof *world->models.enemy);
    loadObjModel(world->models.enemy, octahedronObj);
}

static const int dpad_to_angle[3][3] = {
    { 3584, 3072, 2560 },
    { 0, -1, 2048 },
    { 512, 1024, 1536 },
};

void updatePlayer(World *world, const ControllerResponse controller_response) {
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
        world->player.x += (stick_x * MAX_SPEED) / 128;
        world->player.y += (stick_y * MAX_SPEED) / 128;
        world->player.dir = atan2(stick_y, stick_x);
    } else {
        world->player.x += dx * MAX_SPEED;
        world->player.y += dy * MAX_SPEED;
        world->player.rot += 32;

        const int dir_angle = dpad_to_angle[dx+1][dy+1];
        if (dir_angle >= 0) {
            world->player.dir = dir_angle;
        }
    }

    clamp_axis(&world->player.x, PLAYFIELD_HALF_WIDTH - 100);
    clamp_axis(&world->player.y, PLAYFIELD_HALF_HEIGHT - 100);
}

void spawnBullets(World *world, const ControllerResponse controller_response) {
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

void spawnEnemies(World *world) {
    if (world->frameCount % 50 != 0) return;

    int vx = rand_range(-128, 128);
    int vy = rand_range(-128, 128);
    normalize_direction(&vx, &vy);

    const Enemy newEnemy = {
        .x = rand_range(-(PLAYFIELD_HALF_WIDTH - 500), PLAYFIELD_HALF_WIDTH - 500),
        .y = rand_range(-(PLAYFIELD_HALF_HEIGHT - 500), PLAYFIELD_HALF_HEIGHT - 500),
        .rot = 0,
        .vx = vx,
        .vy = vy,
        .alive = 1,
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

void updateEnemies(World *world) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *enemy = &world->enemies[i];

        if (!enemy->alive) continue;

        enemy->x += (enemy->vx * ENEMY_SPEED) >> 12;
        enemy->y += (enemy->vy * ENEMY_SPEED) >> 12;
        bounce_axis(&enemy->x, &enemy->vx, PLAYFIELD_HALF_WIDTH  - 64);
        bounce_axis(&enemy->y, &enemy->vy, PLAYFIELD_HALF_HEIGHT - 64);
        enemy->rot += 32;
    }
}
