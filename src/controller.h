#pragma once

#include <stdint.h>

#define DTR_DELAY 60

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

typedef enum {
    ADDR_CONTROLLER  = 0x01,
    ADDR_MEMORY_CARD = 0x81
} DeviceAddress;

typedef enum {
    CMD_INIT_PRESSURE   = '@', // Initialize DualShock pressure sensors (config)
    CMD_POLL            = 'B', // Read controller state
    CMD_CONFIG_MODE     = 'C', // Enter or exit configuration mode
    CMD_SET_ANALOG      = 'D', // Set analog mode/LED state (config)
    CMD_GET_ANALOG      = 'E', // Get analog mode/LED state (config)
    CMD_GET_MOTOR_INFO  = 'F', // Get information about a motor (config)
    CMD_GET_MOTOR_LIST  = 'G', // Get list of all motors (config)
    CMD_GET_MOTOR_STATE = 'H', // Get current state of vibration motors (config)
    CMD_GET_MODE        = 'L', // Get list of all supported modes (config)
    CMD_REQUEST_CONFIG  = 'M', // Configure poll request format (config)
    CMD_RESPONSE_CONFIG = 'O', // Configure poll response format (config)
    CMD_CARD_READ       = 'R', // Read 128-byte memory card sector
    CMD_CARD_GET_SIZE   = 'S', // Retrieve memory card size information
    CMD_CARD_WRITE      = 'W'  // Write 128-byte memory card sector
} DeviceCommand;

typedef struct {
    uint16_t buttons;
    uint16_t right_joystick;
    uint16_t left_joystick;
} ControllerResponse;

void initControllerBus(void);

ControllerResponse readController(int port);
