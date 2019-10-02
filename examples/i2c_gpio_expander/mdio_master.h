#pragma once

#include "pins.h"

void mdio_master_init();

void mdio_master_write(uint8_t addr, uint8_t reg, uint8_t data_h, uint8_t data_l);
void mdio_master_read(uint8_t addr, uint8_t reg, __idata uint8_t *data_h, __idata uint8_t *data_l);

