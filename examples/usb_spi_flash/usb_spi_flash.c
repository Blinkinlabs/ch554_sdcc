#include <ch554.h>
#include <debug.h>
#include <bootloader.h>
#include <spi.h>

#include <stdint.h>

#include "usb_spi_flash.h"
#include "spi_flash.h"

#define MODE_READY 0
#define MODE_WRITING 1

#define PAGE_SIZE 256
#define SECTOR_SIZE 4096

#define CLEAR_BIT(reg, bit) (reg &= ~(1<<bit))
#define SET_BIT(reg, bit) (reg |= (1<<bit))

__xdata uint8_t writeBuffer[PAGE_SIZE];

uint8_t mode;
uint32_t address;
uint16_t count;

// Disable all SPI pins, then boot the FPGA
void releaseBus() {
    // Disable SPI pins
    P1_DIR_PU &= ~(0xF0);   // Disable pullups
    P1_MOD_OC |= 0xF0;      // Set to open drain mode
    P1 |= 0xF0;             // Set outputs high

    // Pull FPGA reset line (pin 2.1) high to boot FPGA
    SET_BIT(P1,1);
}

// Put the FPGA in reset, then enable all SPI pins
void takeBus() {
    CLEAR_BIT(P1,1);        // Pull FPGA reset line (pin 2.1) low to hold FPGA in reset

    SPIMasterModeSet(0);    // Set up the SPI hardware
    Flash_ReleaseDeepPowerdown();
}

void setup() {
    // CRESET_B (FPGA reset line) as push-pull output
    SET_BIT(P1_DIR_PU, 1);
    CLEAR_BIT(P1_MOD_OC, 1);

    releaseBus();

    mode = MODE_READY;
}

void handleRx(uint8_t c) {

    if(mode == MODE_READY) {
        if(c == 'E') {          // Put CH554 in bootloader mode
        	USB_INT_EN = 0;
	        USB_CTRL = 0x06;
	
        	mDelaymS(100);
	
	        EA = 0;
            bootloader();
        }
        else if(c=='t') {       // Hang the FPGA, then take the SPI bus
            takeBus();
        }
        else if(c=='T') {       // Release the SPI bus, and boot the FPGA
            releaseBus();
        }
        else if(c=='i') {       // Hang the FPGA, then take the SPI bus
            Flash_ReadJEDECID();
        }
        else if(c == 'e') {     // Erase SPI flash
            Flash_EraseChip();
        }
        else if(c == 'r') {
            Flash_Read(0x00000000, PAGE_SIZE, writeBuffer);
        }
        else if(c == 'w') {
            mode = MODE_WRITING;
            address = 0;
            count = 0;
        }
    }
    else if(mode == MODE_WRITING) {
        writeBuffer[count++] = c;

        if(count==PAGE_SIZE) {
            if(address%SECTOR_SIZE == 0)
                Flash_EraseSector(address);

            Flash_Write(address, PAGE_SIZE, writeBuffer);

            address += PAGE_SIZE;
            count = 0;
        }
    }
}
