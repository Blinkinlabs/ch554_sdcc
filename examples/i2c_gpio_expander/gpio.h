#pragma once

#define PORT_A_REG      0x90
#define PORT_C_REG      0xB0

typedef enum {
    GPIO_MODE_INPUT,                // High impedance, input only
    GPIO_MODE_OUTPUT_PUSHPULL,      // Push-pull output
    GPIO_MODE_OPEN_DRAIN,           // Open drain input/output
    GPIO_MODE_OPEN_DRAIN_PULLUP,    // Open drain input/output w/pullup
} gpio_mode_t;

void gpio_pin_mode(uint8_t pin, uint8_t port, gpio_mode_t mode);
