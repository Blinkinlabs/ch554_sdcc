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

void mdio_master_init() {
    gpio_pin_mode(MDC_PIN, MDC_PORT, GPIO_MODE_INPUT);
    gpio_pin_mode(MDIO_PIN, MDIO_PORT, GPIO_MODE_OPEN_DRAIN);
}

mdio_master_err_t mdio_master_write(
        uint8_t addr,
        uint8_t reg,
        uint16_t data
        ) {
    // write 32 bits of start clock
    // write read/write code
    // write 5 bits of addr
    // write 5 bits of reg
    // write 8 bits of data high
    // write 8 bits of data low
    return MDIO_MASTER_ERROR;
}
mdio_master_err_t mdio_master_read(
        uint8_t addr,
        uint8_t reg,
        __idata uint16_t *data) {
    // write 32 bits of start clock
    // write read/write code
    // write 5 bits of addr
    // write 5 bits of reg
    // read 8 bits of data high
    // read 8 bits of data low
    return MDIO_MASTER_ERROR;
}
