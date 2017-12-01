# SDCC toolchain for CH554 devices

This is a port of the CH554 SDK, from Keil C51 to SDCC.

# Usage

You'll need a recent version of SDCC, as well as gnu autotools (for make support). On windows, use mingw.

# Status

These are the examples included with the original SDK:

ADC: not started
DataFlash
GPIO
IAP
PWM: complete
SPI: not started
Timer: not started
TouchKey: in progress
Type-C:
USB\Device: not started
USB\Host: not started
USB\U_DISK (host): not started
S_CDC: not started
U_DISK: not started
Compound_Dev: not started

And examples that need to be added:
UART0/stdlib: not started
UART1/stdlib: not started
Watchdog: not started

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

## ws2812

Use GPIO to bitbang a WS2812 LED pattern

