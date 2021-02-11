#ifndef __USB_HID_H__
#define __USB_HID_H__

#include <stdint.h>

extern volatile uint8_t USBByteCountEP1;
void USBInit(void);

#endif

