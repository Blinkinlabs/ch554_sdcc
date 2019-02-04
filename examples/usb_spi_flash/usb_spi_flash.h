#pragma once

// Pin assignments:
//
// Debug on TXD1
// ICE_SS on P1.4/SCS
// ICE_MOSI on P1.5/MOSI
// ICE_MISO on P1.6/MISO
// ICE_SCK on P1.7/SCK
// CDONE on P1.0 (input)
// CRESET_B on P1.1 (output)

void setup();

void handleRx(uint8_t c);
