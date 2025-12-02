#pragma once

#include "ps1/gpu.h"

void initOrderingChain(DMAChain *chain, int bufferX, int bufferY);

void buildRenderPackets(DMAChain *chain, ObjModel *modelToRender);
