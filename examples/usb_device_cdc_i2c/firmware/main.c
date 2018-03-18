/********************************** (C) COPYRIGHT *******************************
* File Name		: main.c
* Author		: Zhiyuan Wan
* License		: MIT
* Version		: V1.0
* Date			: 2018/03/17
* Description		: CH551做USB转I2C，使用CDC-ACM协议
*******************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_ENDP0_SIZE	64
#define DEFAULT_ENDP1_SIZE	64
#include <ch554.h>
#include <ch554_usb.h>
#include <debug.h>

#include "i2c.h"

__xdata __at (0x0000) uint8_t  Ep0Buffer[DEFAULT_ENDP0_SIZE];	//端点0 OUT&IN缓冲区，必须是偶地址
__xdata __at (0x0040) uint8_t  Ep1Buffer[DEFAULT_ENDP1_SIZE];	//端点1上传缓冲区
__xdata __at (0x0080) uint8_t  Ep2Buffer[2*MAX_PACKET_SIZE];	//端点2 IN & OUT缓冲区,必须是偶地址

uint16_t SetupLen;
uint8_t   SetupReq,Count,UsbConfig;
const uint8_t *  pDescr;			//USB配置标志
USB_SETUP_REQ   SetupReqBuf;			//暂存Setup包
#define UsbSetupBuf	 ((PUSB_SETUP_REQ)Ep0Buffer)

#define  SET_LINE_CODING				0X20			// Configures DTE rate, stop-bits, parity, and number-of-character
#define  GET_LINE_CODING				0X21			// This request allows the host to find out the currently configured line coding.
#define  SET_CONTROL_LINE_STATE				0X22			// This request generates RS-232/V.24 style control signals.

uint32_t Baud = 0;

/*设备描述符*/
__code uint8_t DevDesc[] = {0x12,0x01,0x10,0x01,0x02,0x00,0x00,DEFAULT_ENDP0_SIZE,
							0x86,0x1a,0x22,0x57,0x00,0x01,0x01,0x02,
							0x03,0x01
						   };
#define SI5351_ReferenceClock		"26000000"
#define Device_Version			"1.0.1"

__code uint8_t CfgDesc[] ={
	0x09,0x02,0x43,0x00,0x02,0x01,0x00,0xa0,0x32,			 //配置描述符（两个接口）
	//以下为接口0（CDC接口）描述符
	0x09,0x04,0x00,0x00,0x01,0x02,0x02,0x01,0x00,			 //CDC接口描述符(一个端点)
	//以下为功能描述符
	0x05,0x24,0x00,0x10,0x01,								 //功能描述符(头)
	0x05,0x24,0x01,0x00,0x00,								 //管理描述符(没有数据类接口) 03 01
	0x04,0x24,0x02,0x02,									  //支持Set_Line_Coding、Set_Control_Line_State、Get_Line_Coding、Serial_State
	0x05,0x24,0x06,0x00,0x01,								 //编号为0的CDC接口;编号1的数据类接口
	0x07,0x05,0x81,0x03,0x10,0x00,0x40,					   //中断上传端点描述符
	//以下为接口1（数据接口）描述符
	0x09,0x04,0x01,0x00,0x02,0x0a,0x00,0x00,0x00,			 //数据接口描述符
	0x07,0x05,0x02,0x02,0x40,0x00,0x00,					   //端点描述符
	0x07,0x05,0x82,0x02,0x40,0x00,0x00,					   //端点描述符
};
/*字符串描述符*/
unsigned char  __code LangDes[]={0x04,0x03,0x09,0x04};		   //语言描述符
unsigned char  __code SerDes[]={								 //序列号字符串描述符
				0x14,0x03,
				'2',0x00,'0',0x00,'1',0x00,'8',0x00,'-',0x00,
				'3',0x00,'-',0x00,
				'1',0x00,'7',0x00
				};
