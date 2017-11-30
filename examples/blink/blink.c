// Blink an LED connected to pin 1.6

#include "../../public/ch554.h"

#define LED1_PIN 7
SBIT(LED1, 0x90, LED1_PIN);

void main() {
    int i;

    // Configure pin 1.6 as GPIO output
    P1_DIR_PU &= 0x0C;
    P1_MOD_OC = P1_MOD_OC & ~(1<<LED1_PIN);
    P1_DIR_PU = P1_DIR_PU |	(1<<LED1_PIN);

    while (1) {
        for(i = 0; i < 30000; i++)
            LED1 = 0;

        for(i = 0; i < 30000; i++)
            LED1 = 1;

    }
}
