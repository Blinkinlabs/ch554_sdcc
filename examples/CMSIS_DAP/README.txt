CMSIS-DAP debug probe implementation

Recommended hardware setup is with VCC=3V3 and 68-100Ohm series resistors on SWDIO & SWCLK.

Performance is partially optimized, with SWCLK running at less than 1Mhz.  Overall throughput flashing a STM32F103 with pyocd is about 6kB/s.  With optimization of the bit-banged SWD code and the USB packet handling, 20kB/s programming speeds should be possible

