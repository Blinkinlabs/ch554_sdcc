#include <ch554.h>
#include <debug.h>

#define LED_PIN 0 // P3.0 on WeAct CH55xx board
SBIT(LED, 0xB0, LED_PIN); // 0xB0 - P3 port register

volatile uint16_t counter;

void timer0_interrupt(void) __interrupt(INT_NO_TMR0){
    TL0 = 0xe0;
    TH0 = 0xff;
    // once timer triggers this interrupt, we'll set the TL and TH values, so 
    // it will start to increment from there
    // The formula used to calculate TL and TH values in regards to the 16 bit `counter`:
    // Timer0/1 will _always_ increment TL/TH every 12th clock cycle:
    // max_val_of_counter+1 * ((0xffff-0xTHTL) * (1/(FREQ_SYS/12)) = seconds for the counter to overflow
    // our `counter` is 16 bit, so the max value is 0xffff:
    // (0xffff+1)*(0xffff-0xffe0)*(1/(24000000/12)) = 1.015 seconds

    // we will update this `counter` on every interrupt
    // and once it overflows, we'll toggle LED pin
    counter++;
    if(!counter)
        LED = !LED;
}

void main() {
    CfgFsys();

    P3_DIR_PU |= (1<<LED_PIN); // LED_PIN set to output

    EA = 1; // global interrupt enable
    ET0 = 1; // enable timer0 interrupt
    TMOD |= bT0_M0; // timing, not counting, mode 1 (counts to 0xFFFF - TLTH)
    TR0 = 1; // start the timer

    while(1);
}