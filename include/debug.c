/********************************** (C) COPYRIGHT *******************************
* File Name          : Debug.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/01/20
* Description        : CH554 DEBUG Interface
                     CH554 main frequency modification, delay function definition
                     Serial port 0 and serial port 1 initialization
                     Serial port 0 and serial port 1 transceiver subfunctions
                     Watchdog initialization			 
*******************************************************************************/

#include <stdint.h>

#include "ch554.h"
#include "debug.h"

/*******************************************************************************
* Function Name  : CfgFsys( )
* Description  : CH554 clock selection and configuration function, Fsys 6MHz is used by default, FREQ_SYS can be passed
                   CLOCK_CFG configuration, the formula is as follows:
                   Fsys = (Fosc * 4 / (CLOCK_CFG & MASK_SYS_CK_SEL); the specific clock needs to be configured by yourself
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/ 
void	CfgFsys( )  
{
// 		SAFE_MOD = 0x55;
// 		SAFE_MOD = 0xAA;
//     CLOCK_CFG |= bOSC_EN_XT;                          // Enable external crystal
//     CLOCK_CFG & = ~ bOSC_EN_INT;                     // Turn off the internal crystal

	SAFE_MOD = 0x55;
	SAFE_MOD = 0xAA;

#if FREQ_SYS == 32000000
	CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x07;  // 32MHz
#elif FREQ_SYS == 24000000
	CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x06;  // 24MHz	
#elif FREQ_SYS == 16000000
	CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x05;  // 16MHz	
#elif FREQ_SYS == 12000000
	CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x04;  // 12MHz
#elif FREQ_SYS == 6000000
	CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x03;  // 6MHz	
#elif FREQ_SYS == 3000000
	CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x02;  // 3MHz	
#elif FREQ_SYS == 750000
	CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x01;  // 750KHz	
#elif FREQ_SYS == 187500
	CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x00;  // 187.5MHz		
#else
	#warning FREQ_SYS invalid or not set
#endif

	SAFE_MOD = 0x00;
}



/*******************************************************************************
* Function Name  : mDelayus(UNIT16 n)
* Description    : us delay function
* Input          : UNIT16 n
* Output         : None
* Return         : None
*******************************************************************************/ 
void	mDelayuS( uint16_t n )  // Delay in uS
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
* Description    : ms delay function
* Input          : UNIT16 n
* Output         : None
* Return         : None
*******************************************************************************/
void	mDelaymS( uint16_t n )                                                  // Delay in mS
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
* Description    : CH554 serial port 0 pin mapping, serial port mapping to P0.2 and P0.3

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
* Description    : CH554 serial port 0 is initialized, T1 is used as the baud rate generator of UART0 by default, T2 can also be used
                   As a baud rate generator
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void	mInitSTDIO( )
{
    uint32_t x;
    uint8_t x2;

    SM0 = 0;
    SM1 = 1;
    SM2 = 0;                                                                   //Serial port 0 usage mode 1
                                                                               //Use Timer1 as a baud rate generator
    RCLK = 0;                                                                  //UART0 receive clock
    TCLK = 0;                                                                  //UART0 transmit clock
    PCON |= SMOD;
    x = 10 * FREQ_SYS / UART0_BAUD / 16;                                       //If you change the main frequency, be careful not to overflow the value of x
    x2 = x % 10;
    x /= 10;
    if ( x2 >= 5 ) x ++;                                                       //rounding

    TMOD = TMOD & ~ bT1_GATE & ~ bT1_CT & ~ MASK_T1_MOD | bT1_M1;              //0X20, Timer1 as 8-bit auto-reload timer
    T2MOD = T2MOD | bTMR_CLK | bT1_CLK;                                        //Timer1 clock selection
    TH1 = 0-x;                                                                 //12MHz crystal oscillator, buad / 12 is the actual need to set the baud rate
    TR1 = 1;                                                                   //Start timer 1
    TI = 1;
    REN = 1;                                                                   //Serial 0 receive enable
}

