#pragma once

#include "ps1/gpu.h"

#define FONT_FIRST_TABLE_CHAR '!'
#define FONT_SPACE_WIDTH 4
#define FONT_TAB_WIDTH 32
#define FONT_LINE_HEIGHT 10

#define FONT_WIDTH 96
#define FONT_HEIGHT 56
#define FONT_COLOR_DEPTH GP0_COLOR_4BPP

extern const uint8_t fontTexture[], fontPalette[];

void printString(DMAChain *chain, const TextureInfo *font, int x, int y, const char *str);
