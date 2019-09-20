#pragma once

#define MDC_PIN         3
#define MDC_PORT        3

#define MDIO_PIN         4
#define MDIO_PORT        3

typedef enum {
    MDIO_MASTER_OK,
    MDIO_MASTER_ERROR,
} mdio_master_err_t;

void mdio_master_init();

mdio_master_err_t mdio_master_write(uint8_t addr, uint8_t reg, uint16_t data);
mdio_master_err_t mdio_master_read(uint8_t addr, uint8_t reg, __idata uint16_t *data);

