// Descriptors for CMSIS_DAP

#include <stdint.h>
#include <ch554_usb.h>
#include "USBconstant.h"


//Device descriptor
__code USB_DEV_DESCR DevDesc = {
    .bLength = 18,
    .bDescriptorType = USB_DESCR_TYP_DEVICE,
    .bcdUSBH = 0x01, .bcdUSBL = 0x10,
    .bDeviceClass =  0,                 // interface will define class
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = DEFAULT_ENDP0_SIZE,
    .idVendorH = 0x12, .idVendorL = 0x09,
    .idProductH = 0xC5, .idProductL = 0x5D,
    .bcdDeviceH = 0x01, .bcdDeviceL = 0x00,
    .iManufacturer = 1,                 // string descriptors
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1
};

__code uint8_t CfgDesc[] ={
    0x09,0x02,sizeof(CfgDesc) & 0xff,sizeof(CfgDesc) >> 8,
    0x01,               /* bNumInterfaces */
    0x01,               /* bConfigurationValue */
    0x00,               /* iConfiguration */
    0x80,               /* bmAttributes */
    0x64,               /* MaxPower */
    
    // HID interface descriptor
    0x09,                       // bLength
    USB_DESCR_TYP_INTERF,       // bDescriptorType
    0x00,                       // bInterfaceNumber
    0x00,                       // bAlternateSetting
    0x02,                       // bNumEndpoints
    USB_DEV_CLASS_HID,          // bInterfaceClass
    0x00,                       // bInterfaceSubClass
    0x00,                       // bInterfaceProtocol
    0x00,                       // iInterface
    
    /* HID Descriptor */
    0x09,               /* Size of this descriptor in UINT8s. */
    USB_DESCR_TYP_HID,         // bDescriptorType
    0x10, 0x01,         /* HID Class Spec. release number. */
    0x00,               /* H/W target country. */
    0x01,               /* Number of HID class descriptors to follow. */
    USB_DESCR_TYP_REPORT,       // bDescriptorType
    sizeof(ReportDesc) & 0xff,sizeof(ReportDesc) >> 8,    /* Total length of report descriptor. */
    
    // EP Descriptor
    sizeof(USB_ENDP_DESCR),     // bLength
    USB_DESCR_TYP_ENDP,         // bDescriptorType
    0x82,                       // bEndpointAddress EP2 IN
    USB_ENDP_TYPE_INTER,        // bmAttributes
    0x40, 0x00,                 // wMaxPacketSize
    1,                          // bInterval

    // EP Descriptor
    sizeof(USB_ENDP_DESCR),     // bLength
    USB_DESCR_TYP_ENDP,         // bDescriptorType
    0x01,                       // bEndpointAddress EP1 OUT
    USB_ENDP_TYPE_INTER,        // bmAttributes
    0x40, 0x00,                 // wMaxPacketSize
    1,                          // bInterval

};


__code uint8_t ReportDescLen = sizeof(ReportDesc);

__code uint8_t ReportDesc[] ={
    0x06, 0x00, 0xFF,   // Usage Page = 0xFF00 (Vendor Defined Page 1)
    0x09, 0x01,         // Usage (Vendor Usage 1)
    0xA1, 0x01,         // Collection (Application)
    0x19, 0x01,         // Usage Minimum
    0x29, 0x40,         // Usage Maximum //64 input usages total (0x01 to 0x40)
    0x15, 0x00,         // Logical Minimum (data bytes in the report may have minimum value = 0x00)
    0x26, 0xFF, 0x00,   // Logical Maximum (data bytes in the report may have maximum value = 0x00FF = unsigned 255)
    0x75, 0x08,         // Report Size: 8-bit field size
    0x95, 0x40,         // Report Count: Make sixty-four 8-bit fields (the next time the parser hits an "Input", "Output", or "Feature" item)
    0x81, 0x00,         // Input (Data, Array, Abs): Instantiates input packet fields based on the above report size, count, logical min/max, and usage.
    0x19, 0x01,         // Usage Minimum
    0x29, 0x40,         // Usage Maximum //64 output usages total (0x01 to 0x40)
    0x91, 0x00,         // Output (Data, Array, Abs): Instantiates output packet fields. Uses same report size and count as "Input" fields, since nothing new/different was specified to the parser since the "Input" item.
    0xC0                // End Collection
};

__code uint8_t CfgDescLen = sizeof(CfgDesc);

//String Descriptors
__code uint8_t LangDes[]={0x04,0x03,0x09,0x04};           //Language Descriptor
__code uint8_t LangDesLen = sizeof(LangDes);

//Product String Descriptor
__code uint16_t Prod_Des[]={
    0x0300 + sizeof(Prod_Des),  // type and length
    // compound literals not supported in SDCC as of v4.0.3
    // (__code uint16_t[]) { u"CH55x CMSIS-DAP" }
    'C','H','5','5','x',' ', 'C','M','S','I','S','-','D','A','P'
};
__code uint8_t Prod_DesLen = sizeof(Prod_Des);

__sfr __at(sizeof(Prod_Des)) PDLen;

__code uint16_t Manuf_Des[]={
    0x0300 + sizeof(Manuf_Des), // type and length
    'W','C','H'
};
__code uint8_t Manuf_DesLen = sizeof(Manuf_Des);

// Serial string descriptor - CMSIS-DAP unique ID
__code uint16_t Ser_Des[]={
    0x0300 + sizeof(Ser_Des),   // type and length
    '1','2','3'
};
__code uint8_t Ser_DesLen = sizeof(Manuf_Des);
