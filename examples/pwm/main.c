// Enable both PWM peripherals, and connect them to their GPIO outputs

#include <ch554.h>
#include <pwm.h>

void main() {
    SetPWMClk(4);
    ForceClearPWMFIFO();
    CancelClearPWMFIFO();

    // Uncomment these lines to use the alternate pin mapping
//    PWM1PinAlter();
//    PWM2PinAlter();

    PWM1OutEnable();
    PWM2OutEnable();

    PWM1OutPolarHighAct();
    PWM2OutPolarLowAct();

    SetPWM1Dat(0x10);
    SetPWM2Dat(0x40);

    while(1) {}
}
