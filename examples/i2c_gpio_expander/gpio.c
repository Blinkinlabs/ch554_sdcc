#include <ch554.h>
#include <stdint.h>

#include "gpio.h"

void gpio_pin_mode(uint8_t pin, uint8_t port, gpio_mode_t mode) {
    // Configure pin 3.4 as GPIO output
    //P3_DIR_PU &= 0x0C;

    const uint8_t val = (1<<pin);

    switch(mode) {
        case GPIO_MODE_INPUT:
            switch(port) {
                case 1: P1_MOD_OC &= ~val; P1_DIR_PU &= ~val; break;
                case 3: P3_MOD_OC &= ~val; P3_DIR_PU &= ~val; break;
                default: break;
            }
            break;
        case GPIO_MODE_OUTPUT_PUSHPULL:
            switch(port) {
                case 1: P1_MOD_OC &= ~val; P1_DIR_PU |= val; break;
                case 3: P3_MOD_OC &= ~val; P3_DIR_PU |= val; break;
                default: break;
            }
            break;
        case GPIO_MODE_OPEN_DRAIN:
            switch(port) {
                case 1: P1_MOD_OC |= val; P1_DIR_PU &= ~val; break;
                case 3: P3_MOD_OC |= val; P3_DIR_PU &= ~val; break;
                default: break;
            }
            break;
        case GPIO_MODE_OPEN_DRAIN_PULLUP:
            switch(port) {
                case 1: P1_MOD_OC |= val; P1_DIR_PU |= val; break;
                case 3: P3_MOD_OC |= val; P3_DIR_PU |= val; break;
                default: break;
            }
            break;
    }
}
