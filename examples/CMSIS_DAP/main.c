/*
  CMSIS_DAP
  Based on DAPLink and Deqing Sun's CH55xduino port 
  https://github.com/ARMmbed/DAPLink
  Ralph Doncaster 2020, 2021

  see DAP.h for RST, SWCLK, & SWDIO pin defines
*/

#include <stdint.h>

#include <ch554.h>
#include <ch554_usb.h>
#include <debug.h>

#include "DAP.h"
#include "USBhandler.h"

void DeviceUSBInterrupt(void) __interrupt (INT_NO_USB)
{
        USBInterrupt();
}

//Bytes of received data on USB endpoint
volatile uint8_t USBByteCountEP1 = 0;

// for EP1 OUT double-buffering 
volatile uint8_t EP1_buffs_avail = 2;
__bit EP1_buf_toggle = 0;

void USBInit(){
    USBDeviceSetup();

    // single Tx buffer for DAP replies
    DAP_TxBuf = (__xdata uint8_t*) UEP2_DMA;
}

void USB_EP2_IN(){
    UEP2_T_LEN = 0;                     // No data to send anymore
    UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;           //Respond NAK by default
}

void USB_EP1_OUT(){
    if ( U_TOG_OK ){               // Discard unsynchronized packets
        USBByteCountEP1 = USB_RX_LEN;
        if (USBByteCountEP1){
            //Respond NAK. Let main change response after handling.
            UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_NAK;

            // double-buffering of DAP request packets
            DAP_RxBuf = (__xdata uint8_t*) UEP1_DMA;
            EP1_buf_toggle = !EP1_buf_toggle;
            if (EP1_buf_toggle)
                UEP1_DMA = (uint16_t) Ep1Buffer + 64;
            else
                UEP1_DMA = (uint16_t) Ep1Buffer;

        }
    }
}

void main() {
    CfgFsys(); 
    disconnectUSB();
    USBInit();
    LED = 0;
    while (1) {
        uint8_t response_len;
        // process if a DAP packet is received, and TxBuf is empty
        // save ByteCountEP1?
        if (USBByteCountEP1 && !UEP2_T_LEN) {
            __xdata uint8_t* RxPkt = DAP_RxBuf;
            if (--EP1_buffs_avail) {
                USBByteCountEP1 = 0 ;
                // Rx another packet while DAP_Thread runs
                UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_ACK;
            }

            response_len = DAP_Thread(RxPkt);

            //UEP2_T_LEN = response_len;
            // enable interrupt IN response
            UEP2_T_LEN = 64;            // hangs on Windoze < 64
            UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;

            // enable receive
            EP1_buffs_avail++;
            UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_ACK;
        }
    }
}
