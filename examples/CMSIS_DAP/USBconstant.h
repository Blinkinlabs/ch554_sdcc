#ifndef __CONST_DATA_H__
#define __CONST_DATA_H__

#include <stdint.h>
#include <ch554_usb.h>

extern __code USB_DEV_DESCR DevDesc;
extern __code uint8_t CfgDesc[];
extern __code uint8_t ReportDesc[];
extern __code uint16_t LangDes[];
extern __code uint16_t Prod_Des[];
extern __code uint16_t Manuf_Des[];
extern __code uint16_t Ser_Des[];

extern __code uint8_t CfgDescLen;
extern __code uint8_t ReportDescLen;

#endif
