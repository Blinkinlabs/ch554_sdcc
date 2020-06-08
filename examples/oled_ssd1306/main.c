// Blink an LED connected to pin 1.7

#include <ch554.h>
#include "i2c.h"
#include <debug.h>
#include "ssd1306.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define I2C_DEFAULT

#define LED_PIN 7
SBIT(LED, 0x90, LED_PIN);

void main() {
    CfgFsys();
    ssd1306Init();

    // Configure pin 1.7 as GPIO output
    P1_DIR_PU &= 0x0C;
    P1_MOD_OC = P1_MOD_OC & ~(1<<LED_PIN);
    P1_DIR_PU = P1_DIR_PU |	(1<<LED_PIN);

    while (1) {
    	mDelaymS(100);
        LED = !LED;
        ssd1306Init();

    }
}
