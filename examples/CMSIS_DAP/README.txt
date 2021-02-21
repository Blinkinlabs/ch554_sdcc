CMSIS-DAP debug probe implementation

Recommended hardware setup is with VCC=3V3 and 68-100Ohm series resistors on SWDIO & SWCLK.

Performance is partially optimized, with SWCLK running at less than 1Mhz.  Overall throughput flashing a STM32F103 with openocd is 9kB/s.  pyOCD, being an interpreted python program, is usually slower.

