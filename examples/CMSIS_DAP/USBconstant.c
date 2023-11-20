// Descriptors for CMSIS_DAP

#include <stdint.h>
#include <ch554_usb.h>
#include "config.h"


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

#if 0
__code uint8_t CfgDesc[] ={
    0x09,0x02,sizeof(CfgDesc) & 0xff,sizeof(CfgDesc) >> 8,
    0x01,               /* bNumInterfaces */
    0x01,               /* bConfigurationValue */
    0x00,               /* iConfiguration */
    0x80,               /* bmAttributes */
    50,                 /* MaxPower in 2mA units */
    
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
#endif

__code uint8_t CfgDescLen = sizeof(CfgDesc);

__code struct {
    USB_CFG_DESCR config;
    USB_ITF_DESCR interface;
    USB_HID_DESCR hid;
    USB_ENDP_DESCR ep2IN;
    USB_ENDP_DESCR ep1OUT;
} CfgDesc = {
    .config = {
        .bLength = sizeof(USB_CFG_DESCR),
        .bDescriptorType = USB_DESCR_TYP_CONFIG,
        .wTotalLengthL = sizeof(CfgDesc),
        .wTotalLengthH = 0,
        .bNumInterfaces = 1,
        .bConfigurationValue = 1,
        .iConfiguration = 0,            // string index
        .bmAttributes = 0x80,
        .MaxPower = 50                  // 2mA units
    },
    .interface = {
        .bLength = sizeof(USB_ITF_DESCR),
        .bDescriptorType = USB_DESCR_TYP_INTERF,
        .bInterfaceNumber = 0,
        .bAlternateSetting = 0,
        .bNumEndpoints = 2,
        .bInterfaceClass = USB_DEV_CLASS_HID,
        .bInterfaceSubClass = 0,
        .bInterfaceProtocol = 0,
        .iInterface = 0,                // no interface string

    },
    .hid = {
        .bLength = sizeof(USB_HID_DESCR),
        .bDescriptorType = USB_DESCR_TYP_HID,
        .bcdHIDH = 0x01,                // HID class version 
        .bcdHIDL = 0x10, 
        .bCountryCode = 0, 
        .bNumDescriptors = 1,
        .bDescriptorTypeX = USB_DESCR_TYP_REPORT,
        .wDescriptorLengthL = sizeof(ReportDesc),
        .wDescriptorLengthH = 0,
    },
    .ep2IN = {
        .bLength = sizeof(USB_ENDP_DESCR),
        .bDescriptorType = USB_DESCR_TYP_ENDP,
        .bEndpointAddress = 0x82,       // EP2 IN
        .bmAttributes = USB_ENDP_TYPE_INTER,
        .wMaxPacketSizeL = HID_PKT_SIZ,
        .wMaxPacketSizeH = 0,
        .bInterval = 1,                 // poll every 1 ms
    },
    .ep1OUT = {
        .bLength = sizeof(USB_ENDP_DESCR),
        .bDescriptorType = USB_DESCR_TYP_ENDP,
        .bEndpointAddress = 0x01,       // EP1 IN
        .bmAttributes = USB_ENDP_TYPE_INTER,
        .wMaxPacketSizeL = HID_PKT_SIZ,
        .wMaxPacketSizeH = 0,
        .bInterval = 1,                 // poll every 1 ms
    }
};

__code uint8_t ReportDesc[] ={
    0x06, 0x00, 0xFF,   // Usage Page = 0xFF00 (Vendor Defined Page 1)
    // USB-IF HID tool says vendor usage not required, but Win7 needs it
    0x09, 0x01,         // Usage (Vendor Usage 1)
    0xA1, 0x01,         // Collection (Application)
    0x15, 0x00,         //  Logical minimum value 0
    0x25, 0xFF,         //  Logical maximum value 255
    0x75, 0x08,         //  Report Size: 8-bit field size
    0x95, HID_PKT_SIZ,  //  Report Count: Make 64 fields

    // Input Report
    0x09, 0x01,         //  Usage (Vendor Usage 1)
    0x81, 0x02,         //  Input (Data,Var,Abs,No Wrap,Linear)

    // Output Report
    0x09, 0x01,         //  Usage (Vendor Usage 1)
    0x91, 0x02,         //  Output (Data,Var,Abs,No Wrap,Linear)

    0xC0                // End Collection
};
__code uint8_t ReportDescLen = sizeof(ReportDesc);

//String Descriptors
//Language Descriptor
__code uint16_t LangDes[]={
    0x0300 + sizeof(LangDes),   // type and length
    0x0409                      // 0x0409 = US English language code
};

//Product String Descriptor
__code uint16_t Prod_Des[]={
    0x0300 + sizeof(Prod_Des),  // type and length

    // compound literals not supported in SDCC as of v4.0.3
    // (__code uint16_t[]) { u"CH55x CMSIS-DAP" }
    'C','H','5','5','x',' ', 'C','M','S','I','S','-','D','A','P'
};

__code uint16_t Manuf_Des[]={
    0x0300 + sizeof(Manuf_Des), // type and length
    'W','C','H'
};

// Serial string descriptor - CMSIS-DAP unique ID
__code uint16_t Ser_Des[]={
    0x0300 + sizeof(Ser_Des),   // type and length
    'N','R'
};

