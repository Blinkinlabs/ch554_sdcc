#ifndef __CONST_DATA_H__
#define __CONST_DATA_H__

#include <stdint.h>
#include <ch554_usb.h>

#define  EP0_ADDR 0
#define  EP1_ADDR 10
#define  EP2_ADDR 138

extern __code USB_DEV_DESCR DevDesc;
extern __code uint8_t CfgDesc[];
extern __code uint8_t LangDes[];
extern __code uint8_t ReportDesc[];
extern __code uint16_t Prod_Des[];
extern __code uint16_t Manuf_Des[];
extern __code uint16_t Ser_Des[];

extern __code uint8_t CfgDescLen;
extern __code uint8_t LangDesLen;
extern __code uint8_t ReportDescLen;
extern __code uint8_t Prod_DesLen;
extern __code uint8_t Manuf_DesLen;
extern __code uint8_t Ser_DesLen;

#endif
