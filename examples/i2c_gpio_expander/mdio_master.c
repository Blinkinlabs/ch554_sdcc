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

static void write_byte(uint8_t byte) {
    uint8_t bit;

    for(bit = 0; bit < 8; bit++) {
        MDIO = (byte & 0x80);
        MDC = 1;
        byte = byte << 1;
        MDC = 0;
    }
}

static uint8_t read_byte() {
    uint8_t byte = 0;
    uint8_t bit;

    for(bit = 0; bit < 8; bit++) {
        byte = byte << 1 | MDIO;
        MDC = 1;
        MDC = 0;
    }

    return byte;
}

void mdio_master_init() {
    gpio_pin_mode(MDC_PIN, MDC_PORT, GPIO_MODE_INPUT);
    gpio_pin_mode(MDIO_PIN, MDIO_PORT, GPIO_MODE_OPEN_DRAIN);
}

void mdio_master_write(
        uint8_t addr,
        uint8_t reg,
        uint16_t data
        ) {
    uint8_t ctrl_byte_0 =
        (0x01 << 6)             // Start bits
        | (0x01 << 4)           // Write opcode
        | ((addr & 0x1E) >> 1); // Top 4 bits of PHY address
    uint8_t ctrl_byte_1 = 
        ((addr & 0x01) << 7)    // Bottom bit of PHY address
        | ((reg & 0x1F) << 2)   // Register to read
        | (0x02 << 0);          // Turnaround bits

    write_byte(0xFF);
    write_byte(0xFF);
    write_byte(0xFF);
    write_byte(0xFF);
    write_byte(ctrl_byte_0);
    write_byte(ctrl_byte_1);
    write_byte(*((__idata uint8_t *)data + 0));
    write_byte(*((__idata uint8_t *)data + 1));
}

void mdio_master_read(
        uint8_t addr,
        uint8_t reg,
        __idata uint16_t *data) {

    uint8_t ctrl_byte_0 =
        (0x01 << 6)             // Start bits
        | (0x02 << 4)           // Read opcode
        | ((addr & 0x1E) >> 1); // Top 4 bits of PHY address
    uint8_t ctrl_byte_1 = 
        ((addr & 0x01) << 7)    // Bottom bit of PHY address
        | ((reg & 0x1F) << 2)   // Register to read
        | (0x03 << 0);          // Turnaround bits

    write_byte(0xFF);
    write_byte(0xFF);
    write_byte(0xFF);
    write_byte(0xFF);
    write_byte(ctrl_byte_0);
    write_byte(ctrl_byte_1);
    *(((__idata uint8_t *)data)+0) = read_byte();
    *(((__idata uint8_t *)data)+1) = read_byte();
}
