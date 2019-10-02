#pragma once

#include "pins.h"

void mdio_master_init();

void mdio_master_write(uint8_t addr, uint8_t reg, uint16_t data);
void mdio_master_read(uint8_t addr, uint8_t reg, __idata uint16_t *data);