unsigned char  __code Prod_Des[]={								//产品字符串描述符
				0x16,0x03,
				'U',0x00,'S',0x00,'B',0x00,' ',0x00,'t',0x00,'o',0x00, ' ', 0x00, 
				'I',0x00,'2',0x00,'C',0x00,
				};
unsigned char  __code Manuf_Des[]={
	0x18,0x03,
	'Z', 0x00, 'h', 0x00, 'i', 0x00,'Y',0x00,'u', 0x00, 'a', 0x00, 'n', 0x00, ' ', 0x00, 
	'W', 0x00, 'a', 0x00, 'n', 0x00
};

//cdc参数
__xdata uint8_t LineCoding[7]={0x00,0xe1,0x00,0x00,0x00,0x00,0x08};   //初始化波特率为57600，1停止位，无校验，8数据位。

#define UART_REV_LEN  64				 //串口接收缓冲区大小
__idata uint8_t Receive_Uart_Buf[UART_REV_LEN];   //串口接收缓冲区
volatile __idata uint8_t Uart_Input_Point = 0;   //循环缓冲区写入指针，总线复位需要初始化为0
volatile __idata uint8_t Uart_Output_Point = 0;  //循环缓冲区取出指针，总线复位需要初始化为0
volatile __idata uint8_t UartByteCount = 0;	  //当前缓冲区剩余待取字节数


volatile __idata uint8_t USBByteCount = 0;	  //代表USB端点接收到的数据
volatile __idata uint8_t USBBufOutPoint = 0;	//取数据指针

volatile __idata uint8_t UpPoint2_Busy  = 0;   //上传端点是否忙标志

#define BOOT_ADDR  0x3800

/* This function provided a way to access the internal bootloader */
void jump_to_bootloader()
{
	USB_INT_EN = 0;
	USB_CTRL = 0x06;
	
	mDelaymS(100);
	
	EA = 0;/* Disable all interrupts */
	
	__asm
		LJMP BOOT_ADDR /* Jump to bootloader */
	__endasm;	
	while(1); 
}



/*******************************************************************************
* Function Name  : USBDeviceCfg()
* Description	: USB设备模式配置
* Input		  : None
* Output		 : None
* Return		 : None
*******************************************************************************/
void USBDeviceCfg()
{
	USB_CTRL = 0x00;														   //清空USB控制寄存器
	USB_CTRL &= ~bUC_HOST_MODE;												//该位为选择设备模式
	USB_CTRL |=  bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN;					//USB设备和内部上拉使能,在中断期间中断标志未清除前自动返回NAK
	USB_DEV_AD = 0x00;														 //设备地址初始化
	//	 USB_CTRL |= bUC_LOW_SPEED;
	//	 UDEV_CTRL |= bUD_LOW_SPEED;												//选择低速1.5M模式
	USB_CTRL &= ~bUC_LOW_SPEED;
	UDEV_CTRL &= ~bUD_LOW_SPEED;											 //选择全速12M模式，默认方式
	UDEV_CTRL = bUD_PD_DIS;  // 禁止DP/DM下拉电阻
	UDEV_CTRL |= bUD_PORT_EN;												  //使能物理端口
}
/*******************************************************************************
* Function Name  : USBDeviceIntCfg()
* Description	: USB设备模式中断初始化
* Input		  : None
* Output		 : None
* Return		 : None
*******************************************************************************/
void USBDeviceIntCfg()
{
	USB_INT_EN |= bUIE_SUSPEND;											   //使能设备挂起中断
	USB_INT_EN |= bUIE_TRANSFER;											  //使能USB传输完成中断
	USB_INT_EN |= bUIE_BUS_RST;											   //使能设备模式USB总线复位中断
	USB_INT_FG |= 0x1F;													   //清中断标志
	IE_USB = 1;															   //使能USB中断
	EA = 1;																   //允许单片机中断
}
/*******************************************************************************
* Function Name  : USBDeviceEndPointCfg()
* Description	: USB设备模式端点配置，模拟兼容HID设备，除了端点0的控制传输，还包括端点2批量上下传
* Input		  : None
* Output		 : None
* Return		 : None
*******************************************************************************/
void USBDeviceEndPointCfg()
{
	// TODO: Is casting the right thing here? What about endianness?
	UEP1_DMA = (uint16_t) Ep1Buffer;													  //端点1 发送数据传输地址
	UEP2_DMA = (uint16_t) Ep2Buffer;													  //端点2 IN数据传输地址
	UEP2_3_MOD = 0xCC;														 //端点2/3 单缓冲收发使能
	UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;				 //端点2自动翻转同步标志位，IN事务返回NAK，OUT返回ACK

	UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;								 //端点1自动翻转同步标志位，IN事务返回NAK
	UEP0_DMA = (uint16_t) Ep0Buffer;													  //端点0数据传输地址
	UEP4_1_MOD = 0X40;														 //端点1上传缓冲区；端点0单64字节收发缓冲区
	UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;								 //手动翻转，OUT事务返回ACK，IN事务返回NAK
}

