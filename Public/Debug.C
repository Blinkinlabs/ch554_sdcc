/********************************** (C) COPYRIGHT *******************************
* File Name          : Debug.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/01/20
* Description        : CH554 DEBUG Interface
                     CH554主频修改、延时函数定义
                     串口0和串口1初始化
                     串口0和串口1的收发子函数
                     看门狗初始化										 
*******************************************************************************/

#include "CH554.H"
#include "Debug.H"

/*******************************************************************************
* Function Name  : CfgFsys( )
* Description    : CH554时钟选择和配置函数,默认使用Fsys 6MHz，FREQ_SYS可以通过
                   CLOCK_CFG配置得到，公式如下：
                   Fsys = (Fosc * 4/(CLOCK_CFG & MASK_SYS_CK_SEL);具体时钟需要自己配置 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/ 
void	CfgFsys( )  
{
// 		SAFE_MOD = 0x55;
// 		SAFE_MOD = 0xAA;
//     CLOCK_CFG |= bOSC_EN_XT;                          //使能外部晶振
//     CLOCK_CFG &= ~bOSC_EN_INT;                        //关闭内部晶振    
		SAFE_MOD = 0x55;
		SAFE_MOD = 0xAA;
// 		CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x07;  // 32MHz	
// 		CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x06;  // 24MHz	
// 		CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x05;  // 16MHz	
		CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x04;  // 12MHz
// 		CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x03;  // 6MHz	
// 		CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x02;  // 3MHz	
// 		CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x01;  // 750KHz	
// 		CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x00;  // 187.5MHz		
		SAFE_MOD = 0x00;
}

/*******************************************************************************
* Function Name  : mDelayus(UNIT16 n)
* Description    : us延时函数
* Input          : UNIT16 n
* Output         : None
* Return         : None
*******************************************************************************/ 
void	mDelayuS( UINT16 n )  // 以uS为单位延时
{
#ifdef	FREQ_SYS
#if		FREQ_SYS <= 6000000
		n >>= 2;
#endif
#if		FREQ_SYS <= 3000000
		n >>= 2;
#endif
#if		FREQ_SYS <= 750000
		n >>= 4;
#endif
#endif
	while ( n ) {  // total = 12~13 Fsys cycles, 1uS @Fsys=12MHz
		++ SAFE_MOD;  // 2 Fsys cycles, for higher Fsys, add operation here
#ifdef	FREQ_SYS
#if		FREQ_SYS >= 14000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 16000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 18000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 20000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 22000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 24000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 26000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 28000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 30000000
		++ SAFE_MOD;
#endif
#if		FREQ_SYS >= 32000000
		++ SAFE_MOD;
#endif
#endif
		-- n;
	}
}

/*******************************************************************************
* Function Name  : mDelayms(UNIT16 n)
* Description    : ms延时函数
* Input          : UNIT16 n
* Output         : None
* Return         : None
*******************************************************************************/
void	mDelaymS( UINT16 n )                                                  // 以mS为单位延时
{
	while ( n ) {
#ifdef	DELAY_MS_HW
		while ( ( TKEY_CTRL & bTKC_IF ) == 0 );
		while ( TKEY_CTRL & bTKC_IF );
#else
		mDelayuS( 1000 );
#endif
		-- n;
	}
}                                         

