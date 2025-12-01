#include "controller.h"
#include "world.h"
#include "stdlib.h"
#include "ps1/trig.h"

void worldInit(World *world) {
    world->player.x = 0;
    world->player.y = 0;
    world->player.rot = 0;
    world->player.dir = 0;
    world->player.model = malloc(sizeof *world->player.model);
    loadObjModel(world->player.model, playerShipObj);
}

static const int dir_to_angle[3][3] = {
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

    const int dir_angle = dir_to_angle[dx+1][dy+1];
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
