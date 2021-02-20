#include <stdint.h>
#include <stdbool.h>
#include <ch554.h>
#include <ch554_usb.h>
#include "USBconstant.h"
#include "USBhandler.h"

volatile uint8_t USBByteCountEP1 = 0;      //Bytes of received data on USB endpoint

void USBInit(){
    USBDeviceCfg();                     //Device mode configuration
    USBDeviceEndPointCfg();             //Endpoint configuration   
    USBDeviceIntCfg();                  //Interrupt configuration    
    UEP0_T_LEN = 0;
    UEP1_T_LEN = 0;                     //Pre-use send length must be cleared	  
    UEP2_T_LEN = 0;                                                          
}

void USB_EP2_IN(){
    UEP2_T_LEN = 0;                     // No data to send anymore
    UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;           //Respond NAK by default
}

void USB_EP1_OUT(){
    if ( U_TOG_OK ){               // Discard unsynchronized packets
        USBByteCountEP1 = USB_RX_LEN;
        if (USBByteCountEP1){
            //Respond NAK after a packet. Let main code change response after handling.
            UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_NAK;
        }
    }
}