/*******************************************************************************
* Function Name  : CH554UART0Alter()
* Description    : CH554串口0引脚映射,串口映射到P0.2和P0.3
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH554UART0Alter()
{
    PIN_FUNC |= bUART0_PIN_X;                                                  //串口映射到P1.2和P1.3
}

/*******************************************************************************
* Function Name  : mInitSTDIO()
* Description    : CH554串口0初始化,默认使用T1作UART0的波特率发生器,也可以使用T2
                   作为波特率发生器
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void	mInitSTDIO( )
{
    UINT32 x;
    UINT8 x2; 

    SM0 = 0;
    SM1 = 1;
    SM2 = 0;                                                                   //串口0使用模式1
                                                                               //使用Timer1作为波特率发生器
    RCLK = 0;                                                                  //UART0接收时钟
    TCLK = 0;                                                                  //UART0发送时钟
    PCON |= SMOD;
    x = 10 * FREQ_SYS / UART0_BUAD / 16;                                       //如果更改主频，注意x的值不要溢出                            
    x2 = x % 10;
    x /= 10;
    if ( x2 >= 5 ) x ++;                                                       //四舍五入

    TMOD = TMOD & ~ bT1_GATE & ~ bT1_CT & ~ MASK_T1_MOD | bT1_M1;              //0X20，Timer1作为8位自动重载定时器
    T2MOD = T2MOD | bTMR_CLK | bT1_CLK;                                        //Timer1时钟选择
    TH1 = 0-x;                                                                 //12MHz晶振,buad/12为实际需设置波特率
    TR1 = 1;                                                                   //启动定时器1
    TI = 1;
    REN = 1;                                                                   //串口0接收使能
}

/*******************************************************************************
* Function Name  : CH554UART0RcvByte()
* Description    : CH554UART0接收一个字节
* Input          : None
* Output         : None
* Return         : SBUF
*******************************************************************************/
UINT8  CH554UART0RcvByte( )
{
    while(RI == 0);                                                            //查询接收，中断方式可不用
    RI = 0;
    return SBUF;
}

/*******************************************************************************
* Function Name  : CH554UART0SendByte(UINT8 SendDat)
* Description    : CH554UART0发送一个字节
* Input          : UINT8 SendDat；要发送的数据
* Output         : None
* Return         : None
*******************************************************************************/
void CH554UART0SendByte(UINT8 SendDat)
{
	SBUF = SendDat;                                                              //查询发送，中断方式可不用下面2条语句,但发送前需TI=0
	while(TI ==0);
	TI = 0;
}

/*******************************************************************************
* Function Name  : UART1Setup()
* Description    : CH554串口1初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void	UART1Setup( )
{
   U1SM0 = 0;                                                                   //UART1选择8位数据位
   U1SMOD = 1;                                                                  //快速模式
   U1REN = 1;                                                                   //使能接收
   SBAUD1 = 0 - FREQ_SYS/16/UART1_BUAD;
}

/*******************************************************************************
* Function Name  : CH554UART1RcvByte()
* Description    : CH554UART1接收一个字节
* Input          : None
* Output         : None
* Return         : SBUF
*******************************************************************************/
UINT8  CH554UART1RcvByte( )
{
    while(U1RI == 0);                                                           //查询接收，中断方式可不用
    U1RI = 0;
    return SBUF1;
}

/*******************************************************************************
* Function Name  : CH554UART1SendByte(UINT8 SendDat)
* Description    : CH554UART1发送一个字节
* Input          : UINT8 SendDat；要发送的数据
* Output         : None
* Return         : None
*******************************************************************************/
void CH554UART1SendByte(UINT8 SendDat)
{
	SBUF1 = SendDat;                                                             //查询发送，中断方式可不用下面2条语句,但发送前需TI=0
	while(U1TI ==0);
	U1TI = 1;
}

/*******************************************************************************
* Function Name  : CH554WDTModeSelect(UINT8 mode)
* Description    : CH554看门狗模式选择
* Input          : UINT8 mode 
                   0  timer
                   1  watchDog
* Output         : None
* Return         : None
*******************************************************************************/
void CH554WDTModeSelect(UINT8 mode)
{
   SAFE_MOD = 0x55;
   SAFE_MOD = 0xaa;                                                             //进入安全模式
   if(mode){
     GLOBAL_CFG |= bWDOG_EN;                                                    //启动看门狗复位
   }
   else GLOBAL_CFG &= ~bWDOG_EN;	                                            //启动看门狗仅仅作为定时器
   SAFE_MOD = 0x00;                                                             //退出安全模式
   WDOG_COUNT = 0;                                                              //看门狗赋初值
}

/*******************************************************************************
* Function Name  : CH554WDTFeed(UINT8 tim)
* Description    : CH554看门狗定时时间设置
* Input          : UINT8 tim 看门狗复位时间设置
                   00H(6MHz)=2.8s
                   80H(6MHz)=1.4s
* Output         : None
* Return         : None
*******************************************************************************/
void CH554WDTFeed(UINT8 tim)
{
   WDOG_COUNT = tim;                                                             //看门狗计数器赋值	
}