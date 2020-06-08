// Enable both PWM peripherals, and connect them to their GPIO outputs

#include <ch554.h>
#include <debug.h>
#include <pwm.h>


void main() {
    int i = 0;
    SetPWMClk(4);
    ForceClearPWMFIFO();
    CancelClearPWMFIFO();

    // Uncomment these lines to use the alternate pin mapping
    PWM1PinAlter();
    PWM2PinAlter();

    PWM1OutEnable();
    PWM2OutEnable();

    PWM1OutPolarHighAct();
    PWM2OutPolarLowAct();

    SetPWM1Dat(0x10);
    SetPWM2Dat(0x40);

    while(1) {
        for (i = 0 ;i < 255; i++){
        SetPWM1Dat(i);
        SetPWM2Dat(i);
        mDelaymS(1);
        }
        for (i = 255 ;i > 0; i--){
        SetPWM1Dat(i);
        SetPWM2Dat(i);
        mDelaymS(1);
        }

        mDelaymS(1);


    }
}
