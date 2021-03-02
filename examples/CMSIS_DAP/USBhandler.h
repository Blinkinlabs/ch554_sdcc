#ifndef __USB_HANDLER_H__
#define __USB_HANDLER_H__

#include <stdint.h>
#include "USBconstant.h"

#define UsbSetupBuf     ((PUSB_SETUP_REQ)Ep0Buffer)

// Out
#define EP0_OUT_Callback USB_EP0_OUT

// IN
#define EP0_IN_Callback USB_EP0_IN

void USBInterrupt(void);
void USBDeviceSetup();

#endif

