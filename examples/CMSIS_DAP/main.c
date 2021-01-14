/*
  CMSIS_DAP
  Based on DAPLink and Deqing Sun's CH55xduino port 
  https://github.com/ARMmbed/DAPLink

  Code cleanup and optimization by Ralph Doncaster

  DAP.h defaults:
  RST   = P30
  SWCLK = P31
  SWDIO = P32
  LED   = P33
*/

#include <debug.h>
#include "DAP.h"
#include "USBHID.h"
#include "USBhandler.h"

void DeviceUSBInterrupt(void) __interrupt (INT_NO_USB)
{
        USBInterrupt();
}

void main() {
    CfgFsys(); 
    USBInit();
    while (1) {
        if (USBByteCountEP1) {
            DAP_Thread();
            USBByteCountEP1 = 0 ;

            UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_ACK; //enable receive

            UEP1_T_LEN = 64;
            UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK; //enable send
        }
    }
}
