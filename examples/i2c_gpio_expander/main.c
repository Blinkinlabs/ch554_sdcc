// Blink an LED connected to pin 1.7

#include <ch554.h>
#include <debug.h>
#include <stdint.h>
#include <stdbool.h>

#include "gpio.h"

#define LED_PIN         4
#define LED_PORT        3
SBIT(LED, PORT_C_REG, LED_PIN);


void main() {
    CfgFsys();
//    mInitSTDIO();   // debug output on P3.1

    gpio_pin_mode(LED_PIN, LED_PORT, GPIO_MODE_OUTPUT_PUSHPULL);

    i2c_slave_init();

    while (1) {
        i2c_slave_poll();

    	//mDelaymS(100);
        //LED = !LED;
    }
}
