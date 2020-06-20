// Jump to the bootloader when pin 1.7 (UART0_RX) is held low

#include <ch554.h>
#include <debug.h>
#include <bootloader.h>

#define ENABLE_IAP_PIN 6
SBIT(EnableIAP, 0x90, ENABLE_IAP_PIN);

#define LED_PIN 7
SBIT(LED, 0x90, LED_PIN);


void main() {
    while(1) {
        LED = !LED;                        //P17 flashes

        mDelaymS(50);
        if(EnableIAP == 0)                 //P16 pin detected low level jump
            break;
    }

    EA = 0;                                //Close the total interrupt, must add
    mDelaymS(100);

    bootloader();
    while(1); 
}
