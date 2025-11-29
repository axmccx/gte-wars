#include <stdint.h>
#include "controller.h"
#include "world.h"
#include "stdlib.h"

void worldInit(World *world) {
    world->player.x = 0;
    world->player.y = 0;
    world->player.model = malloc(sizeof *world->player.model);

    // loadObjModel(monkeyObj);
    loadObjModel(world->player.model, icoSphere);
}

void updatePlayerPosition(World *world, uint16_t buttons) {
    int dx = ((buttons & BUTTON_RIGHT) != 0) - ((buttons & BUTTON_LEFT) != 0);
    int dy = ((buttons & BUTTON_DOWN) != 0) - ((buttons & BUTTON_UP) != 0);

    world->player.x += dx;
    world->player.y += dy;
}
