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
    USBDeviceCfg();                     //Device mode configuration
    USBDeviceEndPointCfg();             //Endpoint configuration   
    USBDeviceIntCfg();                  //Interrupt configuration    
    UEP0_T_LEN = 0;
    UEP1_T_LEN = 0;
    UEP2_T_LEN = 0;                                                          
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
            if (--EP1_buffs_avail == 0) {
                //Respond NAK. Let main change response after handling.
                UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_NAK;
            }

            DAP_RxBuf = (__xdata uint8_t*) UEP1_DMA;
            EP1_buf_toggle = !EP1_buf_toggle;
            if (EP1_buf_toggle)
                UEP1_DMA = (uint16_t) Ep1Buffer + 64;
            else
                UEP1_DMA = (uint16_t) Ep1Buffer;

        }
    }
}

// perform USB bus reset/disconnect
// set UDP to GPIO mode and hold low for device disconnect
inline void resetUSB()
{
    PIN_FUNC &= ~(bUSB_IO_EN);
    UDP = 0;
    mDelaymS(50);
    UDP = 1;
    PIN_FUNC |= bUSB_IO_EN;
}

void main() {
    resetUSB();
    CfgFsys(); 
    USBInit();
    while (1) {
        uint8_t response_len;
        if (USBByteCountEP1) {
            response_len = DAP_Thread();
            USBByteCountEP1 = 0 ;
            // enable receive
            EP1_buffs_avail++;
            UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_ACK;

            //UEP2_T_LEN = response_len;
            // enable interrupt IN response
            UEP2_T_LEN = 64;            // hangs on Windoze < 64
            UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;

        }
    }
}
