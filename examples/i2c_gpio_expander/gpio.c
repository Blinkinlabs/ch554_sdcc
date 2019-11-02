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

void gpio_pin_write(uint8_t pin, uint8_t port, __bit value) {
    const uint8_t mod = (1 << pin);
    switch(port) {
        case 1:
            if(value)
                P1 |= mod;
            else
                P1 &= ~mod;
            break;
        case 3:
            if(value)
                P3 |= mod;
            else
                P3 &= ~mod;
            break;
        default:
            break;
    }
}

__bit gpio_pin_read(uint8_t pin, uint8_t port) {
    const uint8_t mod = (1 << pin);
    switch(port) {
        case 1:
            return (P1 & mod);
            break;
        case 3:
            return (P3 & mod);
            break;
        default:
            break;
    }

    return 0;
}
