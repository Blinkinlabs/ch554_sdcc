# SDCC toolchain for CH554 devices

This is a port of the CH554 SDK, from Keil C51 to SDCC.

# Usage

You'll need a recent version of SDCC, as well as gnu autotools (for make support). On windows, use mingw.

# Status

Here is a list of the different peripheral drivers and examples that need to be ported

| Peripheral | Description | Status |
| --- | --- | --- |
| ADC | Analog-to-digital converter | not started |
| DataFlash | DataFlash (EEPROM) peripheral | not started |
| GPIO | I/O peripheral example | not started |
| UART0/stdlib | stdio example using UART0 | not started |
| UART1/stdlib | stdio example using UART1 | not started |
| Watchdog: not started |
| IAP | Jump from user program to the bootloader | not started |
| PWM | Pulse Width modulation peripheral | complete |
| SPI | Serial Peripheral Interface | not started |
| Timer | 8051-style Timers 0 and 1 | not started |
| Timer2 | Extended Timer 2 | not started |
| TouchKey | Capacitive touch peripheral | in progress |
| Type-C | USB C power negotiation peripheral | not started |
| USB\Device | USB device peripheral: HID (?) profile | not started |
| S_CDC | USB device peripheral: CDC profile | complete |
| U_DISK | USB device peripheral: USB disk profile | not started |
| Compound_Dev | USB device peripheral: compound device example (?) | not started |
| USB\Host | USB host peripheral: hub (?) example | not started |
| USB\U_DISK | USB host peripheral: USB disk read/write | not started |

# Examples

## blink

Blink an LED using a software timer

Note: the delay() routines run slow in SDCC, and needs to re-tuned.

## pwm

Use the PWM hardware to generate PWM output

## pwm_interrupt

Use the PWM hardware as a periodic interrupt source

## touchkey

Exercise the capacitive touch hardware

## usb_device_cdc

Emulate a USB-to-Serial converter, as a USB-CDC device.

## ws2812

Use GPIO to bitbang a WS2812 LED pattern

