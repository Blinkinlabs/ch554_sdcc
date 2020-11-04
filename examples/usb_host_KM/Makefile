TARGET = UsbHostHub_KM

FREQ_SYS=16000000
EXTRA_FLAGS = -DMAX_PACKET_SIZE=64

XRAM_SIZE = 0x380
XRAM_LOC = 0x080

C_FILES = \
	UsbHostHub_KM.c \
	usbhost.c \
	../../include/debug.c

pre-flash:
	

include ../Makefile.include
