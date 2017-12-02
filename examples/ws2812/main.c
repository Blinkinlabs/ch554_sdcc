// Blink an LED connected to pin 1.7

#include <stdint.h>

#include <ch554.h>
#include <debug.h>
#include "bitbang.h"

#define LED_COUNT (255)
__xdata uint8_t led_data[LED_COUNT*3];

void rgbPattern()
{
    static uint8_t brightness = 20;
    static uint8_t color = 0;

    uint8_t i;

    // Fill in some data
    for(i = 0; i < LED_COUNT; i++) {
        // LED ring is GRB
        led_data[i*3+1] = ((color == 0) || (color == 3) || (color == 5) || (color == 6)) ? (brightness) : 0;
        led_data[i*3+0] = ((color == 1) || (color == 3) || (color == 4) || (color == 6)) ? (brightness) : 0;
        led_data[i*3+2] = ((color == 2) || (color == 4) || (color == 5) || (color == 6)) ? (brightness) : 0;
    }

    color = (color + 1) % 7; // 3 for RGB, 7 for RGB???W

    // Bitbang the data...

}

void main() {
    CfgFsys();
    mDelaymS(5);

    bitbangSetup();

    while (1) {
        rgbPattern();
        bitbangWs2812(LED_COUNT, led_data);
        mDelaymS(500);
    }
}
