TARGET = usb_device_midi

# Adjust the XRAM location and size to leave space for the USB DMA buffers
# Buffer layout in XRAM:
# 0x0000 Ep0Buffer[8]
# 0x0040 Ep1Buffer[8]
# 0x0080 EP2Buffer[2*64]
#
# This takes a total of 256bytes, so there are 256 bytes left.
XRAM_SIZE = 0x0100
XRAM_LOC = 0x0100

C_FILES = \
	main.c \
	../../include/debug.c

#EXTRA_FLAGS += -DDE_PRINTF

pre-flash:
	

include ../Makefile.include
