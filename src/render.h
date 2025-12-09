#pragma once

#include "world.h"
#include "ps1/gpu.h"

typedef enum {
    COLOR_RED = 0x0000FF,
    COLOR_YELLOW= 0x00FFFF,
    COLOR_BLUE = 0xFF0000,
    COLOR_GREEN = 0x00FF00,
    COLOR_MAGENTA = 0xFF00FF,
    COLOR_CYAN = 0xFFFF00,
    COLOR_WHITE = 0xFFFFFF
} Color;

void initOrderingChain(DMAChain *chain, int bufferX, int bufferY);

void buildPlayfieldBorder(DMAChain *chain, Camera camera);

void buildRenderPackets(DMAChain *chain, const ObjModel *modelToRender);
