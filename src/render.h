#pragma once

#include "ps1/gpu.h"

typedef enum {
    COLOR_RED = 0x0000FF,
    COLOR_YELLOW= 0x00FFFF,
    COLOR_BLUE = 0xFF0000,
    COLOR_MAGENTA = 0xFF00FF,
    COLOR_CYAN = 0xFFFF00
} Color;

void initOrderingChain(DMAChain *chain, int bufferX, int bufferY);

void buildRenderPackets(DMAChain *chain, ObjModel *modelToRender, uint32_t color);
