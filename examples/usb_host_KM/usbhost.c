
/********************************** (C) COPYRIGHT *******************************
* File Name          : USBHOST.C
* Author             : WCH
* Version            : V1.1
* Date               : 2018/02/28
* Description        : CH554 USB 主机接口函数
*******************************************************************************/

#include <ch554.h>
#include <debug.h>
#include "stdio.h"
#include "string.h"
#include "usbhost.h"

//#include "ch554ufi.h"                                                        //如果使用USBH_HUB_KM.C 此行屏蔽掉
extern __xdata __at (0x0000) uint8_t  RxBuffer[ MAX_PACKET_SIZE ];
extern __xdata __at (0x0040) uint8_t  TxBuffer[ MAX_PACKET_SIZE ];

#include <ch554_usb.h>

__bit HubLowSpeed;
__xdata uint8_t  Com_Buffer[ COM_BUF_SIZE ];                                            // 定义用户临时缓冲区,枚举时用于处理描述符,枚举结束也可以用作普通临时缓冲区
/*******************************************************************************
* Function Name  : DisableRootHubPort( )
* Description    : 关闭HUB端口
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  DisableRootHubPort( )          
{
#ifdef	FOR_ROOT_UDISK_ONLY
	CH554DiskStatus = DISK_DISCONNECT;
#endif
#ifndef	DISK_BASE_BUF_LEN
	ThisUsbDev.DeviceStatus = ROOT_DEV_DISCONNECT;
    ThisUsbDev.DeviceAddress = 0x00;
#endif
}
/*******************************************************************************
* Function Name  : AnalyzeRootHub(void)
* Description    : 分析ROOT-HUB状态,处理ROOT-HUB端口的设备插拔事件
                   如果设备拔出,函数中调用DisableRootHubPort()函数,将端口关闭,插入事件,置相应端口的状态位
* Input          : None
* Output         : None
* Return         : 返回ERR_SUCCESS为没有情况,返回ERR_USB_CONNECT为检测到新连接,返回ERR_USB_DISCON为检测到断开
*******************************************************************************/
uint8_t   AnalyzeRootHub( void )
{ 
	uint8_t	s;
	s = ERR_SUCCESS;
	if ( USB_MIS_ST & bUMS_DEV_ATTACH ) {                                        // 设备存在
#ifdef DISK_BASE_BUF_LEN
		if ( CH554DiskStatus == DISK_DISCONNECT
#else
		if ( ThisUsbDev.DeviceStatus == ROOT_DEV_DISCONNECT                        // 检测到有设备插入
#endif
			|| ( UHOST_CTRL & bUH_PORT_EN ) == 0x00 ) {                              // 检测到有设备插入,但尚未允许,说明是刚插入
			DisableRootHubPort( );                                                   // 关闭端口
#ifdef DISK_BASE_BUF_LEN
			CH554DiskStatus = DISK_CONNECT;
#else
//		ThisUsbDev.DeviceSpeed = USB_HUB_ST & bUHS_DM_LEVEL ? 0 : 1;
			ThisUsbDev.DeviceStatus = ROOT_DEV_CONNECTED;                            //置连接标志
#endif
#if DE_PRINTF
			printf( "USB dev in\n" );
#endif
			s = ERR_USB_CONNECT;
		}
	}
#ifdef DISK_BASE_BUF_LEN
	else if ( CH554DiskStatus >= DISK_CONNECT ) {
#else
	else if ( ThisUsbDev.DeviceStatus >= ROOT_DEV_CONNECTED ) {                  //检测到设备拔出
#endif
		DisableRootHubPort( );                                                     // 关闭端口
#if DE_PRINTF		
		printf( "USB dev out\n" );
#endif
		if ( s == ERR_SUCCESS ) s = ERR_USB_DISCON;
	}
//	UIF_DETECT = 0;                                                            // 清中断标志
	return( s );
}
/*******************************************************************************
* Function Name  : SetHostUsbAddr
* Description    : 设置USB主机当前操作的USB设备地址
* Input          : uint8_t addr
* Output         : None
* Return         : None
*******************************************************************************/
void    SetHostUsbAddr( uint8_t addr )
{
    USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | addr & 0x7F;
}

#ifndef	FOR_ROOT_UDISK_ONLY
/*******************************************************************************
* Function Name  : SetUsbSpeed
* Description    : 设置当前USB速度
* Input          : uint8_t FullSpeed
* Output         : None
* Return         : None
*******************************************************************************/
void    SetUsbSpeed( uint8_t FullSpeed )  
{
    if ( FullSpeed )                                                           // 全速
    {
        USB_CTRL &= ~ bUC_LOW_SPEED;                                           // 全速
        UH_SETUP &= ~ bUH_PRE_PID_EN;                                          // 禁止PRE PID
    }
    else
    {
        USB_CTRL |= bUC_LOW_SPEED;                                             // 低速		
    }
}
#endif

/*******************************************************************************
* Function Name  : ResetRootHubPort( )
* Description    : 检测到设备后,复位总线,为枚举设备准备,设置为默认为全速
* Input          : None   
* Output         : None
* Return         : None
*******************************************************************************/
void  ResetRootHubPort( )
{
    UsbDevEndp0Size = DEFAULT_ENDP0_SIZE;                                      //USB设备的端点0的最大包尺寸
#ifndef DISK_BASE_BUF_LEN	
    memset( &ThisUsbDev,0,sizeof(ThisUsbDev));                                 //清空结构体
#endif	
	SetHostUsbAddr( 0x00 );
    UHOST_CTRL &= ~bUH_PORT_EN;                                                // 关掉端口
	SetUsbSpeed( 1 );                                                          // 默认为全速
	UHOST_CTRL = UHOST_CTRL & ~ bUH_LOW_SPEED | bUH_BUS_RESET;                 // 默认为全速,开始复位
    mDelaymS( 20 );                                                            // 复位时间10mS到20mS
    UHOST_CTRL = UHOST_CTRL & ~ bUH_BUS_RESET;                                 // 结束复位
    mDelayuS( 250 );
    UIF_DETECT = 0;                                                            // 清中断标志
}
/*******************************************************************************
* Function Name  : EnableRootHubPort( )
* Description    : 使能ROOT-HUB端口,相应的bUH_PORT_EN置1开启端口,设备断开可能导致返回失败
* Input          : None
* Output         : None
* Return         : 返回ERR_SUCCESS为检测到新连接,返回ERR_USB_DISCON为无连接
*******************************************************************************/
uint8_t   EnableRootHubPort( )
{
#ifdef DISK_BASE_BUF_LEN
	if ( CH554DiskStatus < DISK_CONNECT ) CH554DiskStatus = DISK_CONNECT;
#else
	if ( ThisUsbDev.DeviceStatus < ROOT_DEV_CONNECTED ) ThisUsbDev.DeviceStatus = ROOT_DEV_CONNECTED;
#endif
	if ( USB_MIS_ST & bUMS_DEV_ATTACH ) {                                        // 有设备
#ifndef DISK_BASE_BUF_LEN
		if ( ( UHOST_CTRL & bUH_PORT_EN ) == 0x00 ) {                              // 尚未使能
			ThisUsbDev.DeviceSpeed = USB_MIS_ST & bUMS_DM_LEVEL ? 0 : 1;
			if ( ThisUsbDev.DeviceSpeed == 0 ) UHOST_CTRL |= bUH_LOW_SPEED;          // 低速
		}
#endif
		USB_CTRL |= bUC_DMA_EN;                                                    // 启动USB主机及DMA,在中断标志未清除前自动暂停
		UH_SETUP = bUH_SOF_EN;		
		UHOST_CTRL |= bUH_PORT_EN;                                                 //使能HUB端口
		return( ERR_SUCCESS );
	}
	return( ERR_USB_DISCON );
}
#ifndef DISK_BASE_BUF_LEN
/*******************************************************************************
* Function Name  : SelectHubPort( uint8_t HubPortIndex )
* Description    : 选定需要操作的HUB口
* Input          : uint8_t HubPortIndex 选择操作指定的ROOT-HUB端口的外部HUB的指定端口
* Output         : None
* Return         : None
*******************************************************************************/
void    SelectHubPort( uint8_t HubPortIndex )  
{ 
    if( HubPortIndex )                                                         // 选择操作指定的ROOT-HUB端口的外部HUB的指定端口
    {
        SetHostUsbAddr( DevOnHubPort[HubPortIndex-1].DeviceAddress );          // 设置USB主机当前操作的USB设备地址
        SetUsbSpeed( DevOnHubPort[HubPortIndex-1].DeviceSpeed );               // 设置当前USB速度
		if ( DevOnHubPort[HubPortIndex-1].DeviceSpeed == 0 )                   // 通过外部HUB与低速USB设备通讯需要前置ID
        {
            UH_SETUP |= bUH_PRE_PID_EN;                                        // 启用PRE PID
            HubLowSpeed = 1;
			mDelayuS(100);
        }
    }
    else                                                                       
    {
        HubLowSpeed = 0;        			
        SetHostUsbAddr( ThisUsbDev.DeviceAddress );                            // 设置USB主机当前操作的USB设备地址
        SetUsbSpeed( ThisUsbDev.DeviceSpeed );                                 // 设置USB设备的速度
    }
}
#endif
/*******************************************************************************
* Function Name  : WaitUSB_Interrupt
* Description    : 等待USB中断
* Input          : None
* Output         : None
* Return         : 返回ERR_SUCCESS 数据接收或者发送成功
                   ERR_USB_UNKNOWN 数据接收或者发送失败
*******************************************************************************/
uint8_t WaitUSB_Interrupt( void )
{
    uint16_t  i;
    for ( i = WAIT_USB_TOUT_200US; i != 0 && UIF_TRANSFER == 0; i -- ){;}
    return( UIF_TRANSFER ? ERR_SUCCESS : ERR_USB_UNKNOWN );
}
/*******************************************************************************
* Function Name  : USBHostTransact
* Description    : CH554传输事务,输入目的端点地址/PID令牌,同步标志,以20uS为单位的NAK重试总时间(0则不重试,0xFFFF无限重试),返回0成功,超时/出错重试
                   本子程序着重于易理解,而在实际应用中,为了提供运行速度,应该对本子程序代码进行优化
* Input          : uint8_t endp_pid 令牌和地址  endp_pid: 高4位是token_pid令牌, 低4位是端点地址
                   uint8_t tog      同步标志
                   uint16_t timeout 超时时间
* Output         : None
* Return         : ERR_USB_UNKNOWN 超时，可能硬件异常
                   ERR_USB_DISCON  设备断开
                   ERR_USB_CONNECT 设备连接
                   ERR_SUCCESS     传输完成
*******************************************************************************/
uint8_t   USBHostTransact( uint8_t endp_pid, uint8_t tog, uint16_t timeout )
{
//	uint8_t	TransRetry;
#define	TransRetry	UEP0_T_LEN	                                               // 节约内存
	uint8_t	s, r;
	uint16_t	i;
	UH_RX_CTRL = UH_TX_CTRL = tog;
	TransRetry = 0;

	do {
		UH_EP_PID = endp_pid;                                                      // 指定令牌PID和目的端点号
		UIF_TRANSFER = 0;                                                          // 允许传输
//  s = WaitUSB_Interrupt( );
		for ( i = WAIT_USB_TOUT_200US; i != 0 && UIF_TRANSFER == 0; i -- );
		UH_EP_PID = 0x00;                                                          // 停止USB传输
//	if ( s != ERR_SUCCESS ) return( s );  // 中断超时,可能是硬件异常
		if ( UIF_TRANSFER == 0 ) return( ERR_USB_UNKNOWN );
		if ( UIF_DETECT ) {                                                        // USB设备插拔事件
//			mDelayuS( 200 );                                                       // 等待传输完成
			UIF_DETECT = 0;                                                          // 清中断标志
			s = AnalyzeRootHub( );                                                   // 分析ROOT-HUB状态
			if ( s == ERR_USB_CONNECT ) FoundNewDev = 1;
#ifdef DISK_BASE_BUF_LEN
			if ( CH554DiskStatus == DISK_DISCONNECT ) return( ERR_USB_DISCON );      // USB设备断开事件
			if ( CH554DiskStatus == DISK_CONNECT ) return( ERR_USB_CONNECT );        // USB设备连接事件
#else
			if ( ThisUsbDev.DeviceStatus == ROOT_DEV_DISCONNECT ) return( ERR_USB_DISCON );// USB设备断开事件
			if ( ThisUsbDev.DeviceStatus == ROOT_DEV_CONNECTED ) return( ERR_USB_CONNECT );// USB设备连接事件
#endif
			mDelayuS( 200 );  // 等待传输完成
		}
		if ( UIF_TRANSFER ) {  // 传输完成
			if ( U_TOG_OK ) return( ERR_SUCCESS );
			r = USB_INT_ST & MASK_UIS_H_RES;  // USB设备应答状态
			if ( r == USB_PID_STALL ) return( r | ERR_USB_TRANSFER );
			if ( r == USB_PID_NAK ) {
				if ( timeout == 0 ) return( r | ERR_USB_TRANSFER );
				if ( timeout < 0xFFFF ) timeout --;
				-- TransRetry;
			}
			else switch ( endp_pid >> 4 ) {
				case USB_PID_SETUP:
				case USB_PID_OUT:
//					if ( U_TOG_OK ) return( ERR_SUCCESS );
//					if ( r == USB_PID_ACK ) return( ERR_SUCCESS );
//					if ( r == USB_PID_STALL || r == USB_PID_NAK ) return( r | ERR_USB_TRANSFER );
					if ( r ) return( r | ERR_USB_TRANSFER );  // 不是超时/出错,意外应答
					break;  // 超时重试
				case USB_PID_IN:
//					if ( U_TOG_OK ) return( ERR_SUCCESS );
//					if ( tog ? r == USB_PID_DATA1 : r == USB_PID_DATA0 ) return( ERR_SUCCESS );
//					if ( r == USB_PID_STALL || r == USB_PID_NAK ) return( r | ERR_USB_TRANSFER );
					if ( r == USB_PID_DATA0 && r == USB_PID_DATA1 ) {  // 不同步则需丢弃后重试
					}  // 不同步重试
					else if ( r ) return( r | ERR_USB_TRANSFER );  // 不是超时/出错,意外应答
					break;  // 超时重试
				default:
					return( ERR_USB_UNKNOWN );  // 不可能的情况
					break;
			}
		}
		else {  // 其它中断,不应该发生的情况
			USB_INT_FG = 0xFF;  /* 清中断标志 */
		}
		mDelayuS( 15 );	
	} while ( ++ TransRetry < 3 );
	return( ERR_USB_TRANSFER );  // 应答超时
}
/*******************************************************************************
* Function Name  : HostCtrlTransfer
* Description    : 执行控制传输,8字节请求码在pSetupReq中,DataBuf为可选的收发缓冲区
* Input          : P__xdata uint8_t DataBuf 如果需要接收和发送数据,那么DataBuf需指向有效缓冲区用于存放后续数据
                   Puint8_t RetLen  实际成功收发的总长度保存在RetLen指向的字节变量中
* Output         : None
* Return         : ERR_USB_BUF_OVER IN状态阶段出错
                   ERR_SUCCESS     数据交换成功
                   其他错误状态
*******************************************************************************/
uint8_t HostCtrlTransfer( __xdata uint8_t *DataBuf, uint8_t *RetLen )  
{
    uint16_t  RemLen  = 0;
    uint8_t   s, RxLen, RxCnt, TxCnt;
    __xdata uint8_t  *pBuf;
    uint8_t  *pLen;
    pBuf = DataBuf;
    pLen = RetLen;
    mDelayuS( 200 );
    if ( pLen )
    {
        *pLen = 0;                                                              // 实际成功收发的总长度
    }
    UH_TX_LEN = sizeof( USB_SETUP_REQ );
    s = USBHostTransact( (uint8_t)(USB_PID_SETUP << 4 | 0x00), 0x00, 10000 );          // SETUP阶段,200mS超时
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
    UH_RX_CTRL = UH_TX_CTRL = bUH_R_TOG | bUH_R_AUTO_TOG | bUH_T_TOG | bUH_T_AUTO_TOG;// 默认DATA1
    UH_TX_LEN = 0x01;                                                           // 默认无数据故状态阶段为IN
    RemLen = (pSetupReq -> wLengthH << 8)|( pSetupReq -> wLengthL);
    if ( RemLen && pBuf )                                                       // 需要收发数据
    {
        if ( pSetupReq -> bRequestType & USB_REQ_TYP_IN )                       // 收
        {
            while ( RemLen )
            {
                mDelayuS( 200 );
                s = USBHostTransact( (uint8_t)(USB_PID_IN << 4 | 0x00), UH_RX_CTRL, 200000/20 );// IN数据
                if ( s != ERR_SUCCESS )
                {
                    return( s );
                }
                RxLen = USB_RX_LEN < RemLen ? USB_RX_LEN : RemLen;
                RemLen -= RxLen;
                if ( pLen )
                {
                    *pLen += RxLen;                                              // 实际成功收发的总长度
                }
//              memcpy( pBuf, RxBuffer, RxLen );
//              pBuf += RxLen;
                for ( RxCnt = 0; RxCnt != RxLen; RxCnt ++ )
                {
                    *pBuf = RxBuffer[ RxCnt ];
                    pBuf ++;
                }
                if ( USB_RX_LEN == 0 || ( USB_RX_LEN & ( UsbDevEndp0Size - 1 ) ) )
                {
                    break;                                                       // 短包
                }
            }
            UH_TX_LEN = 0x00;                                                    // 状态阶段为OUT
        }
        else                                                                     // 发
        {
            while ( RemLen )
            {
                mDelayuS( 200 );
                UH_TX_LEN = RemLen >= UsbDevEndp0Size ? UsbDevEndp0Size : RemLen;
//              memcpy( TxBuffer, pBuf, UH_TX_LEN );
//              pBuf += UH_TX_LEN;
#ifndef DISK_BASE_BUF_LEN
                if(pBuf[1] == 0x09)                                              //HID类命令处理
                {
                    Set_Port = Set_Port^1;
                    *pBuf = Set_Port;
#if DE_PRINTF									
                    printf("SET_PORT  %02X  %02X ",(uint16_t)(*pBuf),(uint16_t)(Set_Port));
#endif									
                }
#endif
                for ( TxCnt = 0; TxCnt != UH_TX_LEN; TxCnt ++ )
                {
                    TxBuffer[ TxCnt ] = *pBuf;
                    pBuf ++;
                }
                s = USBHostTransact( USB_PID_OUT << 4 | 0x00, UH_TX_CTRL, 200000/20 );// OUT数据
                if ( s != ERR_SUCCESS )
                {
                    return( s );
                }
                RemLen -= UH_TX_LEN;
                if ( pLen )
                {
                    *pLen += UH_TX_LEN;                                           // 实际成功收发的总长度
                }
            }
//          UH_TX_LEN = 0x01;                                                     // 状态阶段为IN
        }
    }
    mDelayuS( 200 );
    s = USBHostTransact( ( UH_TX_LEN ? USB_PID_IN << 4 | 0x00: USB_PID_OUT << 4 | 0x00 ), bUH_R_TOG | bUH_T_TOG, 200000/20 );  // STATUS阶段
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
    if ( UH_TX_LEN == 0 )
    {
        return( ERR_SUCCESS );                                                    // 状态OUT
    }
    if ( USB_RX_LEN == 0 )
    {
        return( ERR_SUCCESS );                                                    // 状态IN,检查IN状态返回数据长度
    }
    return( ERR_USB_BUF_OVER );                                                   // IN状态阶段错误
}
/*******************************************************************************
* Function Name  : CopySetupReqPkg
* Description    : 复制控制传输的请求包
* Input          : P__code uint8_t pReqPkt 控制请求包地址
* Output         : None
* Return         : None
*******************************************************************************/
void CopySetupReqPkg( __code uint8_t *pReqPkt )                                        // 复制控制传输的请求包
{
    uint8_t   i;
    if(HubLowSpeed)                                                               //HUB下低速设备
    {
		((__xdata uint8_t *)pSetupReq)[ 0 ] = *pReqPkt;			
		for ( i = 1; i != sizeof( USB_SETUP_REQ )+1; i ++ )
		{
			((__xdata uint8_t *)pSetupReq)[ i ] = *pReqPkt;
			pReqPkt++;
		}
	}
    if(HubLowSpeed == 0)
    {
		for ( i = 0; i != sizeof( USB_SETUP_REQ ); i ++ )
		{
			((__xdata uint8_t *)pSetupReq)[ i ] = *pReqPkt;
			pReqPkt++;
		}			
    }
}
/*******************************************************************************
* Function Name  : CtrlGetDeviceDescr
* Description    : 获取设备描述符,返回在TxBuffer中
* Input          : None
* Output         : None
* Return         : ERR_USB_BUF_OVER 描述符长度错误
                   ERR_SUCCESS      成功
                   其他
*******************************************************************************/
uint8_t   CtrlGetDeviceDescr( void )  
{
    uint8_t   s;
    uint8_t   len;
    UsbDevEndp0Size = DEFAULT_ENDP0_SIZE;
    CopySetupReqPkg( SetupGetDevDescr );
    s = HostCtrlTransfer( Com_Buffer, (uint8_t *)&len );                                      // 执行控制传输
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
    UsbDevEndp0Size = ( (PXUSB_DEV_DESCR)Com_Buffer ) -> bMaxPacketSize0;          // 端点0最大包长度,这是简化处理,正常应该先获取前8字节后立即更新UsbDevEndp0Size再继续
    if ( len < ( (PUSB_SETUP_REQ)SetupGetDevDescr ) -> wLengthL )
    {
        return( ERR_USB_BUF_OVER );                                              // 描述符长度错误
    }
    return( ERR_SUCCESS );
}
/*******************************************************************************
* Function Name  : CtrlGetConfigDescr
* Description    : 获取配置描述符,返回在TxBuffer中
* Input          : None
* Output         : None
* Return         : ERR_USB_BUF_OVER 描述符长度错误
                   ERR_SUCCESS      成功
                   其他
*******************************************************************************/
uint8_t CtrlGetConfigDescr( void )
{
    uint8_t   s,len;
    CopySetupReqPkg( SetupGetCfgDescr );
    s = HostCtrlTransfer( Com_Buffer, (uint8_t *)&len );                                      // 执行控制传输
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }

    len = ( (PXUSB_CFG_DESCR)Com_Buffer ) -> wTotalLengthL;
    CopySetupReqPkg( SetupGetCfgDescr );
    if(HubLowSpeed)                                                                //HUB下低速设备
    {
      pSetupReq -> wLengthH = len;                                                 // 完整配置描述符的总长度
    }
    if(HubLowSpeed == 0)                                                          
    {
      pSetupReq -> wLengthL = len;                                                 // 完整配置描述符的总长度
    }
    s = HostCtrlTransfer( Com_Buffer, (uint8_t *)&len );                                // 执行控制传输
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
#ifdef DISK_BASE_BUF_LEN
	if(len>64) len = 64;
	for(s=0;s!=len;s++)                                                             //U盘操作时，需要拷贝到TxBuffer
		TxBuffer[s]=Com_Buffer[s];                                       
#endif
    return( ERR_SUCCESS );
}
/*******************************************************************************
* Function Name  : CtrlSetUsbAddress
* Description    : 设置USB设备地址
* Input          : uint8_t addr 设备地址
* Output         : None
* Return         : ERR_SUCCESS      成功
                   其他
*******************************************************************************/
uint8_t CtrlSetUsbAddress( uint8_t addr ) 
{
    uint8_t   s;
    CopySetupReqPkg( SetupSetUsbAddr );
    if(HubLowSpeed)                                                               //HUB下低速设备
    {		
      pSetupReq -> wValueH = addr;                                                // USB设备地址
    }
    if(HubLowSpeed == 0)                                                          
    {		
      pSetupReq -> wValueL = addr;                                                // USB设备地址
    }		
    s = HostCtrlTransfer( NULL, NULL );                                         // 执行控制传输
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
    SetHostUsbAddr( addr );                                                     // 设置USB主机当前操作的USB设备地址
    mDelaymS( 10 );                                                             // 等待USB设备完成操作
    return( ERR_SUCCESS );
}
/*******************************************************************************
* Function Name  : CtrlSetUsbConfig
* Description    : 设置USB设备配置
* Input          : uint8_t cfg       配置值
* Output         : None
* Return         : ERR_SUCCESS      成功
                   其他
*******************************************************************************/
uint8_t   CtrlSetUsbConfig( uint8_t cfg )                   
{
    CopySetupReqPkg( SetupSetUsbConfig );
    if(HubLowSpeed)                                                               //HUB下低速设备
    {		
      pSetupReq -> wValueH = cfg;                                                // USB设备配置
    }
    if(HubLowSpeed == 0)                                                          
    {		
      pSetupReq -> wValueL = cfg;                                                // USB设备配置
    }		
    return( HostCtrlTransfer( NULL, NULL ) );                                  // 执行控制传输
}
/*******************************************************************************
* Function Name  : CtrlClearEndpStall
* Description    : 清除端点STALL
* Input          : uint8_t endp       端点地址
* Output         : None
* Return         : ERR_SUCCESS      成功
                   其他
*******************************************************************************/
uint8_t   CtrlClearEndpStall( uint8_t endp )  
{
    CopySetupReqPkg( SetupClrEndpStall );                                      // 清除端点的错误
    if(HubLowSpeed)                                                               //HUB下低速设备
    {		
      pSetupReq -> wIndexH = endp;                                               // 端点地址
    }
    if(HubLowSpeed == 0)                                                          
    {		
      pSetupReq -> wIndexL = endp;                                               // 端点地址
    }	
    return( HostCtrlTransfer( NULL, NULL ) );                                  // 执行控制传输
}

#ifndef DISK_BASE_BUF_LEN
/*******************************************************************************
* Function Name  : CtrlSetUsbIntercace
* Description    : 设置USB设备接口
* Input          : uint8_t cfg       配置值
* Output         : None
* Return         : ERR_SUCCESS      成功
                   其他
*******************************************************************************/
uint8_t   CtrlSetUsbIntercace( uint8_t cfg )                   
{
    CopySetupReqPkg( SetupSetUsbInterface );
    if(HubLowSpeed)                                                               //HUB下低速设备
    {		
      pSetupReq -> wValueH = cfg;                                                 // USB设备配置
    }
    if(HubLowSpeed == 0)                                                          
    {		
      pSetupReq -> wValueL = cfg;                                                 // USB设备配置
    }		
    return( HostCtrlTransfer( NULL, NULL ) );                             // 执行控制传输
}

/*******************************************************************************
* Function Name  : CtrlGetHIDDeviceReport
* Description    : 获取HID设备报表描述符,返回在TxBuffer中
* Input          : None
* Output         : None
* Return         : ERR_SUCCESS 成功
                   其他        错误
*******************************************************************************/
uint8_t   CtrlGetHIDDeviceReport( uint8_t infc )  
{
    uint8_t   s;
    uint8_t   len;

	CopySetupReqPkg( SetupSetHIDIdle );
    if(HubLowSpeed)                                                               //HUB下低速设备
    {	
		TxBuffer[5] = infc;
    }
    else                                                         
    {		
		TxBuffer[4] = infc;
    }	
    s = HostCtrlTransfer( Com_Buffer, (uint8_t *)&len );                                    // 执行控制传输
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }	

	CopySetupReqPkg( SetupGetHIDDevReport );
    if(HubLowSpeed)                                                               //HUB下低速设备
    {	
		TxBuffer[5] = infc;
    }
    else                                                        
    {		
		TxBuffer[4] = infc;
    }	
    s = HostCtrlTransfer( Com_Buffer, (uint8_t *)&len );                                    // 执行控制传输
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }

    return( ERR_SUCCESS );
}
/*******************************************************************************
* Function Name  : CtrlGetHubDescr
* Description    : 获取HUB描述符,返回在TxBuffer中
* Input          : None
* Output         : None
* Return         : ERR_SUCCESS 成功
                   ERR_USB_BUF_OVER 长度错误
*******************************************************************************/
uint8_t   CtrlGetHubDescr( void )  
{
    uint8_t   s;
    uint8_t  len;
    CopySetupReqPkg( SetupGetHubDescr );
    s = HostCtrlTransfer( Com_Buffer, (uint8_t *)&len );                                    // 执行控制传输
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
    if ( len < ( (PUSB_SETUP_REQ)SetupGetHubDescr ) -> wLengthL )
    {
        return( ERR_USB_BUF_OVER );                                            // 描述符长度错误
    }
//  if ( len < 4 ) return( ERR_USB_BUF_OVER );                                 // 描述符长度错误
    return( ERR_SUCCESS );
}
/*******************************************************************************
* Function Name  : HubGetPortStatus
* Description    : 查询HUB端口状态,返回在TxBuffer中
* Input          : uint8_t HubPortIndex 
* Output         : None
* Return         : ERR_SUCCESS 成功
                   ERR_USB_BUF_OVER 长度错误
*******************************************************************************/
uint8_t   HubGetPortStatus( uint8_t HubPortIndex )   
{
    uint8_t   s;
    uint8_t  len;
    pSetupReq -> bRequestType = HUB_GET_PORT_STATUS;
    pSetupReq -> bRequest = HUB_GET_STATUS;
    pSetupReq -> wValueL = 0x00;
    pSetupReq -> wValueH = 0x00;
    pSetupReq -> wIndexL = HubPortIndex;
    pSetupReq -> wIndexH = 0x00;
    pSetupReq -> wLengthL = 0x04;
    pSetupReq -> wLengthH = 0x00;
    s = HostCtrlTransfer( Com_Buffer, (uint8_t *)&len );                           // 执行控制传输
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
    if ( len < 4 )
    {
        return( ERR_USB_BUF_OVER );                                            // 描述符长度错误
    }
    return( ERR_SUCCESS );
}
/*******************************************************************************
* Function Name  : HubSetPortFeature
* Description    : 设置HUB端口特性
* Input          : uint8_t HubPortIndex    //HUB端口
                   uint8_t FeatureSelt     //HUB端口特性
* Output         : None
* Return         : ERR_SUCCESS 成功
                   其他        错误
*******************************************************************************/
uint8_t   HubSetPortFeature( uint8_t HubPortIndex, uint8_t FeatureSelt ) 
{
    pSetupReq -> bRequestType = HUB_SET_PORT_FEATURE;
    pSetupReq -> bRequest = HUB_SET_FEATURE;
    pSetupReq -> wValueL = FeatureSelt;
    pSetupReq -> wValueH = 0x00;
    pSetupReq -> wIndexL = HubPortIndex;
    pSetupReq -> wIndexH = 0x00;
    pSetupReq -> wLengthL = 0x00;
    pSetupReq -> wLengthH = 0x00;
    return( HostCtrlTransfer( NULL, NULL ) );                                 // 执行控制传输
}
/*******************************************************************************
* Function Name  : HubClearPortFeature
* Description    : 清除HUB端口特性
* Input          : uint8_t HubPortIndex                                         //HUB端口
                   uint8_t FeatureSelt                                          //HUB端口特性
* Output         : None
* Return         : ERR_SUCCESS 成功
                   其他        错误
*******************************************************************************/
uint8_t   HubClearPortFeature( uint8_t HubPortIndex, uint8_t FeatureSelt ) 
{
    pSetupReq -> bRequestType = HUB_CLEAR_PORT_FEATURE;
    pSetupReq -> bRequest = HUB_CLEAR_FEATURE;
    pSetupReq -> wValueL = FeatureSelt;
    pSetupReq -> wValueH = 0x00;
    pSetupReq -> wIndexL = HubPortIndex;
    pSetupReq -> wIndexH = 0x00;
    pSetupReq -> wLengthL = 0x00;
    pSetupReq -> wLengthH = 0x00;
    return( HostCtrlTransfer( NULL, NULL ) );                                // 执行控制传输
}
/*******************************************************************************
* Function Name  : CtrlGetXPrinterReport1
* Description    : 打印机类命令
* Input          : None
* Output         : None
* Return         : ERR_USB_BUF_OVER 描述符长度错误
                   ERR_SUCCESS      成功
                   其他
*******************************************************************************/
uint8_t   CtrlGetXPrinterReport1( void )  
{
    uint8_t   s;
    uint16_t   len;
    CopySetupReqPkg( XPrinterReport );
    s = HostCtrlTransfer( Com_Buffer, (uint8_t *)&len );                         // 执行控制传输
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
//     if ( len < ( (XPrinterReport[7]<<8)|(XPrinterReport[6]) ))
//     {
//         return( ERR_USB_BUF_OVER );                                         // 描述符长度错误
//     }
    return( ERR_SUCCESS );
}

