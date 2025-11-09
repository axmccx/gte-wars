#pragma once

void initControllerBus(void);

typedef enum {
    BUTTON_SELECT   = 1 << 0,
    BUTTON_L3       = 1 << 1,
    BUTTON_R3       = 1 << 2,
    BUTTON_START    = 1 << 3,
    BUTTON_UP       = 1 << 4,
    BUTTON_RIGHT    = 1 << 5,
    BUTTON_DOWN     = 1 << 6,
    BUTTON_LEFT     = 1 << 7,
    BUTTON_L2       = 1 << 8,
    BUTTON_R2       = 1 << 9,
    BUTTON_L1       = 1 << 10,
    BUTTON_R1       = 1 << 11,
    BUTTON_TRIANGLE = 1 << 12,
    BUTTON_CIRCLE   = 1 << 13,
    BUTTON_CROSS    = 1 << 14,
    BUTTON_SQUARE   = 1 << 15
} Buttons;

uint16_t readControllerButtons(int port);
