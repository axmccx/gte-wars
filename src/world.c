#include "controller.h"
#include "world.h"
#include "stdlib.h"

void worldInit(World *world) {
    world->player.x = 0;
    world->player.y = 0;
    world->player.model = malloc(sizeof *world->player.model);
    loadObjModel(world->player.model, playerShipObj);
}

void updatePlayer(World *world, const ControllerResponse controller_response) {
    const bool right = (controller_response.buttons & BUTTON_RIGHT) != 0;
    const bool left = (controller_response.buttons & BUTTON_LEFT) != 0;
    const bool down = (controller_response.buttons & BUTTON_DOWN) != 0;
    const bool up = (controller_response.buttons & BUTTON_UP) != 0;

    const int dx = right ? MAX_SPEED : (left ? -MAX_SPEED : 0);
    const int dy = down ? MAX_SPEED : (up ? -MAX_SPEED: 0);

    world->player.x += dx;
    world->player.y += dy;
}