/*******************************************************************************
* Function Name  : AnalyzeHidIntEndp
* Description    : 从描述符中分析出HID中断端点的地址,如果HubPortIndex是0保存到ROOTHUB，如果是非零值则保存到HUB下结构体
* Input          : P__xdata uint8_t buf ： 待分析数据缓冲区地址 HubPortIndex：0表示根HUB，非0表示外部HUB下的端口号
* Output         : None
* Return         : 端点数
*******************************************************************************/
uint8_t   AnalyzeHidIntEndp( __xdata uint8_t *buf, uint8_t HubPortIndex ) 
{
    uint8_t   i, s, l;
    s = 0;

	if(HubPortIndex)
		memset( DevOnHubPort[HubPortIndex-1].GpVar,0,sizeof(DevOnHubPort[HubPortIndex-1].GpVar) ); //清空数组
	else
		memset( ThisUsbDev.GpVar,0,sizeof(ThisUsbDev.GpVar) );                     //清空数组

    for ( i = 0; i < ( (PXUSB_CFG_DESCR)buf ) -> wTotalLengthL; i += l )       // 搜索中断端点描述符,跳过配置描述符和接口描述符
    {
        if ( ( (PXUSB_ENDP_DESCR)(buf+i) ) -> bDescriptorType == USB_DESCR_TYP_ENDP  // 是端点描述符
                && ( ( (PXUSB_ENDP_DESCR)(buf+i) ) -> bmAttributes & USB_ENDP_TYPE_MASK ) == USB_ENDP_TYPE_INTER// 是中断端点
                && ( ( (PXUSB_ENDP_DESCR)(buf+i) ) -> bEndpointAddress & USB_ENDP_DIR_MASK ) )// 是IN端点
        {           // 保存中断端点的地址,位7用于同步标志位,清0
            if(HubPortIndex)
				DevOnHubPort[HubPortIndex-1].GpVar[s] = ( (PXUSB_ENDP_DESCR)(buf+i) ) -> bEndpointAddress & USB_ENDP_ADDR_MASK;
			else
				ThisUsbDev.GpVar[s] = ( (PXUSB_ENDP_DESCR)(buf+i) ) -> bEndpointAddress & USB_ENDP_ADDR_MASK;// 中断端点的地址，可以根据需要保存wMaxPacketSize和bInterval                                                          
#if DE_PRINTF			
			printf("%02x ",(uint16_t)ThisUsbDev.GpVar[s]);
#endif
			s++;
			if(s >= 4) break;	//只分析4个端点

		}
        l = ( (PXUSB_ENDP_DESCR)(buf+i) ) -> bLength;                          // 当前描述符长度,跳过
        if ( l > 16 )
        {
            break;
        }
    }
#if DE_PRINTF
	printf("\n");
#endif	
    return( s );
}

