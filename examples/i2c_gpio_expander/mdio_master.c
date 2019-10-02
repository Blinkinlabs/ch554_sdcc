#include <ch554.h>
#include <debug.h>
#include <stdint.h>
#include <stdbool.h>

#include "mdio_master.h"
#include "gpio.h"

SBIT(MDC, PORT_C_REG, MDC_PIN);
SBIT(MDIO, PORT_C_REG, MDIO_PIN);

#define read_bit(dest)                      \
        dest = (dest << 1);                 \
        while (MDC == 0) {}                 \
        dest |= MDIO;                       \
        while(MDC == 1) {}

#define write_bit(src)                      \
        MDIO = (src & 0x80);                \
        while(MDC == 0) {}                  \
        src = src << 1;                     \
        while(MDC == 1) {}

static void write_byte(uint8_t val) {
    uint8_t bit;

    for(bit = 8; bit > 0; bit--) {
        MDIO = (val & 0x80);
        MDC = 1;
        val = val << 1;
        MDC = 0;
    }
}

static uint8_t read_byte() {
    uint8_t val = 0;
    uint8_t bit;

    for(bit = 8; bit > 0; bit--) {
        val = val << 1 | MDIO;
        MDC = 1;
        MDC = 1;    // NOP
        MDC = 0;
    }

    return val;
}

void mdio_master_init() {
    gpio_pin_mode(MDC_PIN, MDC_PORT, GPIO_MODE_OUTPUT_PUSHPULL);
    gpio_pin_mode(MDIO_PIN, MDIO_PORT, GPIO_MODE_OPEN_DRAIN_PULLUP);
}

void mdio_master_write(
        uint8_t addr,
        uint8_t reg,
        uint8_t data_h,
        uint8_t data_l
        ) {
    const uint8_t ctrl_byte_0 =
        (0x01 << 6)             // Start bits
        | (0x01 << 4)           // Write opcode
        | ((addr & 0x1E) >> 1); // Top 4 bits of PHY address
    const uint8_t ctrl_byte_1 = 
        ((addr & 0x01) << 7)    // Bottom bit of PHY address
        | ((reg & 0x1F) << 2)   // Register to read
        | (0x02 << 0);          // Turnaround bits

    write_byte(0xFF);
    write_byte(0xFF);
    write_byte(0xFF);
    write_byte(0xFF);
    write_byte(ctrl_byte_0);
    write_byte(ctrl_byte_1);
    write_byte(data_h);
    write_byte(data_l);
}

void mdio_master_read(
        uint8_t addr,
        uint8_t reg,
        __idata uint8_t *data_h,
        __idata uint8_t *data_l) {

    const uint8_t ctrl_byte_0 =
        (0x01 << 6)             // Start bits
        | (0x02 << 4)           // Read opcode
        | ((addr & 0x1E) >> 1); // Top 4 bits of PHY address
    const uint8_t ctrl_byte_1 = 
        ((addr & 0x01) << 7)    // Bottom bit of PHY address
        | ((reg & 0x1F) << 2)   // Register to read
        | (0x03 << 0);          // Turnaround bits

    write_byte(0xFF);
    write_byte(0xFF);
    write_byte(0xFF);
    write_byte(0xFF);
    write_byte(ctrl_byte_0);
    write_byte(ctrl_byte_1);
    *data_h = read_byte();
    *data_l = read_byte();
}
