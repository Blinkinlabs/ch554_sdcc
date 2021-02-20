/*
  CMSIS_DAP
  Based on DAPLink and Deqing Sun's CH55xduino port 
  https://github.com/ARMmbed/DAPLink

  see DAP.h for RST, SWCLK, & SWDIO pin defines
*/

#include <debug.h>
#include "DAP.h"
#include "USBHID.h"
#include "USBhandler.h"

void DeviceUSBInterrupt(void) __interrupt (INT_NO_USB)
{
        USBInterrupt();
}

// perform USB bus reset/disconnect
// set UDP to GPIO mode and hold low for device disconnect
void resetUSB()
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

            //UEP2_T_LEN = response_len;
            UEP2_T_LEN = 64;            // hangs on Windoze < 64
            UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK; //enable send

            UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_ACK; //enable receive
        }
    }
}
