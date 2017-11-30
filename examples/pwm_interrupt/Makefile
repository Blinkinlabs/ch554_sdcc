#######################################################

# toolchain
CC = sdcc.exe
PACK_HEX = packihx.exe

#######################################################

TARGET = pwm_interrupt

CFLAGS = -V -mmcs51 --debug --model-small --nostdlib --std-sdcc11 -DFREQ_SYS=16000000
LFLAGS = $(CFLAGS)

# Note: Target name must be the same as the first filename...
C_FILES = \
	pwm_interrupt.c

RELS := $(C_FILES:.c=.rel)

print-%  : ; @echo $* = $($*)

%.rel : %.c
	$(CC) -c $(CFLAGS) $<

$(TARGET).ihx: $(RELS)
	$(CC) $(RELS)
	
$(TARGET).hex: $(TARGET).ihx
	$(PACK_HEX) $(TARGET).ihx > $(TARGET).hex

all: $(TARGET).hex

clean:
	rm -f \
	$(C_FILES:.c=.asm) \
	$(C_FILES:.c=.lk) \
	$(C_FILES:.c=.map) \
	$(C_FILES:.c=.lst) \
	$(C_FILES:.c=.mem) \
	$(C_FILES:.c=.rel) \
	$(C_FILES:.c=.rst) \
	$(C_FILES:.c=.sym) \
	$(C_FILES:.c=.ihx) \
	$(C_FILES:.c=.adb) \
	$(TARGET).hex
