#ifndef __USB_HANDLER_H__
#define __USB_HANDLER_H__

#include <stdint.h>
#include "USBconstant.h"

extern uint16_t SetupLen;
extern uint8_t SetupReq,UsbConfig;
extern const __code uint8_t *pDescr;


#define UsbSetupBuf     ((PUSB_SETUP_REQ)Ep0Buffer)

// Out
#define EP0_OUT_Callback USB_EP0_OUT
#define EP1_OUT_Callback USB_EP1_OUT
#define EP2_OUT_Callback NOP_Process
#define EP3_OUT_Callback NOP_Process
#define EP4_OUT_Callback NOP_Process

// SOF
#define EP0_SOF_Callback NOP_Process
#define EP1_SOF_Callback NOP_Process
#define EP2_SOF_Callback NOP_Process
#define EP3_SOF_Callback NOP_Process
#define EP4_SOF_Callback NOP_Process

// IN
#define EP0_IN_Callback USB_EP0_IN
#define EP1_IN_Callback USB_EP1_IN
#define EP2_IN_Callback NOP_Process
#define EP3_IN_Callback NOP_Process
#define EP4_IN_Callback NOP_Process

// SETUP
#define EP0_SETUP_Callback USB_EP0_SETUP
#define EP1_SETUP_Callback NOP_Process
#define EP2_SETUP_Callback NOP_Process
#define EP3_SETUP_Callback NOP_Process
#define EP4_SETUP_Callback NOP_Process

void USBInterrupt(void);
void USBDeviceCfg();
void USBDeviceIntCfg();
void USBDeviceEndPointCfg();

#endif