/*******************************************************************************
* Function Name  : AnalyzeBulkEndp
* Description    : 分析出批量端点,GpVar[0]、GpVar[1]存放上传端点。GpVar[2]、GpVar[3]存放下传端点
* Input          : buf：待分析数据缓冲区地址   HubPortIndex：0表示根HUB，非0表示外部HUB下的端口号
* Output         : None
* Return         : 0
*******************************************************************************/
uint8_t   AnalyzeBulkEndp( __xdata uint8_t *buf, uint8_t HubPortIndex ) 
{
    uint8_t   i, s1,s2, l;
    s1 = 0;s2 = 2;

	if(HubPortIndex)
		memset( DevOnHubPort[HubPortIndex-1].GpVar,0,sizeof(DevOnHubPort[HubPortIndex-1].GpVar) ); //清空数组
	else
		memset( ThisUsbDev.GpVar,0,sizeof(ThisUsbDev.GpVar) );                     //清空数组

    for ( i = 0; i < ( (PXUSB_CFG_DESCR)buf ) -> wTotalLengthL; i += l )       // 搜索中断端点描述符,跳过配置描述符和接口描述符
    {
        if ( (( (PXUSB_ENDP_DESCR)(buf+i) ) -> bDescriptorType == USB_DESCR_TYP_ENDP)     // 是端点描述符
                && ((( (PXUSB_ENDP_DESCR)(buf+i) ) -> bmAttributes & USB_ENDP_TYPE_MASK ) == USB_ENDP_TYPE_BULK))  // 是中断端点

        {
            if(HubPortIndex)
			{
				if(( (PXUSB_ENDP_DESCR)(buf+i) ) -> bEndpointAddress & USB_ENDP_DIR_MASK )
					DevOnHubPort[HubPortIndex-1].GpVar[s1++] = ( (PXUSB_ENDP_DESCR)(buf+i) ) -> bEndpointAddress & USB_ENDP_ADDR_MASK;
				else
					DevOnHubPort[HubPortIndex-1].GpVar[s2++] = ( (PXUSB_ENDP_DESCR)(buf+i) ) -> bEndpointAddress & USB_ENDP_ADDR_MASK;
			}
			else
			{
				if(( (PXUSB_ENDP_DESCR)(buf+i) ) -> bEndpointAddress & USB_ENDP_DIR_MASK )
					ThisUsbDev.GpVar[s1++] = ( (PXUSB_ENDP_DESCR)(buf+i) ) -> bEndpointAddress & USB_ENDP_ADDR_MASK;
				else
					ThisUsbDev.GpVar[s2++] = ( (PXUSB_ENDP_DESCR)(buf+i) ) -> bEndpointAddress & USB_ENDP_ADDR_MASK;
			}

			if(s1 == 2) s1 = 1;
			if(s2 == 4) s2 = 3;			
		}
        l = ( (PXUSB_ENDP_DESCR)(buf+i) ) -> bLength;                          // 当前描述符长度,跳过
        if ( l > 16 )
        {
            break;
        }
    }
    return( 0 );
}

