#pragma once

typedef struct {
    int xPos, yPos;
} Player;

typedef struct {
    Player player;
} World;

void worldInit(World *world);
void updatePlayerPosition(World *world, uint16_t buttons);