/*******************************************************************************
* Function Name  : CH554UART0RcvByte()
* Description    : CH554UART0 receives a byte
* Input          : None
* Output         : None
* Return         : SBUF
*******************************************************************************/
uint8_t  CH554UART0RcvByte( )
{
    while(RI == 0);                                                            //Inquiry receiving, the interrupt mode can be used
    RI = 0;
    return SBUF;
}

/*******************************************************************************
* Function Name  : CH554UART0SendByte(uint8_t SendDat)
* Description    : CH554UART0 sends a byte
* Input          : uint8_t SendDat; the data to be sent
* Output         : None
* Return         : None
*******************************************************************************/
void CH554UART0SendByte(uint8_t SendDat)
{

        SBUF = SendDat;                                                              // Query sending, the following two statements can be used in the interrupt mode, but TI = 0 is required before sending
        while(TI ==0);
        TI = 0;
}

#if SDCC < 370
void putchar(char c)
{
    while (!TI) /* assumes UART is initialized */
    ;
    TI = 0;
    SBUF = c;
}

char getchar() {
    while(!RI); /* assumes UART is initialized */
    RI = 0;
    return SBUF;
}
#else
int putchar(int c)
{
    while (!TI) /* assumes UART is initialized */
    ;
    TI = 0;
    SBUF = c & 0xFF;

    return c;
}

int getchar() {
    while(!RI); /* assumes UART is initialized */
    RI = 0;
    return SBUF;
}
#endif

/*******************************************************************************
* Function Name  : CH554UART1Alter()
* Description    : Set the alternate pin mappings for UART1 (TX on P3.2, RX on P3.4)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH554UART1Alter()
{
    PIN_FUNC |= bUART1_PIN_X;
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
   SBAUD1 = 256 - FREQ_SYS/16/UART1_BAUD;
}

/*******************************************************************************
* Function Name  : CH554UART1RcvByte()
* Description    : CH554UART1接收一个字节
* Input          : None
* Output         : None
* Return         : SBUF
*******************************************************************************/
uint8_t  CH554UART1RcvByte( )
{
    while(U1RI == 0);                                                           //查询接收，中断方式可不用
    U1RI = 0;
    return SBUF1;
}

/*******************************************************************************
* Function Name  : CH554UART1SendByte(uint8_t SendDat)
* Description    : CH554UART1发送一个字节
* Input          : uint8_t SendDat；要发送的数据
* Output         : None
* Return         : None
*******************************************************************************/
void CH554UART1SendByte(uint8_t SendDat)
{
        SBUF1 = SendDat;                                                             //查询发送，中断方式可不用下面2条语句,但发送前需TI=0
        while(U1TI ==0);
        U1TI = 0;
}

/*******************************************************************************
* Function Name  : CH554WDTModeSelect(uint8_t mode)
* Description    : CH554 watchdog mode selection
* Input          : uint8_t mode
                   0  timer
                   1  watchDog
* Output         : None
* Return         : None
*******************************************************************************/
void CH554WDTModeSelect(uint8_t mode)
{
   SAFE_MOD = 0x55;
   SAFE_MOD = 0xaa;                                                             //Enter Safe Mode
   if(mode){
     GLOBAL_CFG |= bWDOG_EN;                                                    //Start watchdog reset
   }

   else GLOBAL_CFG &= ~bWDOG_EN;	                                            //Start watchdog only as a timer
   SAFE_MOD = 0x00;                                                             //exit safe Mode
   WDOG_COUNT = 0;                                                              //Watchdog assignment initial value

}

/*******************************************************************************
* Function Name  : CH554WDTFeed(uint8_t tim)
* Description    : CH554 watchdog timer time setting
* Input          : uint8_t tim watchdog reset time setting

                   00H(6MHz)=2.8s
                   80H(6MHz)=1.4s
* Output         : None
* Return         : None
*******************************************************************************/
void CH554WDTFeed(uint8_t tim)
{

   WDOG_COUNT = tim;                                                            // Watchdog counter assignment

}
