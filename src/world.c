#include "controller.h"
#include "world.h"
#include "stdlib.h"
#include "ps1/trig.h"

void worldInit(World *world) {
    world->frameCount = 0;
    world->nextFreeBullet = 0;
    world->player.x = 0;
    world->player.y = 0;
    world->player.rot = 0;
    world->player.dir = 0;

    for (int i = 0; i < MAX_BULLETS; i++) {
        world->bullets[i].lifetime = 0,
        world->bullets[i].alive = 0;
    }

    world->models.player = malloc(sizeof *world->models.player);
    loadObjModel(world->models.player, playerShipObj);

    world->models.bullet = malloc(sizeof *world->models.bullet);
    loadObjModel(world->models.bullet, bulletObj);
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

    world->player.x += dx * MAX_SPEED;
    world->player.y += dy * MAX_SPEED;
    world->player.rot += 32;

    const int dir_angle = dpad_to_angle[dx+1][dy+1];
    if (dir_angle >= 0) {
        world->player.dir = dir_angle;
    }

    uint8_t stick_x_raw = controller_response.left_joystick & 0xFF;
    uint8_t stick_y_raw = (controller_response.left_joystick >> 8) & 0xFF;

    int8_t stick_x = (int8_t)(stick_x_raw - 128);
    int8_t stick_y = (int8_t)(stick_y_raw - 128);

    const int8_t DEADZONE = 64;

    int abs_x = (stick_x < 0) ? -stick_x : stick_x;
    int abs_y = (stick_y < 0) ? -stick_y : stick_y;
    int magnitude = abs_x + abs_y;

    if (magnitude > DEADZONE) {
        int dx = (stick_x * MAX_SPEED) / 128;
        int dy = (stick_y * MAX_SPEED) / 128;

        world->player.x += dx;
        world->player.y += dy;
        world->player.dir = atan2(stick_y, stick_x);
    }
}

void spawnBullets(World *world, const ControllerResponse controller_response) {
    uint8_t stick_x_raw = controller_response.right_joystick & 0xFF;
    uint8_t stick_y_raw = (controller_response.right_joystick >> 8) & 0xFF;

    int8_t stick_x = (int8_t)(stick_x_raw - 128);
    int8_t stick_y = (int8_t)(stick_y_raw - 128);

    const int8_t DEADZONE = 64;

    int abs_x = (stick_x < 0) ? -stick_x : stick_x;
    int abs_y = (stick_y < 0) ? -stick_y : stick_y;
    int magnitude = abs_x + abs_y;

    if (magnitude > DEADZONE && world->frameCount % 5 == 0) {
        int vx = (stick_x * (1 << 12)) / magnitude;
        int vy = (stick_y * (1 << 12)) / magnitude;

        const int BULLET_OFFSET = 80;

        int spawn_x = world->player.x + ((vx * BULLET_OFFSET) >> 12);
        int spawn_y = world->player.y + ((vy * BULLET_OFFSET) >> 12);

        const Bullet newBullet = {
            .x = spawn_x,
            .y = spawn_y,
            .vx = vx,
            .vy = vy,
            .dir = atan2(stick_y, stick_x),
            .lifetime = 0,
            .alive = 1,
        };

        int start = world->nextFreeBullet;

        for (int i = 0; i < MAX_BULLETS; i++) {
            int idx = (start + i) % MAX_BULLETS;

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

        if (bullet->lifetime > 100) {
            bullet->alive = 0;
        }

        if (bullet->alive) {
            int dx = (bullet->vx * BULLET_SPEED) >> 12;
            int dy = (bullet->vy * BULLET_SPEED) >> 12;

            bullet->x += dx;
            bullet->y += dy;
            bullet->lifetime++;
        }
    }
}
