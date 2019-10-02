#pragma once

#define I2C_ADDRESS     0x12        // 7-bit address

#define I2C_ADDRESS_READ ((I2C_ADDRESS << 1) | 0x01)
#define I2C_ADDRESS_WRITE (I2C_ADDRESS << 1)

typedef enum {
    I2C_SLAVE_READ,
    I2C_SLAVE_WRITE,
    I2C_SLAVE_WRONG_ADDRESS,
    I2C_SLAVE_INVALID_REG,
} i2c_slave_transaction_t;

extern uint8_t i2c_slave_reg;
extern uint8_t i2c_slave_val;

extern __idata uint8_t *regs_ptr[I2C_SLAVE_REG_COUNT];

void i2c_slave_init();
i2c_slave_transaction_t i2c_slave_poll();
