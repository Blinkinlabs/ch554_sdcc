#pragma once

#define BOOT_ADDR  0x3800
void (* __data bootloader)(void) = BOOT_ADDR;
