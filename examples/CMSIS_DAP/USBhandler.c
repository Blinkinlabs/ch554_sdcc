#include <string.h>

#include <ch554.h>
#include <ch554_usb.h>
#include "USBhandler.h"
#include "USBconstant.h"
#include "config.h"

//HID functions:
void USB_EP2_IN();
void USB_EP1_OUT();

//on page 47 of data sheet, the receive buffer need to be min(possible packet size+2,64)
__xdata uint8_t  Ep0Buffer[DEFAULT_ENDP0_SIZE + 2];
__xdata uint8_t  Ep1Buffer[HID_PKT_SIZ * 2];    // EP1 OUT*2
__xdata uint8_t  Ep2Buffer[HID_PKT_SIZ];

uint8_t SetupLen;
uint8_t SetupReq,UsbConfig;

__code uint8_t *pDescr;

volatile uint8_t usbMsgFlags=0;    // uint8_t usbMsgFlags copied from VUSB

inline void NOP_Process(void) {}

// copy descriptor *pDescr to Ep0
#pragma callee_saves cpy_desc_Ep0
void cpy_desc_Ep0(uint8_t len) __naked
{
    len;            // stop unused arg warning
    __asm
    xch A, _DPL     ; ACC = len
    inc _XBUS_AUX
    mov DPL, #_Ep0Buffer
    mov DPH, #(_Ep0Buffer >> 8)
    dec _XBUS_AUX
    mov DPL, _pDescr
    mov DPH, (_pDescr + 1)
    sjmp _ccpyx
    __endasm;
}

// copy code to xram 
// *dest in DPTR1, len in A
#pragma callee_saves ccpyx
void ccpyx(__code char* src)
{
    src;            // stop unused arg warning
    __asm
    push ar7
    xch A, R7
    01$:
    clr A
    movc A, @A+DPTR
    inc DPTR
    .DB  0xA5       ;MOVX @DPTR1,A & INC DPTR1
    djnz R7, 01$
    pop ar7
    __endasm;
}

