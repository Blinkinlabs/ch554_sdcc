// Use the PWM interrupt to blink an LED connected to pin 1.7

#include <ch554.h>
#include <pwm.h>
#include <debug.h>

#define LED1_PIN 7
SBIT(LED1, 0x90, LED1_PIN);

void PWM_ISR(void) __interrupt (INT_NO_PWMX) {
    // Clear the interrupt flag
    PWM_CTRL |= bPWM_IF_END;

    // Toggle the LED
    LED1 = !LED1;
}

void main() {
    // Configure pin 1.7 as GPIO output
    P1_DIR_PU &= 0x0C;
    P1_MOD_OC = P1_MOD_OC & ~(1<<LED1_PIN);
    P1_DIR_PU = P1_DIR_PU |	(1<<LED1_PIN);
    LED1 = 1;

    SetPWMClk(254);
    ForceClearPWMFIFO();
    CancelClearPWMFIFO();

    PWM1OutEnable();
    PWM2OutEnable();

    SetPWM1Dat(0x10);
    SetPWM2Dat(0x40);

    PWMInterruptEnable();
    EA = 1;

    while(1) {
    }
}
