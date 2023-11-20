// Blink an LED connected to pin 1.4

#include <ch554.h>
#include <debug.h>

// 0x90-0x97 are P1 bit addresses, 0xB0-0xB7 are P3 bit addresses
#define LED_ADDR 0x94
__sbit __at (LED_ADDR) LED;

// set GPIO to push-pull mode
inline void pushPull(uint8_t pinaddr)
{
    if (pinaddr >= 0xB0 && pinaddr < 0xB8)
        P3_MOD_OC &= ~(1 << (pinaddr - 0xB0));
    else if (pinaddr >= 0x90 && pinaddr < 0x98)
        P1_MOD_OC &= ~(1 << (pinaddr - 0x90));
}

void main() {
    CfgFsys();

    pushPull(LED_ADDR);

    while (1) {
        mDelaymS(250);
        LED = !LED;
    }
}
