TARGET = i2c_gpio_expander

C_FILES = \
	main.c \
	gpio.c \
	i2c_slave.c \
	mdio_master.c \
	../../include/debug.c

pre-flash:
	
FREQ_SYS = 16000000
EXTRA_FLAGS = -DUART0_BAUD=57600 -DI2C_SLAVE_REG_COUNT=10

include ../Makefile.include