//尝试启动AOA模式
uint8_t TouchStartAOA(void)
{
	uint8_t len,s,i,Num;
	uint16_t cp_len;
    //获取协议版本号
    CopySetupReqPkg( GetProtocol );
    s = HostCtrlTransfer( Com_Buffer, &len );  // 执行控制传输
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
	if(Com_Buffer[0]<2) return  ERR_AOA_PROTOCOL;

    //输出字符串
    for(i=0; i<6; i++)
    {
        Num=Sendlen[i];
        CopySetupReqPkg(&SetStringID[8*i]);
		cp_len = (pSetupReq -> wLengthH << 8)|( pSetupReq -> wLengthL);
		memcpy(Com_Buffer,&StringID[Num],cp_len);
        s = HostCtrlTransfer( Com_Buffer, &len );  // 执行控制传输
        if ( s != ERR_SUCCESS )
        {
            return( s );
        }
    }	

    CopySetupReqPkg(TouchAOAMode);
    s = HostCtrlTransfer( Com_Buffer, &len );  // 执行控制传输
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
    return ERR_SUCCESS;	
}

/*******************************************************************************
* Function Name  : InitRootDevice
* Description    : 初始化指定ROOT-HUB端口的USB设备
* Input          : uint8_t RootHubIndex 指定端口，内置HUB端口号0/1
* Output         : None
* Return         :
*******************************************************************************/
uint8_t InitRootDevice( void ) 
{
    uint8_t   t,i, s, cfg, dv_cls, if_cls,ifc;
	uint8_t touchaoatm = 0;
    t = 0;
#if DE_PRINTF	
    printf( "Reset USB Port\n");
#endif
USBDevEnum:
    for(i=0;i<t;i++)
    {
        mDelaymS( 100 );	
        if(t>10) return( s );			
    }
    ResetRootHubPort( );                                                    // 检测到设备后,复位相应端口的USB总线
    for ( i = 0, s = 0; i < 100; i ++ )                                     // 等待USB设备复位后重新连接,100mS超时
    {
        mDelaymS( 1 );
        if ( EnableRootHubPort( ) == ERR_SUCCESS )                          // 使能ROOT-HUB端口
        {
            i = 0;
            s ++;                                                           // 计时等待USB设备连接后稳定
            if ( s > (20+t) )
            {
                break;                                                      // 已经稳定连接15mS
            }
        }
    }	
    if ( i )                                                                 // 复位后设备没有连接
    {
        DisableRootHubPort( );
#if DE_PRINTF
        printf( "Disable usb port because of disconnect\n" );
#endif	
//         return( ERR_USB_DISCON );
    }
    SelectHubPort( 0 );
#if DE_PRINTF		
    printf( "GetDevDescr: " );
#endif
    s = CtrlGetDeviceDescr( );                                               // 获取设备描述符
    if ( s == ERR_SUCCESS )
    {
#if DE_PRINTF	
        for ( i = 0; i < ( (PUSB_SETUP_REQ)SetupGetDevDescr ) -> wLengthL; i ++ )
        {
            printf( "x%02X ", (uint16_t)( Com_Buffer[i] ) );				
        }
        printf( "\n" );                                                       // 显示出描述符
#endif	
		ThisUsbDev.DeviceVID = (((uint16_t)((PXUSB_DEV_DESCR)Com_Buffer)->idVendorH)<<8 ) + ((PXUSB_DEV_DESCR)Com_Buffer)->idVendorL; //保存VID PID信息
		ThisUsbDev.DevicePID = (((uint16_t)((PXUSB_DEV_DESCR)Com_Buffer)->idProductH)<<8 ) + ((PXUSB_DEV_DESCR)Com_Buffer)->idProductL;
        dv_cls = ( (PXUSB_DEV_DESCR)Com_Buffer ) -> bDeviceClass;               // 设备类代码			
        s = CtrlSetUsbAddress( ( (PUSB_SETUP_REQ)SetupSetUsbAddr ) -> wValueL );// 设置USB设备地址,加上RootHubIndex可以保证2个HUB端口分配不同的地址
        if ( s == ERR_SUCCESS )
        {
            ThisUsbDev.DeviceAddress = ( (PUSB_SETUP_REQ)SetupSetUsbAddr ) -> wValueL;  // 保存USB地址
#if DE_PRINTF						
            printf( "GetCfgDescr: " );
#endif					
            s = CtrlGetConfigDescr( );                                        // 获取配置描述符
            if ( s == ERR_SUCCESS )
            {
                cfg = ( (PXUSB_CFG_DESCR)Com_Buffer ) -> bConfigurationValue;
                ifc = ( (PXUSB_CFG_DESCR)Com_Buffer ) -> bNumInterfaces;					
#if DE_PRINTF							
                for ( i = 0; i < ( (PXUSB_CFG_DESCR)Com_Buffer ) -> wTotalLengthL; i ++ )
                {
                    printf( "x%02X ", (uint16_t)( Com_Buffer[i] ) );
                }
                printf("\n");
#endif								
                                                                              //分析配置描述符,获取端点数据/各端点地址/各端点大小等,更新变量endp_addr和endp_size等
                if_cls = ( (PXUSB_CFG_DESCR_LONG)Com_Buffer ) -> itf_descr.bInterfaceClass;  // 接口类代码								
                if ( dv_cls == 0x00 && if_cls == USB_DEV_CLASS_STORAGE )      // 是USB存储类设备,基本上确认是U盘
                {
					AnalyzeBulkEndp(Com_Buffer , 0 );
#if DE_PRINTF
					for(i=0;i!=4;i++)
					{
						printf("%02x ",(uint16_t)ThisUsbDev.GpVar[i] );
					}
					printf("\n");
#endif					
                    s = CtrlSetUsbConfig( cfg );                              // 设置USB设备配置
                    if ( s == ERR_SUCCESS )
                    {
                        ThisUsbDev.DeviceStatus = ROOT_DEV_SUCCESS;
                        ThisUsbDev.DeviceType = USB_DEV_CLASS_STORAGE;
#if DE_PRINTF												
                        printf( "USB-Disk Ready\n" );
#endif											
                        SetUsbSpeed( 1 );                                     // 默认为全速
                        return( ERR_SUCCESS );
                    }
                }
                else if ( dv_cls == 0x00 && if_cls == USB_DEV_CLASS_PRINTER && ( (PXUSB_CFG_DESCR_LONG)Com_Buffer ) -> itf_descr.bInterfaceSubClass == 0x01 )// 是打印机类设备
                {
#if DE_PRINTF										
                    printf( "USB-Print OK\n" );
#endif									
                    if((Com_Buffer[19] == 5)&&(Com_Buffer[20]&&0x80)){
                       ThisUsbDev.GpVar[0] = Com_Buffer[20];                     //IN 端点											
                    }
                    else if((Com_Buffer[19] == 5)&&((Com_Buffer[20]&&0x80) == 0)){
                       ThisUsbDev.GpVar[1] = Com_Buffer[20];                    //OUT 端点												
                    }		
                    if((Com_Buffer[26] == 5)&&(Com_Buffer[20]&&0x80)){
                       ThisUsbDev.GpVar[0] = Com_Buffer[27];                     //IN 端点											
                    }
                    else if((Com_Buffer[26] == 5)&&((Com_Buffer[20]&&0x80) == 0)){
                       ThisUsbDev.GpVar[1] = Com_Buffer[27];                    //OUT 端点												
                    }										
//                  ThisUsbDev.GpVar = ( (PUSB_CFG_DESCR_LONG)Com_Buffer ) -> endp_descr[0].bEndpointAddress;// 保存批量传输端点
                    s = CtrlSetUsbConfig( cfg );                            // 设置USB设备配置
                    if ( s == ERR_SUCCESS )
                    {									
                        s = CtrlSetUsbIntercace(cfg);
//                         if(s == ERR_SUCCESS){
                                                                           //需保存端点信息以便主程序进行USB传输
                         s = CtrlGetXPrinterReport1( );                    //打印机类命令
                         if(s == ERR_SUCCESS){													 
                           ThisUsbDev.DeviceStatus = ROOT_DEV_SUCCESS;
                           ThisUsbDev.DeviceType = USB_DEV_CLASS_PRINTER;
#if DE_PRINTF														 
                           printf( "USB-Print Ready\n" );
#endif													 
                           SetUsbSpeed( 1 );                               // 默认为全速
                           return( ERR_SUCCESS );
						 }													 
//                         }
                    }
                }
                else if ( (dv_cls == 0x00) && (if_cls == USB_DEV_CLASS_HID) && (( (PXUSB_CFG_DESCR_LONG)Com_Buffer ) -> itf_descr.bInterfaceSubClass <= 0x01) )// 是HID类设备,键盘/鼠标等
                { 									
                    s = AnalyzeHidIntEndp( Com_Buffer,0 );                    // 从描述符中分析出HID中断端点的地址								
#if DE_PRINTF														 
                    printf( "AnalyzeHidIntEndp %02x\n",(uint16_t)s );
#endif		                    
					if_cls = ( (PXUSB_CFG_DESCR_LONG)Com_Buffer ) -> itf_descr.bInterfaceProtocol;
#if DE_PRINTF														 
                    printf( "CtrlSetUsbConfig %02x\n",(uint16_t)cfg );
#endif		
                    s = CtrlSetUsbConfig( cfg );                          // 设置USB设备配置								
                    if ( s == ERR_SUCCESS )
                    {
#if DE_PRINTF												
                        printf( "GetHIDReport: " );
#endif			
                        for(dv_cls=0;dv_cls<ifc;dv_cls++)
                        {											
							s = CtrlGetHIDDeviceReport(dv_cls);                    //获取报表描述符
							if(s == ERR_SUCCESS)
							{
#if DE_PRINTF														
								for ( i = 0; i < 64; i++ )
								{
									printf( "x%02X ", (uint16_t)( Com_Buffer[i] ) );
								}
								printf("\n");
#endif														
							}
						}
                        //Set_Idle( );
                                                                         //需保存端点信息以便主程序进行USB传输
                        ThisUsbDev.DeviceStatus = ROOT_DEV_SUCCESS;
                        if ( if_cls == 1 )
                        {
                            ThisUsbDev.DeviceType = DEV_TYPE_KEYBOARD;
                                                                         //进一步初始化,例如设备键盘指示灯LED等
                            if(ifc > 1)
                            {
#if DE_PRINTF														
								printf( "USB_DEV_CLASS_HID Ready\n" );
#endif																
								ThisUsbDev.DeviceType = USB_DEV_CLASS_HID;//复合HID设备															
                            }																												
#if DE_PRINTF														
                            printf( "USB-Keyboard Ready\n" );
#endif													
                            SetUsbSpeed( 1 );                            // 默认为全速

                            return( ERR_SUCCESS );
                        }
                        else if ( if_cls == 2 )
                        {
                            ThisUsbDev.DeviceType = DEV_TYPE_MOUSE;
                                                                         //为了以后查询鼠标状态,应该分析描述符,取得中断端口的地址,长度等信息
                            if(ifc > 1)
                            {
#if DE_PRINTF														
								printf( "USB_DEV_CLASS_HID Ready\n" );
#endif																
								ThisUsbDev.DeviceType = USB_DEV_CLASS_HID;//复合HID设备															
                            }															
#if DE_PRINTF													
                            printf( "USB-Mouse Ready\n" );
#endif													
                            SetUsbSpeed( 1 );                            // 默认为全速

                            return( ERR_SUCCESS );
                        }
                        s = ERR_USB_UNSUPPORT;
                    }
                }
                else if ( dv_cls == USB_DEV_CLASS_HUB )                   // 是HUB类设备,集线器等
                {
                    s = AnalyzeHidIntEndp( Com_Buffer,0 );                    // 从描述符中分析出HID中断端点的地址
#if DE_PRINTF		
                    printf( "AnalyzeHidIntEndp %02x\n",(uint16_t)s );
#endif	                   			
#if DE_PRINTF										
                    printf( "GetHubDescr: ");
#endif									
                    s = CtrlGetHubDescr( );
                    if ( s == ERR_SUCCESS )
                    {
#if DE_PRINTF												
                        for( i = 0; i < Com_Buffer[0]; i++ )
                        {
                            printf( "x%02X ",(uint16_t)(Com_Buffer[i]) );
                        }                       
						printf("\n");
#endif												
                        ThisUsbDev.GpHUBPortNum = ( (PXUSB_HUB_DESCR)Com_Buffer ) -> bNbrPorts;// 保存HUB的端口数量
                        if ( ThisUsbDev.GpHUBPortNum > HUB_MAX_PORTS )
                        {
                            ThisUsbDev.GpHUBPortNum = HUB_MAX_PORTS;// 因为定义结构DevOnHubPort时人为假定每个HUB不超过HUB_MAX_PORTS个端口
                        }
                        //if ( ( (PXUSB_HUB_DESCR)Com_Buffer ) -> wHubCharacteristics[0] & 0x04 ) printf("带有集线器的复合设备\n");
                        //else printf("单一的集线器产品\n");
                        s = CtrlSetUsbConfig( cfg );                     // 设置USB设备配置
                        if ( s == ERR_SUCCESS )
                        {
                            ThisUsbDev.DeviceStatus = ROOT_DEV_SUCCESS;
                            ThisUsbDev.DeviceType = USB_DEV_CLASS_HUB;
                            //需保存端点信息以便主程序进行USB传输,本来中断端点可用于HUB事件通知,但本程序使用查询状态控制传输代替
                            //给HUB各端口上电,查询各端口状态,初始化有设备连接的HUB端口,初始化设备
                            for ( i = 1; i <= ThisUsbDev.GpHUBPortNum; i ++ )// 给HUB各端口都上电
                            {
                                DevOnHubPort[i-1].DeviceStatus = ROOT_DEV_DISCONNECT;  // 清外部HUB端口上设备的状态
                                s = HubSetPortFeature( i, HUB_PORT_POWER );
                                if ( s != ERR_SUCCESS )
                                {
#if DE_PRINTF																		
                                    printf( "Ext-HUB Port_%1d# power on error\n",(uint16_t)i );// 端口上电失败
#endif																	
                                }
                            }
//							for ( i = 1; i <= ThisUsbDev.GpVar[0]; i ++ )            // 查询HUB各端口连接状态
//							{
//								s = HubGetPortStatus( i );                           // 获取端口状态
//								if ( s != ERR_SUCCESS ) 
//#if DE_PRINTF																	
//								printf( "Ext-HUB Port_%1d#	clear connection error\n",(uint16_t)i );	// 端口连接状态清除失败
//#endif															
//							}														
                            SetUsbSpeed( 1 );                                        // 默认为全速
                            return( ERR_SUCCESS );
                        }
                    }
                }
                else                                                                 //其他设备
                {			
#if DE_PRINTF														 
                    printf( "dv_cls %02x\n",(uint16_t)dv_cls );
                    printf( "if_cls %02x\n",(uint16_t)if_cls );
                    printf( "if_subcls %02x\n",(uint16_t)( (PXUSB_CFG_DESCR_LONG)Com_Buffer ) -> itf_descr.bInterfaceSubClass );									
#endif				
					AnalyzeBulkEndp(Com_Buffer , 0 );                                  //分析出批量端点
#if DE_PRINTF
					for(i=0;i!=4;i++)
					{
						printf("%02x ",(uint16_t)ThisUsbDev.GpVar[i] );
					}
					printf("\n");
#endif
                    s = CtrlSetUsbConfig( cfg );                                     // 设置USB设备配置
                    if ( s == ERR_SUCCESS ) 
                    {
#if DE_PRINTF						
						printf("%02x %02x\n",(uint16_t)ThisUsbDev.DeviceVID,(uint16_t)ThisUsbDev.DevicePID);
#endif						
						if((ThisUsbDev.DeviceVID==0x18D1)&&(ThisUsbDev.DevicePID&0xff00)==0x2D00)   //如果是AOA配件
						{
							printf("AOA Mode\n");
							ThisUsbDev.DeviceStatus = ROOT_DEV_SUCCESS;
							ThisUsbDev.DeviceType = DEF_AOA_DEVICE;                      //这只是自定义的变量类，不属于USB协议类
							SetUsbSpeed( 1 );                                            // 默认为全速
							return( ERR_SUCCESS );
						}
						else   //如果不是AOA 配件模式，尝试启动配件模式.
						{
							s = TouchStartAOA();
							if( s == ERR_SUCCESS ) 
							{
								if(touchaoatm<3)         //尝试AOA启动次数限制
								{
									touchaoatm++;
									mDelaymS(500);      //部分安卓设备自动断开重连，所以此处最好有延时
									goto USBDevEnum;    //其实这里可以不用跳转，AOA协议规定，设备会自动重新接入总线。
								}
								//执行到这，说明可能不支持AOA，或是其他设备
								ThisUsbDev.DeviceType = dv_cls ? dv_cls : if_cls;
								ThisUsbDev.DeviceStatus = ROOT_DEV_SUCCESS;
								SetUsbSpeed( 1 );                                            // 默认为全速
								return( ERR_SUCCESS );                                       // 未知设备初始化成功									
							}							
						}
                    }
                }
            }
        }
    }
#if DE_PRINTF			
    printf( "InitRootDev Err = %02X\n", (uint16_t)s );
#endif		
    ThisUsbDev.DeviceStatus = ROOT_DEV_FAILED;
    SetUsbSpeed( 1 );                                                                 // 默认为全速
    t++;
    goto USBDevEnum;		
}
/*******************************************************************************
* Function Name  : EnumAllRootDevice
* Description    : 枚举所有ROOT-HUB端口的USB设备
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t   EnumAllRootDevice( void )   
{
    __idata uint8_t   s;
#if DE_PRINTF	
    printf( "EnumUSBDev\n" );
#endif
	if ( ThisUsbDev.DeviceStatus == ROOT_DEV_CONNECTED )                        // 刚插入设备尚未初始化
	{
		s = InitRootDevice( );                                      // 初始化/枚举指定HUB端口的USB设备
		if ( s != ERR_SUCCESS )
		{
			return( s );
		}
	}
    return( ERR_SUCCESS );
}
/*******************************************************************************
* Function Name  : InitDevOnHub
* Description    : 初始化枚举外部HUB后的二级USB设备
* Input          : uint8_t HubPortIndex  指定外部HUB
* Output         : None
* Return         : ERR_SUCCESS 成功
                   ERR_USB_UNKNOWN 未知设备                 
*******************************************************************************/
uint8_t InitDevOnHub( uint8_t HubPortIndex ) 
{
    uint8_t   i, s, cfg, dv_cls, if_cls;
    uint8_t   ifc;
#if DE_PRINTF		
    printf( "Init dev @ExtHub-port_%1d ", (uint16_t)HubPortIndex );
#endif
    if ( HubPortIndex == 0 )
    {
        return( ERR_USB_UNKNOWN );
    }
    SelectHubPort( HubPortIndex );                                      // 选择操作指定的ROOT-HUB端口的外部HUB的指定端口,选择速度
#if DE_PRINTF		
    printf( "GetDevDescr: " );
#endif
    s = CtrlGetDeviceDescr( );                                          // 获取设备描述符
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
	DevOnHubPort[HubPortIndex-1].DeviceVID = (((uint16_t)((PXUSB_DEV_DESCR)Com_Buffer)->idVendorH)<<8 ) + ((PXUSB_DEV_DESCR)Com_Buffer)->idVendorL; //保存VID PID信息
	DevOnHubPort[HubPortIndex-1].DevicePID = (((uint16_t)((PXUSB_DEV_DESCR)Com_Buffer)->idProductH)<<8 ) + ((PXUSB_DEV_DESCR)Com_Buffer)->idProductL;

    dv_cls = ( (PXUSB_DEV_DESCR)Com_Buffer ) -> bDeviceClass;             // 设备类代码
    cfg = ( 1<<4 ) + HubPortIndex;                                      // 计算出一个USB地址,避免地址重叠
    s = CtrlSetUsbAddress( cfg );                                       // 设置USB设备地址
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
    DevOnHubPort[HubPortIndex-1].DeviceAddress = cfg;                   // 保存分配的USB地址
#if DE_PRINTF			
    printf( "GetCfgDescr: " );
#endif
    s = CtrlGetConfigDescr( );                                          // 获取配置描述符
    if ( s != ERR_SUCCESS )
    {
        return( s );
    }
    cfg = ( (PXUSB_CFG_DESCR)Com_Buffer ) -> bConfigurationValue;
#if DE_PRINTF			
    for ( i = 0; i < ( (PXUSB_CFG_DESCR)Com_Buffer ) -> wTotalLengthL; i ++ )
    {
        printf( "x%02X ", (uint16_t)( Com_Buffer[i] ) );
    }
    printf("\n");
#endif		
    /* 分析配置描述符,获取端点数据/各端点地址/各端点大小等,更新变量endp_addr和endp_size等 */
    if_cls = ( (PXUSB_CFG_DESCR_LONG)Com_Buffer ) -> itf_descr.bInterfaceClass; // 接口类代码
    if ( dv_cls == 0x00 && if_cls == USB_DEV_CLASS_STORAGE )                  // 是USB存储类设备,基本上确认是U盘
    {
		AnalyzeBulkEndp(Com_Buffer , HubPortIndex );
#if DE_PRINTF
		for(i=0;i!=4;i++)
		{
			printf("%02x ",(uint16_t)DevOnHubPort[HubPortIndex-1].GpVar[i] );
		}
		printf("\n");
#endif
        s = CtrlSetUsbConfig( cfg );                                          // 设置USB设备配置
        if ( s == ERR_SUCCESS )
        {
            DevOnHubPort[HubPortIndex-1].DeviceStatus = ROOT_DEV_SUCCESS;
            DevOnHubPort[HubPortIndex-1].DeviceType = USB_DEV_CLASS_STORAGE;
#if DE_PRINTF						
            printf( "USB-Disk Ready\n" );
#endif					
            SetUsbSpeed( 1 );                                                 // 默认为全速
            return( ERR_SUCCESS );
        }
    }
    else if ( (dv_cls == 0x00) && (if_cls == USB_DEV_CLASS_HID) && (( (PXUSB_CFG_DESCR_LONG)Com_Buffer ) -> itf_descr.bInterfaceSubClass <= 0x01) )    // 是HID类设备,键盘/鼠标等
    {
        ifc = ( (PXUSB_CFG_DESCR_LONG)Com_Buffer ) -> cfg_descr.bNumInterfaces;			
        s = AnalyzeHidIntEndp( Com_Buffer, HubPortIndex);                                     // 从描述符中分析出HID中断端点的地址
#if DE_PRINTF														 
        printf( "AnalyzeHidIntEndp %02x\n",(uint16_t)s );
#endif	
        if_cls = ( (PXUSB_CFG_DESCR_LONG)Com_Buffer ) -> itf_descr.bInterfaceProtocol;
        s = CtrlSetUsbConfig( cfg );                                           // 设置USB设备配置
        if ( s == ERR_SUCCESS )
        {
			for(dv_cls=0;dv_cls<ifc;dv_cls++)
			{											
				s = CtrlGetHIDDeviceReport(dv_cls);                    //获取报表描述符
				if(s == ERR_SUCCESS)
				{
#if DE_PRINTF														
					for ( i = 0; i < 64; i++ )
					{
						printf( "x%02X ", (uint16_t)( Com_Buffer[i] ) );
					}
					printf("\n");
#endif														
				}
			}					
            //需保存端点信息以便主程序进行USB传输
            DevOnHubPort[HubPortIndex-1].DeviceStatus = ROOT_DEV_SUCCESS;
			if ( if_cls == 1 )
			{
				DevOnHubPort[HubPortIndex-1].DeviceType = DEV_TYPE_KEYBOARD;
															 //进一步初始化,例如设备键盘指示灯LED等
				if(ifc > 1)
				{
#if DE_PRINTF														
					printf( "USB_DEV_CLASS_HID Ready\n" );
#endif																
					DevOnHubPort[HubPortIndex-1].DeviceType = USB_DEV_CLASS_HID;//复合HID设备															
				}																												
#if DE_PRINTF														
				printf( "USB-Keyboard Ready\n" );
#endif													
				SetUsbSpeed( 1 );                            // 默认为全速

				return( ERR_SUCCESS );
			}
			else if ( if_cls == 2 )
			{
				DevOnHubPort[HubPortIndex-1].DeviceType = DEV_TYPE_MOUSE;
															 //为了以后查询鼠标状态,应该分析描述符,取得中断端口的地址,长度等信息
				if(ifc > 1)
				{
#if DE_PRINTF														
					printf( "USB_DEV_CLASS_HID Ready\n" );
#endif																
					DevOnHubPort[HubPortIndex-1].DeviceType = USB_DEV_CLASS_HID;//复合HID设备															
				}															
#if DE_PRINTF													
				printf( "USB-Mouse Ready\n" );
#endif													
				SetUsbSpeed( 1 );                            // 默认为全速

				return( ERR_SUCCESS );
			}
			s = ERR_USB_UNSUPPORT;			
        }
    }
    else if ( dv_cls == USB_DEV_CLASS_HUB )                                     // 是HUB类设备,集线器等
    {
        DevOnHubPort[HubPortIndex-1].DeviceType = USB_DEV_CLASS_HUB;
#if DE_PRINTF				
        printf( "This program don't support Level 2 HUB\n");                    // 需要支持多级HUB级联请参考本程序进行扩展
#endif		
        s = HubClearPortFeature( i, HUB_PORT_ENABLE );                          // 禁止HUB端口
        if ( s != ERR_SUCCESS )
        {
            return( s );
        }
        s = ERR_USB_UNSUPPORT;
    }
	else                                                                //其他设备
	{
		AnalyzeBulkEndp(Com_Buffer , HubPortIndex );                      //分析出批量端点
#if DE_PRINTF
		for(i=0;i!=4;i++)
		{
			printf("%02x ",(uint16_t)DevOnHubPort[HubPortIndex-1].GpVar[i] );
		}
		printf("\n");
#endif	
		s = CtrlSetUsbConfig( cfg );                                     // 设置USB设备配置
		if ( s == ERR_SUCCESS ) 
		{
			//需保存端点信息以便主程序进行USB传输
			DevOnHubPort[HubPortIndex-1].DeviceStatus = ROOT_DEV_SUCCESS;
			DevOnHubPort[HubPortIndex-1].DeviceType = dv_cls ? dv_cls : if_cls;
            SetUsbSpeed( 1 );                                                    // 默认为全速
            return( ERR_SUCCESS );                                               //未知设备初始化成功
		}		
	}
#if DE_PRINTF			
    printf( "InitDevOnHub Err = %02X\n", (uint16_t)s );
#endif		
    DevOnHubPort[HubPortIndex-1].DeviceStatus = ROOT_DEV_FAILED;
    SetUsbSpeed( 1 );                                                            // 默认为全速
    return( s );
}
/*******************************************************************************
* Function Name  : EnumHubPort
* Description    : 枚举指定ROOT-HUB端口上的外部HUB集线器的各个端口,检查各端口有无连接或移除事件并初始化二级USB设备
* Input          : uint8_t RootHubIndex ROOT_HUB0和ROOT_HUB1
* Output         : None
* Return         : ERR_SUCCESS 成功
                   其他        失败
*******************************************************************************/
uint8_t EnumHubPort( ) 
{
    uint8_t   i, s;

    for ( i = 1; i <= ThisUsbDev.GpHUBPortNum; i ++ )                                       // 查询集线器的端口是否有变化
    {
        SelectHubPort( 0 );                                                          // 选择操作指定的ROOT-HUB端口,设置当前USB速度以及被操作设备的USB地址
        s = HubGetPortStatus( i );                                                   // 获取端口状态
        if ( s != ERR_SUCCESS )
        {
            return( s );                                                              // 可能是该HUB断开了
        }
        if ( (( Com_Buffer[0]&(1<<(HUB_PORT_CONNECTION&0x07)) ) && ( Com_Buffer[2]&(1<<(HUB_C_PORT_CONNECTION&0x07)) ))||(Com_Buffer[2] == 0x10) ) 
        {                                                                            // 发现有设备连接
			DevOnHubPort[i-1].DeviceStatus = ROOT_DEV_CONNECTED;                     // 有设备连接
            DevOnHubPort[i-1].DeviceAddress = 0x00;
            s = HubGetPortStatus( i );                                               // 获取端口状态
            if ( s != ERR_SUCCESS )
            {
                return( s );                                                         // 可能是该HUB断开了
            }
            DevOnHubPort[i-1].DeviceSpeed = Com_Buffer[1] & (1<<(HUB_PORT_LOW_SPEED&0x07)) ? 0 : 1;// 低速还是全速
            if ( DevOnHubPort[i-1].DeviceSpeed )
            {
#if DE_PRINTF								
                printf( "Found full speed device on port %1d\n", (uint16_t)i );
#endif							
            }
            else
            {
#if DE_PRINTF								
                printf( "Found low speed device on port %1d\n", (uint16_t)i );
#endif							
            }
            mDelaymS( 200 );                                                          // 等待设备上电稳定
            s = HubSetPortFeature( i, HUB_PORT_RESET );                               // 对有设备连接的端口复位
            if ( s != ERR_SUCCESS )
            {
                return( s );                                                          // 可能是该HUB断开了
            }
#if DE_PRINTF							
            printf( "Reset port and then wait in\n" );
#endif						
            do                                                                        // 查询复位端口,直到复位完成,把完成后的状态显示出来
            {
                mDelaymS( 1 );
                s = HubGetPortStatus( i );
                if ( s != ERR_SUCCESS )
                {
                    return( s );                                                      // 可能是该HUB断开了
                }
            }
            while ( Com_Buffer[0] & (1<<(HUB_PORT_RESET&0x07)) );                       // 端口正在复位则等待
            mDelaymS( 100 );
            s = HubClearPortFeature( i, HUB_C_PORT_RESET );                           // 清除复位完成标志
//             s = HubSetPortFeature( i, HUB_PORT_ENABLE );                              // 启用HUB端口
            s = HubClearPortFeature( i, HUB_C_PORT_CONNECTION );                      // 清除连接或移除变化标志
            if ( s != ERR_SUCCESS )
            {
                return( s );
            }
            s = HubGetPortStatus( i );                                                // 再读取状态,复查设备是否还在
            if ( s != ERR_SUCCESS )
            {
                return( s );
            }
            if ( ( Com_Buffer[0]&(1<<(HUB_PORT_CONNECTION&0x07)) ) == 0 )
            {
                DevOnHubPort[i-1].DeviceStatus = ROOT_DEV_DISCONNECT;                 // 设备不在了
            }
            s = InitDevOnHub( i );                                                    // 初始化二级USB设备
            if ( s != ERR_SUCCESS )
            {
                return( s );
            }
            SetUsbSpeed( 1 );                                                         // 默认为全速
        }
		else if (Com_Buffer[2]&(1<<(HUB_C_PORT_ENABLE&0x07))  )                         // 设备连接出错
		{
			HubClearPortFeature( i, HUB_C_PORT_ENABLE );                              // 清除连接错误标志
#if DE_PRINTF						
			printf( "Device on port error\n" );		
#endif					
			s = HubSetPortFeature( i, HUB_PORT_RESET );                               // 对有设备连接的端口复位
			if ( s != ERR_SUCCESS ) 
			return( s );                                                              // 可能是该HUB断开了
			do                                                                        // 查询复位端口,直到复位完成,把完成后的状态显示出来
			{
				mDelaymS( 1 );
				s = HubGetPortStatus( i );
				if ( s != ERR_SUCCESS ) return( s );                                    // 可能是该HUB断开了
			} while ( Com_Buffer[0] & (1<<(HUB_PORT_RESET&0x07)) );                     // 端口正在复位则等待
		}
        else if ( ( Com_Buffer[0]&(1<<(HUB_PORT_CONNECTION&0x07)) ) == 0 )              // 设备已经断开
        {
            if ( DevOnHubPort[i-1].DeviceStatus >= ROOT_DEV_CONNECTED )
            {
#if DE_PRINTF								
                printf( "Device on port %1d removed\n", (uint16_t)i );
#endif							
            }
            DevOnHubPort[i-1].DeviceStatus = ROOT_DEV_DISCONNECT;                     // 有设备连接
            if ( Com_Buffer[2]&(1<<(HUB_C_PORT_CONNECTION&0x07)) )
            {
                HubClearPortFeature( i, HUB_C_PORT_CONNECTION );                      // 清除移除变化标志
            }
        }
    }
    return( ERR_SUCCESS );                                                            // 返回操作成功
}
/*******************************************************************************
* Function Name  : EnumAllHubPort
* Description    : 枚举所有ROOT-HUB端口下外部HUB后的二级USB设备
* Input          : None
* Output         : None
* Return         : ERR_SUCCESS 成功
                   其他        失败
*******************************************************************************/
uint8_t   EnumAllHubPort( void ) 
{
    uint8_t   s;

	if ( (ThisUsbDev.DeviceStatus >= ROOT_DEV_SUCCESS) && (ThisUsbDev.DeviceType == USB_DEV_CLASS_HUB) )// HUB枚举成功
	{ 
		SelectHubPort( 0 );                                                        // 选择操作指定的ROOT-HUB端口,设置当前USB速度以及被操作设备的USB地址
		//做点什么?  给HUB各端口上电,查询各端口状态,初始化有设备连接的HUB端口,初始化设备
//             for ( i = 1; i <= ThisUsbDev.GpVar; i ++ ){                             // 初始化HUB各端口
//               s = HubSetPortFeature( i, HUB_PORT_POWER );                           // 给HUB各端口上电
//               if ( s != ERR_SUCCESS )
//               {
//                 return( s );                                                        // 可能是该HUB断开了
//               }             							
//             }				
		s = EnumHubPort( );                                                        // 枚举指定ROOT-HUB端口上的外部HUB集线器的各个端口,检查各端口有无连接或移除事件
		if ( s != ERR_SUCCESS )                                                    // 可能是HUB断开了
		{
#if DE_PRINTF								
			printf( "EnumAllHubPort err = %02X\n", (uint16_t)s );
#endif							
		}
		SetUsbSpeed( 1 );                                                          // 默认为全速
	}
    return( ERR_SUCCESS );
}
/*******************************************************************************
* Function Name  : SearchTypeDevice
* Description    : 在ROOT-HUB以及外部HUB各端口上搜索指定类型的设备所在的端口号,输出端口号为0xFFFF则未搜索到
* Input          : uint8_t type 搜索的设备类型
* Output         : None
* Return         : 输出高8位为ROOT-HUB端口号,低8位为外部HUB的端口号,低8位为0则设备直接在ROOT-HUB端口上
                   当然也可以根据USB的厂商VID产品PID进行搜索(事先要记录各设备的VID和PID),以及指定搜索序号
*******************************************************************************/
uint16_t  SearchTypeDevice( uint8_t type )   
{
	uint8_t  RootHubIndex;                                                          //CH554只有一个USB口,RootHubIndex = 0,只需看返回值的低八位即可
    uint8_t  HubPortIndex;

	RootHubIndex = 0;
	if ( (ThisUsbDev.DeviceType == USB_DEV_CLASS_HUB) && (ThisUsbDev.DeviceStatus >= ROOT_DEV_SUCCESS) )// 外部集线器HUB且枚举成功
	{
		for ( HubPortIndex = 1; HubPortIndex <= ThisUsbDev.GpHUBPortNum; HubPortIndex ++ )// 搜索外部HUB的各个端口
		{
			if ( DevOnHubPort[HubPortIndex-1].DeviceType == type && DevOnHubPort[HubPortIndex-1].DeviceStatus >= ROOT_DEV_SUCCESS )
			{
				return( ( (uint16_t)RootHubIndex << 8 ) | HubPortIndex );           // 类型匹配且枚举成功
			}
		}
	}
	if ( (ThisUsbDev.DeviceType == type) && (ThisUsbDev.DeviceStatus >= ROOT_DEV_SUCCESS) )
	{
		return( (uint16_t)RootHubIndex << 8 );                                      // 类型匹配且枚举成功,在ROOT-HUB端口上
	} 

    return( 0xFFFF );
}
/*******************************************************************************
* Function Name  : SETorOFFNumLock
* Description    : NumLock的点灯判断
* Input          : Puint8_t buf 点灯键值
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t SETorOFFNumLock(uint8_t *buf)
{
    uint8_t tmp[]= {0x21,0x09,0x00,0x02,0x00,0x00,0x01,0x00};
    uint8_t len,s;
    if((buf[2]==0x53)&(buf[0]|buf[1]|buf[3]|buf[4]|buf[5]|buf[6]|buf[7]==0))
    {			
        if(HubLowSpeed)                                                               //HUB下低速设备
        {	
			((__xdata uint8_t *)pSetupReq)[ 0 ] = 0X21;					
			for ( s = 1; s != sizeof( tmp )+1; s ++ )
			{
				((__xdata uint8_t *)pSetupReq)[ s ] = tmp[s];
			}
        }
        if(HubLowSpeed == 0)                                                          
        {		
			for ( s = 0; s != sizeof( tmp ); s ++ )
			{
				((__xdata uint8_t *)pSetupReq)[ s ] = tmp[s];
			}
        }	
        s = HostCtrlTransfer( Com_Buffer, &len );                                     // 执行控制传输
        if ( s != ERR_SUCCESS )
        {
            return( s );
        }
    }
    return( ERR_SUCCESS );
}
#endif


#ifdef DISK_BASE_BUF_LEN
uint8_t	InitRootDevice( void )                                                       // 初始化USB设备
{
	uint8_t	i, s, cfg, dv_cls, if_cls;
#if DE_PRINTF	
	printf( "Reset host port\n" );
#endif
	ResetRootHubPort( );                                                            // 检测到设备后,复位相应端口的USB总线
	for ( i = 0, s = 0; i < 100; i ++ ) {                                           // 等待USB设备复位后重新连接,100mS超时
		mDelaymS( 1 );
		if ( EnableRootHubPort( ) == ERR_SUCCESS ) {                                  // 使能端口
			i = 0;
			s ++;                                                                       // 计时等待USB设备连接后稳定
			if ( s > 100 ) break;                                                       // 已经稳定连接100mS
		}
	}
	if ( i ) {                                                                      // 复位后设备没有连接
		DisableRootHubPort( );
#if DE_PRINTF			
		printf( "Disable host port because of disconnect\n" );
#endif		
		return( ERR_USB_DISCON );
	}
	SetUsbSpeed( 1 );                                                              // 设置当前USB速度
	s = CtrlGetDeviceDescr( );                                                     // 获取设备描述符
	if ( s == ERR_SUCCESS ) {
#if DE_PRINTF			
	  printf( "GetDevDescr: " );		
		for ( i = 0; i < ( (PUSB_SETUP_REQ)SetupGetDevDescr ) -> wLengthL; i ++ ) printf( "x%02X ", (uint16_t)( Com_Buffer[i] ) );
		printf( "\n" );                                                             // 显示出描述符
#endif
		dv_cls = ( (PXUSB_DEV_DESCR)Com_Buffer ) -> bDeviceClass;                     // 设备类代码
		s = CtrlSetUsbAddress( ( (PUSB_SETUP_REQ)SetupSetUsbAddr ) -> wValueL );    // 设置USB设备地址
		if ( s == ERR_SUCCESS ) {
			s = CtrlGetConfigDescr( );                                                // 获取配置描述符
			if ( s == ERR_SUCCESS ) {
				cfg = ( (PXUSB_CFG_DESCR)Com_Buffer ) -> bConfigurationValue;
#if DE_PRINTF					
			  printf( "GetCfgDescr: " );				
				for ( i = 0; i < ( (PXUSB_CFG_DESCR)Com_Buffer ) -> wTotalLengthL; i ++ ) 
					printf( "x%02X ", (uint16_t)( Com_Buffer[i] ) );
				printf("\n");
#endif				
/* 分析配置描述符,获取端点数据/各端点地址/各端点大小等,更新变量endp_addr和endp_size等 */
				if_cls = ( (PXUSB_CFG_DESCR_LONG)Com_Buffer ) -> itf_descr.bInterfaceClass; // 接口类代码
				if ( (dv_cls == 0x00) && (if_cls == USB_DEV_CLASS_STORAGE) ) {            // 是USB存储类设备,基本上确认是U盘					
//					s = CtrlSetUsbConfig( cfg );                                     // 设置USB设备配置
//                    if ( s == ERR_SUCCESS )
					{
						CH554DiskStatus = DISK_USB_ADDR;
						return( ERR_SUCCESS );
					}
//					else return( ERR_USB_UNSUPPORT );
				}
				else {
					return( ERR_USB_UNSUPPORT );
				}
			}
		}
	}