void USB_EP0_Setup(){
    uint8_t len = USB_RX_LEN;
    if(len == (sizeof(USB_SETUP_REQ)))
    {
        uint16_t wLength = ((uint16_t)UsbSetupBuf->wLengthH<<8) | (UsbSetupBuf->wLengthL);
        SetupLen = wLength;
        // maximum supported reply size is 254 bytes
        if (wLength > 254) SetupLen = 254;
        len = 0;                                                      // Default is success and upload 0 length
        SetupReq = UsbSetupBuf->bRequest;
        usbMsgFlags = 0;
        if ( ( UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )//Not standard request
        {
            switch( ( UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK ))
            {
                case USB_REQ_TYP_VENDOR:    
                {
                    switch( SetupReq )
                    {
                        default:
                            len = 0xFF; //command not supported
                            break;
                    }
                    break;
                }
                case USB_REQ_TYP_CLASS:
                {
                    switch( SetupReq )
                    {
                        default:
                            len = 0xFF; //command not supported
                            break;
                    }
                    break;
                }
                default:
                    len = 0xFF;         //command not supported
                    break;
            }

        }
        else                           //Standard request
        {
            switch(SetupReq)           //Request ccfType
            {
                case USB_GET_DESCRIPTOR:
                    switch(UsbSetupBuf->wValueH)
                {
                    case 1:             // Device Descriptor
                        pDescr = (uint8_t*)&DevDesc;
                        len = sizeof(DevDesc);
                        break;
                    case 2:             // configureation descriptor
                        pDescr = CfgDesc;                                       
                        len = CfgDescLen;
                        break;
                    case 3:             // string descriptor
                        if(UsbSetupBuf->wValueL == 0)
                        {
                            pDescr = LangDes;
                        }
                        else if(UsbSetupBuf->wValueL == 1)
                        {
                            pDescr = Manuf_Des;
                        }
                        else if(UsbSetupBuf->wValueL == 2)
                        {
                            pDescr = Prod_Des;
                        }
                        else
                        {
                            pDescr = Ser_Des;
                        }
                        len = pDescr[0];    // bLength
                        break;
                    case USB_DESCR_TYP_REPORT:
                        if(UsbSetupBuf->wValueL == 0){
                            pDescr = ReportDesc;
                            len = ReportDescLen;
                        }
                        else
                        {
                            len = 0xff;
                        }
                        break;
                    default:
                        len = 0xff;    // Unsupported descriptors or error
                        break;
                }
                    if (len != 0xff){
                        if ( SetupLen > len )
                        {
                            SetupLen = len;    // Limit length
                        }
                        len = SetupLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : SetupLen;                            //transmit length for this packet
                        cpy_desc_Ep0(len);

                        SetupLen -= len;
                        pDescr += len;
                    }
                    break;
                case USB_SET_ADDRESS:
                    SetupLen = UsbSetupBuf->wValueL;                              // Save the assigned address
                    break;
                case USB_GET_CONFIGURATION:
                    Ep0Buffer[0] = UsbConfig;
                    if ( SetupLen >= 1 )
                    {
                        len = 1;
                    }
                    break;
                case USB_SET_CONFIGURATION:
                    UsbConfig = UsbSetupBuf->wValueL;
                    break;
                case USB_GET_INTERFACE:
                    break;
                case USB_SET_INTERFACE:
                    break;
                case USB_CLEAR_FEATURE:
                    if( ( UsbSetupBuf->bRequestType & 0x1F ) == USB_REQ_RECIP_DEVICE )                  // Clear the device featuee.
                    {
                        if( ( ( ( uint16_t )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x01 )
                        {
                            if( CfgDesc[ 7 ] & 0x20 )
                            {
                                // wake up
                            }
                            else
                            {
                                len = 0xFF;                                        //Failed
                            }
                        }
                        else
                        {
                            len = 0xFF;                                            //Failed
                        }
                    }
                    else if ( ( UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )// endpoint
                    {
                        switch( UsbSetupBuf->wIndexL )
                        {
                            case 0x84:
                                UEP4_CTRL = UEP4_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
                                break;
                            case 0x04:
                                UEP4_CTRL = UEP4_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
                                break;
                            case 0x83:
                                UEP3_CTRL = UEP3_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
                                break;
                            case 0x03:
                                UEP3_CTRL = UEP3_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
                                break;
                            case 0x82:
                                UEP2_CTRL = UEP2_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
                                break;
                            case 0x02:
                                UEP2_CTRL = UEP2_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
                                break;
                            case 0x81:
                                UEP1_CTRL = UEP1_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
                                break;
                            case 0x01:
                                UEP1_CTRL = UEP1_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
                                break;
                            default:
                                len = 0xFF;                                         // Unsupported endpoint
                                break;
                        }
                    }
                    else
                    {
                        len = 0xFF;                                                // Unsupported for non-endpoint
                    }
                    break;
                case USB_SET_FEATURE:                                          // Set Feature
                    if( ( UsbSetupBuf->bRequestType & 0x1F ) == USB_REQ_RECIP_DEVICE )                  // Set  the device featuee.
                    {
                        if( ( ( ( uint16_t )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x01 )
                        {
                            if( CfgDesc[ 7 ] & 0x20 )
                            {
                                // suspend not supported 
                            }
                            else
                            {
                                len = 0xFF;                                        // Failed
                            }
                        }
                        else
                        {
                            len = 0xFF;                                            // Failed
                        }
                    }
                    else if( ( UsbSetupBuf->bRequestType & 0x1F ) == USB_REQ_RECIP_ENDP )             //endpoint
                    {
                        if( ( ( ( uint16_t )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x00 )
                        {
                            switch( ( ( uint16_t )UsbSetupBuf->wIndexH << 8 ) | UsbSetupBuf->wIndexL )
                            {
                                case 0x84:
                                    UEP4_CTRL = UEP4_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;// Set endpoint4 IN STALL 
                                    break;
                                case 0x04:
                                    UEP4_CTRL = UEP4_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;// Set endpoint4 OUT Stall 
                                    break;
                                case 0x83:
                                    UEP3_CTRL = UEP3_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;// Set endpoint3 IN STALL 
                                    break;
                                case 0x03:
                                    UEP3_CTRL = UEP3_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;// Set endpoint3 OUT Stall 
                                    break;
                                case 0x82:
                                    UEP2_CTRL = UEP2_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;// Set endpoint2 IN STALL 
                                    break;
                                case 0x02:
                                    UEP2_CTRL = UEP2_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;// Set endpoint2 OUT Stall 
                                    break;
                                case 0x81:
                                    UEP1_CTRL = UEP1_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;// Set endpoint1 IN STALL 
                                    break;
                                case 0x01:
                                    UEP1_CTRL = UEP1_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;// Set endpoint1 OUT Stall 
                                default:
                                    len = 0xFF;                                    // Failed
                                    break;
                            }
                        }
                        else
                        {
                            len = 0xFF;                                      // Failed
                        }
                    }
                    else
                    {
                        len = 0xFF;                                          // Failed
                    }
                    break;
                case USB_GET_STATUS:
                    Ep0Buffer[0] = 0x00;
                    Ep0Buffer[1] = 0x00;
                    if ( SetupLen >= 2 )
                    {
                        len = 2;
                    }
                    else
                    {
                        len = SetupLen;
                    }
                    break;
                default:
                    len = 0xff;                                                    // Failed
                    break;
            }
        }
    }
    else
    {
        len = 0xff;                                                         //Wrong packet length
    }
    if(len == 0xff)
    {
        SetupReq = 0xFF;
        UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;//STALL
    }
    else if(len <= DEFAULT_ENDP0_SIZE)                                                       // Tx data to host or send 0-length packet
    {
        UEP0_T_LEN = len;
        UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//Expect DATA1, Answer ACK
    }
    else
    {
        // TODO: remove unreachable code here
        UEP0_T_LEN = 0;  // Tx data to host or send 0-length packet
        UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//Expect DATA1, Answer ACK
    }
}

void USB_EP0_IN(){
    switch(SetupReq)
    {
        case USB_GET_DESCRIPTOR:
        {
            uint8_t len = SetupLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : SetupLen;                                 //send length
            cpy_desc_Ep0(len);

            SetupLen -= len;
            pDescr += len;
            UEP0_T_LEN = len;
            UEP0_CTRL ^= bUEP_T_TOG;                    //Switch between DATA0 and DATA1
        }
            break;
        case USB_SET_ADDRESS:
            USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | SetupLen;
            UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
            break;
        default:
            UEP0_T_LEN = 0;                                                      // End of transaction
            UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
            break;
    }
}

void USB_EP0_OUT(){
    UEP0_T_LEN = 0;
    UEP0_CTRL |= UEP_R_RES_ACK | UEP_T_RES_NAK;  //Respond Nak
}

#pragma save
#pragma nooverlay
void USBInterrupt(void) {   //inline not really working in multiple files in SDCC
    if(UIF_TRANSFER) {
        // Dispatch to service functions
        uint8_t callIndex=USB_INT_ST & MASK_UIS_ENDP;
        switch (USB_INT_ST & MASK_UIS_TOKEN) {
            case UIS_TOKEN_OUT:
            {//SDCC will take IRAM if array of function pointer is used.
                switch (callIndex) {
                    case 0: EP0_OUT_Callback(); break;
                    case 1: USB_EP1_OUT(); break;
                    case 2: break;
                    case 3: break;
                    case 4: break;
                    default: break;
                }
            }
                break;
            case UIS_TOKEN_IN:
            {//SDCC will take IRAM if array of function pointer is used.
                switch (callIndex) {
                    case 0: EP0_IN_Callback(); break;
                    case 1: break;
                    case 2: USB_EP2_IN(); break;
                    case 3: break;
                    case 4: break;
                    default: break;
                }
            }
                break;
            case UIS_TOKEN_SETUP:
            {
                switch (callIndex) {
                    case 0: USB_EP0_Setup(); break;
                    default: break;
                }
            }
                break;
            default:
                break;
        }
        
        UIF_TRANSFER = 0;               // Clear interrupt flag
    }
    
    // Device mode USB bus reset
    if(UIF_BUS_RST) {
        UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;                //Endpoint 1 automatically flips the sync flag, and IN transaction returns NAK
        UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;        //Endpoint 2 automatically flips the sync flag, IN transaction returns NAK, OUT returns ACK
        //UEP4_CTRL = UEP_T_RES_NAK | UEP_R_RES_ACK;  //bUEP_AUTO_TOG only work for endpoint 1,2,3
        
        USB_DEV_AD = 0;
        UIF_SUSPEND = 0;
        UIF_TRANSFER = 0;
        UIF_BUS_RST = 0;                                                        // Clear interrupt flag
        
    }
    
    // USB bus suspend / wake up
    if (UIF_SUSPEND) {
        UIF_SUSPEND = 0;
        if ( USB_MIS_ST & bUMS_SUSPEND ) {
           // suspend not supported 
        } else {    // Unexpected interrupt, not supposed to happen !
            USB_INT_FG = 0xFF;        // Clear interrupt flag
        }
    }
}
#pragma restore

void USBDeviceSetup()
{
    //USB internal pull-up enable, return NAK if USB INT flag not clear 
    USB_CTRL = bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN;

    // enable port, full-speed, disable UDP/UDM pulldown resistor
    UDEV_CTRL = bUD_PD_DIS | bUD_PORT_EN;

    // configure interrupts
    // USB_INT_EN |= bUIE_SUSPEND;         //Enable device hang interrupt
    USB_INT_EN |= bUIE_TRANSFER;        //Enable USB transfer completion interrupt
    USB_INT_EN |= bUIE_BUS_RST;         //Enable device mode USB bus reset interrupt
    USB_INT_FG |= 0x1F;                 //Clear interrupt flag
    IE_USB = 1;                         //Enable USB interrupt
    EA = 1;                             //Enable global interrupts

    // configure endpoints
    UEP1_DMA = (uint16_t) Ep1Buffer;    //Endpoint data transfer address
    UEP2_DMA = (uint16_t) Ep2Buffer;    //Endpoint data transfer address

    UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;        //Endpoint 1 automatically flips the sync flag, IN transaction returns NAK, OUT returns ACK
    UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_NAK;        //Endpoint 2 automatically flips the sync flag, IN & OUT transaction returns NAK
    
    UEP0_DMA = (uint16_t) Ep0Buffer;    //Endpoint 0 data transfer address
    UEP4_1_MOD = bUEP1_RX_EN;
    UEP2_3_MOD = bUEP2_TX_EN;
    UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;                //Manual flip, OUT transaction returns ACK, IN transaction returns NAK

    UEP0_T_LEN = 0;
    UEP1_T_LEN = 0;
    UEP2_T_LEN = 0;                                                          
}
