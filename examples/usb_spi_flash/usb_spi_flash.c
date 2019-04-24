#include <ch554.h>
#include <debug.h>
#include <bootloader.h>
#include <spi.h>

#include <stdint.h>
#include <stdbool.h>

#include "usb_spi_flash.h"
#include "spi_flash.h"

#define MODE_READY 0
#define MODE_READ_WRITELENGTH 1
#define MODE_WRITING 2

#define PAGE_SIZE 256
#define BLOCK_SIZE 65536    // Can be 32768 or 65536, choose matching Flash_EraseBlock function

#define CLEAR_BIT(reg, bit) (reg &= ~(1<<bit))
#define SET_BIT(reg, bit) (reg |= (1<<bit))

__xdata uint8_t writeBuffer[PAGE_SIZE];

uint8_t mode;
uint32_t address;
uint32_t writelength;
uint16_t count;

// Disable all SPI pins, then boot the FPGA
void releaseBus() {
    // Disable SPI pins
    P1_DIR_PU &= ~(0xF0);   // Set SPI pins to high-impedance input mode
    P1_MOD_OC &= ~(0xF0);

    // Pull FPGA reset line (pin 2.1) high to boot FPGA
    SET_BIT(P1,1);
}

// Put the FPGA in reset, then enable all SPI pins
void takeBus() {
    CLEAR_BIT(P1,1);        // Pull FPGA reset line (pin 2.1) low to hold FPGA in reset

    P1_MOD_OC &= ~(0xF0);   // Set SPI pins to push/pull mode
    P1_DIR_PU |=   0xF0;


    SPIMasterModeSet(0);    // Set up the SPI hardware
    Flash_ReleaseDeepPowerdown();
}

void setup() {
    P1_DIR_PU &= ~(0x01);   // CDONE (FPGA reset line) as high-impedance input
    P1_MOD_OC &= ~(0x01);

    P1_MOD_OC &= ~(0x02);   // CRESET as push-pull output
    P1_DIR_PU |=   0x02;

    P1_DIR_PU &= ~(0x0C);   // Unused pins P1.2, P1.3 as high-impedance inputs
    P1_MOD_OC &= ~(0x0C);

    CLEAR_BIT(P1,1);        // Pull FPGA reset line (pin 2.1) low to hold FPGA in reset

    mDelaymS(10);

    releaseBus();

    mode = MODE_READY;
}

void beginFlashWritePage(uint32_t address) {
    while (Flash_Busy()) {}

    SPIMasterAssertCS();
        CH554SPIMasterWrite(CMD_WRITE_ENABLE);
    SPIMasterDeassertCS();

    SPIMasterAssertCS();
        CH554SPIMasterWrite(CMD_PAGEPROGRAM);
        CH554SPIMasterWrite(*((uint8_t*)&(address) + 2));    // addr[23:16]
        CH554SPIMasterWrite(*((uint8_t*)&(address) + 1));    // addr[15:8]
        CH554SPIMasterWrite(*((uint8_t*)&(address) + 0));    // addr[7:0]
}

void endFlashWritePage() {
    SPIMasterDeassertCS();
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
            mode = MODE_READ_WRITELENGTH;
            address = 0;
            count = 0;
        }
    }
    else if(mode == MODE_READ_WRITELENGTH) {
        (*((uint8_t*)&(writelength) + count)) = c;    // LSB first
        count += 1;

        if(count == 4) {
            mode = MODE_WRITING;
            count = 0;

            Flash_EraseBlock64K(address);
        }
    }
    else if(mode == MODE_WRITING) {
        if(count == 0)
            beginFlashWritePage(address);

        CH554SPIMasterWrite(c);
        count++;

        if(count==PAGE_SIZE) {
            endFlashWritePage();

            address += PAGE_SIZE;
            writelength -= PAGE_SIZE;
            count = 0;

            if(writelength == 0) {
                while (Flash_Busy()) {}

                mode = MODE_READY;
            }
            else {
                if(address%BLOCK_SIZE == 0) {
                    while (Flash_Busy()) {}
                    Flash_EraseBlock64K(address);
                }
            }
        }
    }
}
