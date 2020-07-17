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