#if DE_PRINTF		
	printf( "InitRootDev Err = %02X\n", (uint16_t)s );
#endif
	CH554DiskStatus = DISK_CONNECT;
	SetUsbSpeed( 1 );                                                              // 默认为全速
	return( s );
}
#endif
/*******************************************************************************
* Function Name  : InitUSB_Host
* Description    : 初始化USB主机
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  InitUSB_Host( void )
{
    uint8_t   i;
    IE_USB = 0;
//  LED_CFG = 1;
//  LED_RUN = 0;
    USB_CTRL = bUC_HOST_MODE;                                                    // 先设定模式
    UHOST_CTRL &= ~bUH_PD_DIS;                                                   //启用主机下拉
    USB_DEV_AD = 0x00;
    UH_EP_MOD = bUH_EP_TX_EN | bUH_EP_RX_EN ;
    UH_RX_DMA = (uint16_t)RxBuffer;
    UH_TX_DMA = (uint16_t)TxBuffer;
    UH_RX_CTRL = 0x00;
    UH_TX_CTRL = 0x00;
    USB_CTRL = bUC_HOST_MODE | bUC_INT_BUSY;// | bUC_DMA_EN;                     // 启动USB主机及DMA,在中断标志未清除前自动暂停
//  UHUB0_CTRL = 0x00;
//  UHUB1_CTRL = 0x00;
//  UH_SETUP = bUH_SOF_EN;
    USB_INT_FG = 0xFF;                                                           // 清中断标志
    for ( i = 0; i != 2; i ++ )
    {
        DisableRootHubPort( );                                                   // 清空
    }
    USB_INT_EN = bUIE_TRANSFER | bUIE_DETECT;
//  IE_USB = 1;                                                                  // 查询方式
}

