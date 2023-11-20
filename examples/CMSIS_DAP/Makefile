TARGET = daplink

# 24M not stable if VCC = 3V3
FREQ_SYS = 16000000

# reserve 256 bytes for USB DMA buffers
XRAM_SIZE = 0x0300
XRAM_LOC = 0x0100

# no initialized data in xram
EXTRA_FLAGS = --no-xinit-opt

C_FILES = \
    $(wildcard *.c) \
	../../include/debug.c

pre-flash:
	

include ../Makefile.include
