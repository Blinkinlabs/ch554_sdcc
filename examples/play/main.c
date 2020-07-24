// Blink an LED 

// ED CH552 LED = P1.4, CH551 LED = P3.0
#include <8052.h>
#include <ch554.h>
#include <debug.h>

#define LED_PIN 0
//SBIT(LED, 0x90, LED_PIN);
SBIT(LED, 0xB0, LED_PIN);

void mSspin(uint16_t);
static void msSpin_asm()                // dummy C function wrapper for asm
{
#define FREQ_MHZ (FREQ_SYS / 1000000)
#define CYCLES_PER_MS (166 * (FREQ_MHZ))
    __asm
    ; mSspin = busy loop for n = DP milliseconds
    _mSspin::
    inc DPH
    1$:
    mov r7, #FREQ_MHZ
    2$:                                 ; 1us loop
    mov r6, #166
    ;mov r6, #(CYCLES_PER_MS & 0xFF) 
    ;mov r7, #((CYCLES_PER_MS >> 8) +1)
    3$:
    ; nop + djnz will total 6 cycles whether even or odd aligned - 166 * 6 = 996c
    nop
    djnz r6, 3$
    djnz r7, 2$                      ; repeat FREQ_MHZ times
    djnz DPL, 1$
    djnz DPH, 1$
    __endasm;
}

/*
inline void usSpin(uint8_t us)
{
    DPL = us;
    __asm
    push ar7
    1$:
    mov r7, 2
    2$:                                 ; 6 cycle loop * 2
    nop
    djnz r7, 2$
    djnz DPL, 1$                        ; + 6 cycles
    pop ar7
    __endasm;
}
*/

// try as macro instead?
inline void setpin(uint8_t pin)
{
    if (pin >= 30) P3 |= (1 << (pin % 10));
}

// define PIN_P0_0 through PIN_P3_7 for convenience
inline void pushpull(uint8_t pinaddr)
{
    if (pinaddr == 0xB0) P3_MOD_OC = 0xFE; 
}

#define LEDBIT 0xB0
void main() {
    //CfgFsys();

    uint8_t power = 42;
    // Configure pin 1.7 as GPIO output
    //P1_DIR_PU &= 0x0C;
    //P1_DIR_PU |= (1<<LED_PIN);
    //P1_MOD_OC &= ~(1<<LED_PIN);
    //P3_DIR_PU |= (1<<LED_PIN);
    //P3_MOD_OC &= ~(1<<LED_PIN);
    //pin_input(&LED);     // can't take address of sbit
    pushpull(LEDBIT);

    while (1) {
    	//mDelaymS(250);
        mSspin(125);
        //usSpin(16);
	__asm
    ;cpl P3.0                            ; same as cpl _P3_0 when P3_0 defined
    cpl LEDBIT
	__endasm;
    //setpin(30);
    //P3_0 = 1;
    //    LED = !LED;
    }
}