/*******************************************************************************
* Function Name  : DeviceInterrupt()
* Description	: CH559USB中断处理函数
*******************************************************************************/
void DeviceInterrupt(void) __interrupt (INT_NO_USB)					   //USB中断服务程序,使用寄存器组1
{
	uint16_t len;
	if(UIF_TRANSFER)															//USB传输完成标志
	{
		switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
		{
		case UIS_TOKEN_IN | 1:												  //endpoint 1# 端点中断上传
			UEP1_T_LEN = 0;
			UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;		   //默认应答NAK
			break;
		case UIS_TOKEN_IN | 2:												  //endpoint 2# 端点批量上传
		{
			UEP2_T_LEN = 0;													//预使用发送长度一定要清空
			UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;		   //默认应答NAK
			UpPoint2_Busy = 0;												  //清除忙标志
		}
			break;
		case UIS_TOKEN_OUT | 2:												 //endpoint 3# 端点批量下传
			if ( U_TOG_OK )													 // 不同步的数据包将丢弃
			{
				USBByteCount = USB_RX_LEN;
				USBBufOutPoint = 0;											 //取数据指针复位
				UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_NAK;	   //收到一包数据就NAK，主函数处理完，由主函数修改响应方式
			}
			break;
		case UIS_TOKEN_SETUP | 0:												//SETUP事务
			len = USB_RX_LEN;
			if(len == (sizeof(USB_SETUP_REQ)))
			{
				SetupLen = ((uint16_t)UsbSetupBuf->wLengthH<<8) | (UsbSetupBuf->wLengthL);
				len = 0;													  // 默认为成功并且上传0长度
				SetupReq = UsbSetupBuf->bRequest;
				if ( ( UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )//非标准请求
				{
					switch( SetupReq )
					{
					case GET_LINE_CODING:   //0x21  currently configured
						pDescr = LineCoding;
						len = sizeof(LineCoding);
						len = SetupLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : SetupLen;  // 本次传输长度
						memcpy(Ep0Buffer,pDescr,len);
						SetupLen -= len;
						pDescr += len;
						break;
					case SET_CONTROL_LINE_STATE:  //0x22  generates RS-232/V.24 style control signals
						break;
					case SET_LINE_CODING:	  //0x20  Configure
						break;
					default:
						len = 0xFF;  								 									 /*命令不支持*/
						break;
					}
				}
				else															 //标准请求
				{
					switch(SetupReq)											 //请求码
					{
					case USB_GET_DESCRIPTOR:
						switch(UsbSetupBuf->wValueH)
						{
						case 1:													   //设备描述符
							pDescr = DevDesc;										 //把设备描述符送到要发送的缓冲区
							len = sizeof(DevDesc);
							break;
						case 2:														//配置描述符
							pDescr = CfgDesc;										  //把设备描述符送到要发送的缓冲区
							len = sizeof(CfgDesc);
							break;
						case 3:
							if(UsbSetupBuf->wValueL == 0)
							{
								pDescr = LangDes;
								len = sizeof(LangDes);
							}
							else if(UsbSetupBuf->wValueL == 1)
							{
								pDescr = Manuf_Des;
								len = sizeof(Manuf_Des);
							}
							else if(UsbSetupBuf->wValueL == 2)
							{
								pDescr = Prod_Des;
								len = sizeof(Prod_Des);
							}
							else
							{
								pDescr = SerDes;
								len = sizeof(SerDes);
							}
							break;
						default:
							len = 0xff;												//不支持的命令或者出错
							break;
						}
						if ( SetupLen > len )
						{
							SetupLen = len;	//限制总长度
						}
						len = SetupLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : SetupLen;							//本次传输长度
						memcpy(Ep0Buffer,pDescr,len);								  //加载上传数据
						SetupLen -= len;
						pDescr += len;
						break;
					case USB_SET_ADDRESS:
						SetupLen = UsbSetupBuf->wValueL;							  //暂存USB设备地址
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
					case USB_CLEAR_FEATURE:											//Clear Feature
						if( ( UsbSetupBuf->bRequestType & 0x1F ) == USB_REQ_RECIP_DEVICE )				  /* 清除设备 */
						{
							if( ( ( ( uint16_t )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x01 )
							{
								if( CfgDesc[ 7 ] & 0x20 )
								{
									/* 唤醒 */
								}
								else
								{
									len = 0xFF;										/* 操作失败 */
								}
							}
							else
							{
								len = 0xFF;											/* 操作失败 */
							}
						}
						else if ( ( UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )// 端点
						{
							switch( UsbSetupBuf->wIndexL )
							{
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
								len = 0xFF;										 // 不支持的端点
								break;
							}
						}
						else
						{
							len = 0xFF;												// 不是端点不支持
						}
						break;
					case USB_SET_FEATURE:										  /* Set Feature */
						if( ( UsbSetupBuf->bRequestType & 0x1F ) == USB_REQ_RECIP_DEVICE )				  /* 设置设备 */
						{
							if( ( ( ( uint16_t )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x01 )
							{
								if( CfgDesc[ 7 ] & 0x20 )
								{
									/* 休眠 */
#ifdef DE_PRINTF
									printf( "suspend\n" );															 //睡眠状态
#endif
									while ( XBUS_AUX & bUART0_TX )
									{
										;	//等待发送完成
									}
									SAFE_MOD = 0x55;
									SAFE_MOD = 0xAA;
									WAKE_CTRL = bWAK_BY_USB | bWAK_RXD0_LO | bWAK_RXD1_LO;					  //USB或者RXD0/1有信号时可被唤醒
									PCON |= PD;																 //睡眠
									SAFE_MOD = 0x55;
									SAFE_MOD = 0xAA;
									WAKE_CTRL = 0x00;
								}
								else
								{
									len = 0xFF;										/* 操作失败 */
								}
							}
							else
							{
								len = 0xFF;											/* 操作失败 */
							}
						}
						else if( ( UsbSetupBuf->bRequestType & 0x1F ) == USB_REQ_RECIP_ENDP )			 /* 设置端点 */
						{
							if( ( ( ( uint16_t )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x00 )
							{
								switch( ( ( uint16_t )UsbSetupBuf->wIndexH << 8 ) | UsbSetupBuf->wIndexL )
								{
								case 0x83:
									UEP3_CTRL = UEP3_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* 设置端点3 IN STALL */
									break;
								case 0x03:
									UEP3_CTRL = UEP3_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;/* 设置端点3 OUT Stall */
									break;
								case 0x82:
									UEP2_CTRL = UEP2_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* 设置端点2 IN STALL */
									break;
								case 0x02:
									UEP2_CTRL = UEP2_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;/* 设置端点2 OUT Stall */
									break;
								case 0x81:
									UEP1_CTRL = UEP1_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* 设置端点1 IN STALL */
									break;
								case 0x01:
									UEP1_CTRL = UEP1_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;/* 设置端点1 OUT Stall */
								default:
									len = 0xFF;									/* 操作失败 */
									break;
								}
							}
							else
							{
								len = 0xFF;									  /* 操作失败 */
							}
						}
						else
						{
							len = 0xFF;										  /* 操作失败 */
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
						len = 0xff;													//操作失败
						break;
					}
				}
			}
			else
			{
				len = 0xff;														 //包长度错误
			}
			if(len == 0xff)
			{
				SetupReq = 0xFF;
				UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;//STALL
			}
			else if(len <= DEFAULT_ENDP0_SIZE)													   //上传数据或者状态阶段返回0长度包
			{
				UEP0_T_LEN = len;
				UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//默认数据包是DATA1，返回应答ACK
			}
			else
			{
				UEP0_T_LEN = 0;  //虽然尚未到状态阶段，但是提前预置上传0长度数据包以防主机提前进入状态阶段
				UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//默认数据包是DATA1,返回应答ACK
			}
			break;
		case UIS_TOKEN_IN | 0:													  //endpoint0 IN
			switch(SetupReq)
			{
			case USB_GET_DESCRIPTOR:
				len = SetupLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : SetupLen;								 //本次传输长度
				memcpy( Ep0Buffer, pDescr, len );								   //加载上传数据
				SetupLen -= len;
				pDescr += len;
				UEP0_T_LEN = len;
				UEP0_CTRL ^= bUEP_T_TOG;											 //同步标志位翻转
				break;
			case USB_SET_ADDRESS:
				USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | SetupLen;
				UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
				break;
			default:
				UEP0_T_LEN = 0;													  //状态阶段完成中断或者是强制上传0长度数据包结束控制传输
				UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
				break;
			}
			break;
		case UIS_TOKEN_OUT | 0:  // endpoint0 OUT
			if(SetupReq == SET_LINE_CODING)  //设置串口属性
			{
				if( U_TOG_OK )
				{
					memcpy(LineCoding,UsbSetupBuf,USB_RX_LEN);
					*((uint8_t *)&Baud) = LineCoding[0];
					*((uint8_t *)&Baud+1) = LineCoding[1];
					*((uint8_t *)&Baud+2) = LineCoding[2];
					*((uint8_t *)&Baud+3) = LineCoding[3];
					
					if(Baud > 999999) Baud = 57600;
					
					UEP0_T_LEN = 0;
					UEP0_CTRL |= UEP_R_RES_ACK | UEP_T_RES_ACK;  // 准备上传0包
				}
			}
			else
			{
				UEP0_T_LEN = 0;
				UEP0_CTRL |= UEP_R_RES_ACK | UEP_T_RES_ACK;  // 只要ACK就好了
			}
			break;



		default:
			break;
		}
		UIF_TRANSFER = 0;														   //写0清空中断
	}
	if(UIF_BUS_RST)																 //设备模式USB总线复位中断
	{
#ifdef DE_PRINTF
		printf( "reset\n" );															 //睡眠状态
#endif
		UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
		UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;
		UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;
		USB_DEV_AD = 0x00;
		UIF_SUSPEND = 0;
		UIF_TRANSFER = 0;
		UIF_BUS_RST = 0;															 //清中断标志
		Uart_Input_Point = 0;   //循环缓冲区输入指针
		Uart_Output_Point = 0;  //循环缓冲区读出指针
		UartByteCount = 0;	  //当前缓冲区剩余待取字节数
		USBByteCount = 0;	   //USB端点收到的长度
		UsbConfig = 0;		  //清除配置值
		UpPoint2_Busy = 0;
	}
	if (UIF_SUSPEND)																 //USB总线挂起/唤醒完成
	{
		UIF_SUSPEND = 0;
		if ( USB_MIS_ST & bUMS_SUSPEND )											 //挂起
		{
#ifdef DE_PRINTF
			printf( "suspend\n" );															 //睡眠状态
#endif
			while ( XBUS_AUX & bUART0_TX )
			{
				;	//等待发送完成
			}
			SAFE_MOD = 0x55;
			SAFE_MOD = 0xAA;
			WAKE_CTRL = bWAK_BY_USB | bWAK_RXD0_LO | bWAK_RXD1_LO;					  //USB或者RXD0/1有信号时可被唤醒
			PCON |= PD;																 //睡眠
			SAFE_MOD = 0x55;
			SAFE_MOD = 0xAA;
			WAKE_CTRL = 0x00;
		}
	}
	else {																			 //意外的中断,不可能发生的情况
		USB_INT_FG = 0xFF;															 //清中断标志

	}
}

void virtual_uart_tx(uint8_t tdata)
{
	Receive_Uart_Buf[Uart_Input_Point++] = tdata;
	UartByteCount++;					//当前缓冲区剩余待取字节数
	if(Uart_Input_Point>=UART_REV_LEN)
	{
		Uart_Input_Point = 0;
	}
}

void v_uart_puts(char *str)
{
	while(*str)
		virtual_uart_tx(*(str++));
}

void usb_poll()
{
	uint8_t length;
	static uint8_t Uart_Timeout = 0;
	if(UsbConfig)
	{
		if(UartByteCount)
			Uart_Timeout++;
		if(!UpPoint2_Busy)   //端点不繁忙（空闲后的第一包数据，只用作触发上传）
		{
			length = UartByteCount;
			if(length>0)
			{
				if(length>39 || Uart_Timeout>100)
				{
					Uart_Timeout = 0;
					if(Uart_Output_Point+length>UART_REV_LEN)
						length = UART_REV_LEN-Uart_Output_Point;
					UartByteCount -= length;
					//写上传端点
					memcpy(Ep2Buffer+MAX_PACKET_SIZE,&Receive_Uart_Buf[Uart_Output_Point],length);
					Uart_Output_Point+=length;
					if(Uart_Output_Point>=UART_REV_LEN)
						Uart_Output_Point = 0;
					UEP2_T_LEN = length;													//预使用发送长度一定要清空
					UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;			//应答ACK
					UpPoint2_Busy = 1;
				}
			}
		}
	}
}


void uart_poll()
{/* 串口 处理程序 */
	uint8_t uart_data;
	bool i2c_ack = 0;
//	static bool i2c_status = 0;
	static uint8_t i2c_frame_len = 0;
	static uint8_t i2c_frame_rx_len = 0;
	static uint8_t i2c_error_no = 0;
	static uint8_t uart_rx_status = 0;
	static uint8_t former_data = 0;
	static uint8_t dontstop = 0;
	uint8_t i;
	
	if(USBByteCount)   //USB接收端点有数据
	{
		uart_data = Ep2Buffer[USBBufOutPoint++];
				
		if(uart_rx_status == 0)
		{
			if(uart_data == 'Q')
			{
				v_uart_puts(SI5351_ReferenceClock); /* 26MHz Crystal */
				v_uart_puts("\r\n");
			}
			else if(uart_data == 'V')
			{ /* Version */
				v_uart_puts(Device_Version); /* Device version */
				v_uart_puts("\r\n");
			}
			else if(uart_data == 'E')
			{
				jump_to_bootloader();
			
			}
			else if(uart_data == 'B')
			{
				virtual_uart_tx(Baud / 100000 + '0');
				virtual_uart_tx(Baud % 100000 / 10000 + '0');
				virtual_uart_tx(Baud % 10000 / 1000 + '0');
				virtual_uart_tx(Baud % 1000 / 100 + '0');
				virtual_uart_tx(Baud % 100 / 10 + '0');
				virtual_uart_tx(Baud % 10 / 1 + '0');
				v_uart_puts("\r\n");
			}
			else if(uart_data == 'T' && former_data != 'A') /* BAN AT commands */
			{ /* Transmit I2C Data: T: <LEN>, 16bytes data, performing S, <AR>, <DAT>, E */
				
				i2c_frame_rx_len = 0;
				uart_rx_status = 1;
				i2c_error_no = 0;
				i2c_frame_len = 0;
			}
			else if(uart_data == 'R')
			{ /* Recieve I2C Data: R<AR><LEN> */
				i2c_frame_rx_len = 0;
				uart_rx_status = 3;
				i2c_error_no = 0;
				i2c_frame_len = 0;								
			}
			else if(uart_data == 'T' && former_data == 'A') /* BAN AT commands */
			{
				v_uart_puts("OK\r\n");
			}
			else if(uart_data == 'A')
			{
			}
			else 
			{
				v_uart_puts("NOT SUPPORTED\r\n");
			}
		}
		else if(uart_rx_status == 1)
		{ // 54	03	C0	02	53	
			i2c_frame_len = uart_data & 0x3f; /* 拿到长度 */
			if(uart_data & 0x80)
				dontstop = 1;
			else
				dontstop = 0;
				
			i2c_start();
			uart_rx_status = 2;
		}
		else if(uart_rx_status == 2)
		{
			if(i2c_error_no == 0)
			{
				i2c_write(uart_data);
				i2c_ack = i2c_read_ack();
				if(i2c_ack != 1)
				{
					i2c_stop();
					i2c_error_no = i2c_frame_rx_len + 1;
				}
			}
			i2c_frame_rx_len ++;
				
			if(i2c_frame_len == i2c_frame_rx_len)
			{
				if(i2c_error_no == 0)
				{
					v_uart_puts("OK\r\n");
					if(dontstop == 0)
						i2c_stop(); /* 停止I2C */
				}
				else
				{
					virtual_uart_tx('F');
					virtual_uart_tx(i2c_error_no / 10 + '0');
					virtual_uart_tx(i2c_error_no % 10 + '0'); /* 传输失败 */
					v_uart_puts("\r\n");
				}
				
				i2c_frame_len = 0;
				i2c_frame_rx_len = 0;
				uart_rx_status = 0;
				i2c_error_no = 0;
			}
			
		}
		else if(uart_rx_status == 3)
		{
			i2c_start();
			i2c_write(uart_data); /* 送地址 */
			i2c_ack = i2c_read_ack();
			if(i2c_ack != 1)
			{
				v_uart_puts("FAIL\r\n");
				i2c_stop();
				uart_rx_status = 0;
			}				
			uart_rx_status = 4;
		}
		else if(uart_rx_status == 4)
		{
			i2c_frame_len = uart_data & 0x3f;
			for(i = 0; i < i2c_frame_len; i++)
			{
				virtual_uart_tx(i2c_read());
				i2c_ack = i2c_read_ack();
				if(i2c_ack != 1)
					break;
			}
			i2c_stop();
			uart_rx_status = 0;
		}
		former_data = uart_data;
		
		USBByteCount--;
			
		if(USBByteCount==0)
			UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_ACK;		
	}
}

//主函数
main()
{	
	CfgFsys( );														   //CH559时钟选择配置
	mDelaymS(5);														  //修改主频等待内部晶振稳定,必加
	mInitSTDIO( );														//串口0,可以用于调试
	//UART1Setup( );														//用于CDC

#ifdef DE_PRINTF
	printf("start ...\n");
#endif
	i2c_init();
	
	USBDeviceCfg();
	USBDeviceEndPointCfg();											   //端点配置
	USBDeviceIntCfg();													//中断初始化
	UEP0_T_LEN = 0;
	UEP1_T_LEN = 0;													   //预使用发送长度一定要清空
	UEP2_T_LEN = 0;											   //预使用发送长度一定要清空

	while(1)
	{
		usb_poll();
		uart_poll();
	}
}
