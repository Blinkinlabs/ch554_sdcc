#include <stdint.h>

#include "../../include/ch554.h"
#include "bitbang.h"

#define LED_PIN 5
SBIT(LED, 0x90, LED_PIN);


void bitbangSetup()
{
    // Configure pin 1.5 as GPIO output
    P1_DIR_PU &= 0x0C;
    P1_MOD_OC = P1_MOD_OC & ~(1<<LED_PIN);
    P1_DIR_PU = P1_DIR_PU |	(1<<LED_PIN);
}

void bitbangWs2812( uint8_t ledCount, __xdata uint8_t * ledData )
{
    ledCount;
    ledData;

    // Bitbang routine
    // Input parameters: (determined by compilation)
    // * byteCount should be allocated in dpl
    // * ledData should be allocated with name '_bitbangWs2812_PARAM_2'

    // Strategy:
    // * Keep the data memory pointer in DPTR
    // * Keep ledCount in r2
    // * Store bitCount in r3
    // * Store the current data variable in ACC

    //  TODO: What context needs to be pushed/popped?

    __asm

    mov r2, dpl             ; Load the LED count into r2

    mov dpl, _bitbangWs2812_PARM_2  ; Load the LED data start address into DPTR
    mov dph, (_bitbangWs2812_PARM_2 + 1)

    00001$:                 ; byte loop

// Red byte
        movx a,@dptr        ; Load the current LED data value into the accumulator (1)
        inc dptr            ; and advance the counter for the next LED data value (1)

        mov r3, #8          ; Set up the bit loop (2)
    00002$:                 ; red bit loop
        setb _LED           ; Begin bit cycle- set bit high (2)

        nop                 ; Tune this count by hand, want ~.4uS (1*2)
        nop

        rlc A               ; Shift the LED data value left to get the high bit (1)
        mov _LED, C         ; Set the output bit high if the current bit is high, (2)
                            ; otherwise set it low

        nop                 ; Tune this count by hand, want ~.4uS (1*2)
        nop

        clr _LED            ; final part of bit cycle, set bit low (2)

//        nop                 ; Tune this count by hand, want ~.45uS
//        nop

        djnz r3, 00002$     ; If there are more bits in this byte (2, ?)

// Green byte
        movx a,@dptr        ; Load the current LED data value into the accumulator (1)
        inc dptr            ; and advance the counter for the next LED data value (1)

        mov r3, #8          ; Set up the bit loop (2)
    00003$:                 ; red bit loop
        setb _LED           ; Begin bit cycle- set bit high (2)

        nop                 ; Tune this count by hand, want ~.4uS (1*2)
        nop

        rlc A               ; Shift the LED data value left to get the high bit (1)
        mov _LED, C         ; Set the output bit high if the current bit is high, (2)
                            ; otherwise set it low

        nop                 ; Tune this count by hand, want ~.4uS (1*2)
        nop

        clr _LED            ; final part of bit cycle, set bit low (2)

//        nop                 ; Tune this count by hand, want ~.45uS
//        nop

        djnz r3, 00003$     ; If there are more bits in this byte (2, ?)

// Blue byte
        movx a,@dptr        ; Load the current LED data value into the accumulator (1)
        inc dptr            ; and advance the counter for the next LED data value (1)

        mov r3, #8          ; Set up the bit loop (2)
    00004$:                 ; red bit loop
        setb _LED           ; Begin bit cycle- set bit high (2)

        nop                 ; Tune this count by hand, want ~.4uS (1*2)
        nop

        rlc A               ; Shift the LED data value left to get the high bit (1)
        mov _LED, C         ; Set the output bit high if the current bit is high, (2)
                            ; otherwise set it low

        nop                 ; Tune this count by hand, want ~.4uS (1*2)
        nop

        clr _LED            ; final part of bit cycle, set bit low (2)

//        nop                 ; Tune this count by hand, want ~.45uS
//        nop

        djnz r3, 00004$     ; If there are more bits in this byte (2, ?)


        djnz r2, 00001$     ; If there are more LEDs (2, ?)

    __endasm;
}
