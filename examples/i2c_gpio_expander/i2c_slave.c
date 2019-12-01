#include <ch554.h>
#include <debug.h>
#include <stdint.h>
#include <stdbool.h>

#include "i2c_slave.h"
#include "gpio.h"
#include "pins.h"

SBIT(SCL, PORT_C_REG, SCL_PIN);
SBIT(SDA, PORT_C_REG, SDA_PIN);

SBIT(DBGA, PORT_A_REG, DEBUG_A_PIN);
SBIT(DBGB, PORT_A_REG, DEBUG_B_PIN);

//static uint8_t regs[I2C_SLAVE_REG_COUNT];


uint8_t i2c_slave_reg;     //!< Current state machine register (must always be valid)
uint8_t i2c_slave_val;     //!< Current state machine value (valid after a write)

void i2c_slave_init() {
//    uint8_t i;

    i2c_slave_reg = 0;
//    for(i = 0; i < I2C_SLAVE_REG_COUNT; i++)
//        regs[i] = i;

    gpio_pin_mode(SCL_PIN, SCL_PORT, GPIO_MODE_INPUT);
    gpio_pin_mode(SDA_PIN, SDA_PORT, GPIO_MODE_OPEN_DRAIN);

    gpio_pin_mode(DEBUG_A_PIN, DEBUG_A_PORT, GPIO_MODE_OUTPUT_PUSHPULL);
    gpio_pin_mode(DEBUG_B_PIN, DEBUG_B_PORT, GPIO_MODE_OUTPUT_PUSHPULL);
}

inline void wait_for_start() {
    __bit SCL_last;
    __bit SDA_last;

begin_wait:
    // wait for SCL and SDA both high
    while((SCL == 0) || (SDA == 0)) {}

    // wait for transition to SCL high and SDA low
    do {
        SCL_last = SCL;
        SDA_last = SDA;
    } while((SCL_last == 1) && (SDA_last == 1));
    if(SCL_last == 0)
        goto begin_wait;

    // wait for transition to SCL low and SDA low
    do {
        SCL_last = SCL;
        SDA_last = SDA;
    } while((SCL_last == 1) && (SDA_last == 0));
    if(SDA_last == 1)
        goto begin_wait;

}

// Read a bit, but check for a read_address
#define read_bit_2(dest)                    \
        while (SCL == 0) {}                 \
        if(SDA == 1) {                      \
            dest |= SDA;                    \
            while(SCL == 1) {               \
                if(SDA == 0) {              \
                    while(SCL == 1) {}      \
                    goto read_address;      \
                }                           \
            }                               \
        } else {                            \
            dest |= SDA;                    \
            while(SCL == 1) {}              \
        }

// Read a bit on the next rising edge
#define read_bit(dest)                      \
        dest = (dest << 1);                 \
        while (SCL == 0) {}                 \
        dest |= SDA;                        \
        while(SCL == 1) {}

#define write_bit(src)                      \
        SDA = (src & 0x80);                 \
        while(SCL == 0) {}                  \
        src = src << 1;                     \
        while(SCL == 1) {}

// Pull data line low, wait for a clock rise and fall, then release
#define send_ack()                          \
        SDA = 0;                            \
        while(SCL == 0) {}                  \
        while (SCL == 1) {}                 \
        SDA = 1;

#define send_ack_and_load(data)             \
        SDA = 0;                            \
        while(SCL == 0) {}                  \
        data = *(regs_ptr[i2c_slave_reg]);  \
        while (SCL == 1) {}                 \
        SDA = 1;

#define send_ack_and_store(data)            \
        SDA = 0;                            \
        while(SCL == 0) {}                  \
        /**(regs_ptr[i2c_slave_reg]) = data;*/  \
        while (SCL == 1) {}                 \
        SDA = 1;

// release data line, wait for a clock rise and fall, then release
#define send_nack()                         \
        SDA = 1;                            \
        while(SCL == 0) {}                  \
        while (SCL == 1) {}                 \
        SDA = 1;


i2c_slave_transaction_t i2c_slave_poll() {

    uint8_t address = 0;    // Address that 
    uint8_t new_reg = 0;    // 
    i2c_slave_val = 0;

    // TODO: Timeout during transaction

    wait_for_start();
read_address:
    DBGA = 1;
    DBGB = 0;
    read_bit(address);  // 7 address bits
    read_bit(address);
    read_bit(address);
    read_bit(address);
    read_bit(address);
    read_bit(address);
    read_bit(address);
    read_bit(address);  // Read/Write bit

    if(address == I2C_ADDRESS_WRITE) {
        DBGA = 0;
        DBGB = 1;
        send_ack();

        read_bit(new_reg);
        read_bit(new_reg);
        read_bit(new_reg);
        read_bit(new_reg);
        read_bit(new_reg);
        read_bit(new_reg);
        read_bit(new_reg);
        read_bit(new_reg);

        if(new_reg >= I2C_SLAVE_REG_COUNT) {
            send_nack();
            DBGB = 0;
            DBGA = 0;
            return I2C_SLAVE_INVALID_REG;
        }
        i2c_slave_reg = new_reg;
        send_ack();

        // Look for a repeated start here
        read_bit_2(i2c_slave_val);
        read_bit(i2c_slave_val);
        read_bit(i2c_slave_val);
        read_bit(i2c_slave_val);
        read_bit(i2c_slave_val);
        read_bit(i2c_slave_val);
        read_bit(i2c_slave_val);
        read_bit(i2c_slave_val);

        send_ack_and_store(i2c_slave_val);
        DBGA = 0;
        DBGB = 0;
        return I2C_SLAVE_WRITE;
    }
    else if(address == I2C_ADDRESS_READ) {
        DBGA = 1;
        DBGB = 1;
        send_ack_and_load(i2c_slave_val);

        write_bit(i2c_slave_val);
        write_bit(i2c_slave_val);
        write_bit(i2c_slave_val);
        write_bit(i2c_slave_val);
        write_bit(i2c_slave_val);
        write_bit(i2c_slave_val);
        write_bit(i2c_slave_val);
        write_bit(i2c_slave_val);

        send_ack();
        DBGA = 0;
        DBGB = 0;
        return I2C_SLAVE_READ;
    }
    else {
        DBGA = 0;
        DBGB = 0;
        return I2C_SLAVE_WRONG_ADDRESS;
    }
}
