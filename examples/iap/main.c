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
        LED = !LED;                        //P17闪烁

        mDelaymS(50);
        if(EnableIAP == 0)                 //P16引脚检测到低电平跳转
            break;
    }

    EA = 0;                                //关闭总中断，必加
    mDelaymS(100);

    bootloader();
    while(1); 
}
