#include <stdint.h>
#include "controller.h"
#include "world.h"

void worldInit(World *world) {
    world->player.xPos = 0;
    world->player.yPos = 0;
}

void updatePlayerPosition(World *world, uint16_t buttons) {
    int dx = ((buttons & BUTTON_RIGHT) != 0) - ((buttons & BUTTON_LEFT) != 0);
    int dy = ((buttons & BUTTON_DOWN) != 0) - ((buttons & BUTTON_UP) != 0);

    world->player.xPos += dx;
    world->player.yPos += dy;
}
