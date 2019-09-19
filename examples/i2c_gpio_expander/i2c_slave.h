#pragma once

#define I2C_ADDRESS     0x12        // 7-bit address

#define I2C_ADDRESS_READ ((I2C_ADDRESS << 1) | 0x01)
#define I2C_ADDRESS_WRITE (I2C_ADDRESS << 1)

void i2c_slave_init();
void i2c_slave_poll();
