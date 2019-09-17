// Blink an LED connected to pin 1.7

#include <ch554.h>
#include <debug.h>
#include <stdint.h>
#include <stdbool.h>

#include "gpio.h"

#define PORT_A_REG      0x90
#define PORT_C_REG      0xB0

#define I2C_ADDRESS     0x12        // 7-bit address

#define I2C_ADDRESS_READ ((I2C_ADDRESS << 1) | 0x01)
#define I2C_ADDRESS_WRITE (I2C_ADDRESS << 1)

#define LED_PIN         4
#define LED_PORT        3
SBIT(LED, PORT_C_REG, LED_PIN);

#define SCL_PIN         0
#define SCL_PORT        1
SBIT(SCL, PORT_A_REG, SCL_PIN);

#define SDA_PIN         1
#define SDA_PORT        1
SBIT(SDA, PORT_A_REG, SDA_PIN);

inline void wait_for_start() {
    __bit SCL_last;
    __bit SDA_last;

begin_wait:
    // wait for SCL and SDA both high
    while((SCL == 0) || (SDA == 0)) {}

    // wait for SCL high and SDA low
//    while ((SCL == 1) && (SDA == 1)) {}
//    if(SCL == 0)
//        goto begin_wait;
    do {
        SCL_last = SCL;
        SDA_last = SDA;
    } while((SCL_last == 1) && (SDA_last == 1));
    if(SCL_last == 0)
        goto begin_wait;

//    while ((SCL == 1) && (SDA == 0)) {}
//    if(SDA == 1)
//        goto begin_wait;
    do {
        SCL_last = SCL;
        SDA_last = SDA;
    } while((SCL_last == 1) && (SDA_last == 0));
    if(SDA_last == 1)
        goto begin_wait;

}

// Read a bit, but check for a restart
#define read_bit_2(dest)        \
        while (SCL == 0) {}     \
        LED = 1;                \
        if(SDA == 1) {          \
            dest |= SDA;        \
            while(SCL == 1) {   \
                if(SDA == 0) {    \
                    while(SCL == 1) {} \
                    goto restart;   \
                }               \
            }                   \
        } else {                \
            dest |= SDA;        \
            while(SCL == 1) {}  \
        }                       \
                    LED = 0;    


// Read a bit on the next rising edge
#define read_bit(dest)          \
        dest = (dest << 1);     \
        while (SCL == 0) {}     \
        dest |= SDA;            \
        while(SCL == 1) {}

#define write_bit(src)          \
        SDA = (src & 0x80);     \
        while(SCL == 0) {}      \
        src = src << 1;         \
        while(SCL == 1) {}

// Pull data line low, then wait for a clock rise and fall, the release
#define ack_bit()               \
        SDA = 0;                \
        while(SCL == 0) {}      \
        while (SCL == 1) {}     \
        SDA = 1;

#define REG_COUNT   8
static uint8_t regs[REG_COUNT];
static uint8_t reg;

void i2c_slave_init() {
    uint8_t i;

    reg = 0;
    for(i = 0; i < REG_COUNT; i++)
        regs[i] = i;

    gpio_pin_mode(SCL_PIN, SCL_PORT, GPIO_MODE_INPUT);
    gpio_pin_mode(SDA_PIN, SDA_PORT, GPIO_MODE_OPEN_DRAIN);

    gpio_pin_mode(LED_PIN, LED_PORT, GPIO_MODE_OUTPUT_PUSHPULL);
}

void i2c_slave_poll() {
    uint8_t address = 0;
    uint8_t val = 0;

    // TODO: Timeout during transaction
    wait_for_start();

restart:
    LED = 0;
    read_bit(address);
    read_bit(address);
    read_bit(address);
    read_bit(address);
    read_bit(address);
    read_bit(address);
    read_bit(address);
    read_bit(address);    // Read/Write bit

    if(address == I2C_ADDRESS_WRITE) {
        ack_bit();

        read_bit(reg);
        read_bit(reg);
        read_bit(reg);
        read_bit(reg);
        read_bit(reg);
        read_bit(reg);
        read_bit(reg);
        read_bit(reg);

        if(reg >= REG_COUNT) {
            //putchar(address);
            //putchar(reg);
            return;
        }

        ack_bit();

        // Look for a repeated start here
        read_bit_2(val);
        read_bit(val);
        read_bit(val);
        read_bit(val);
        read_bit(val);
        read_bit(val);
        read_bit(val);
        read_bit(val);

        ack_bit();

        regs[reg] = val;

        //putchar(address);
        //putchar(reg);
        //putchar(val);
    }
    else if(address == I2C_ADDRESS_READ) {
        //ack_bit();
        SDA = 0;
        while(SCL == 0) {}
        val = regs[reg];
        while (SCL == 1) {}
        SDA = 1;

        LED = 1;
        LED = 0;
        write_bit(val);
        write_bit(val);
        write_bit(val);
        write_bit(val);
        write_bit(val);
        write_bit(val);
        write_bit(val);
        write_bit(val);

        ack_bit();

        //putchar(address);
        //putchar(reg);
        //putchar(regs[reg]);
    }
    else {
        //putchar(address);
    }
}

void main() {
    CfgFsys();
    mInitSTDIO();   // debug output on P3.1

    i2c_slave_init();

    while (1) {
        i2c_slave_poll();

    	//mDelaymS(100);
        //LED = !LED;
    }
}
