/* Debug */
/* Provide printf subroutine and delay function */

#pragma once

#include <stdint.h>
#include "ch554.h"

#ifndef  UART0_BAUD
#define  UART0_BAUD    9600
#endif

#ifndef UART1_BAUD
#define  UART1_BAUD    9600
#endif

void mDelayuS (uint16_t n); // Delay in units of uS
void mDelaymS (uint16_t n); // Delay in mS


/*******************************************************************************
* Function Name  : CfgFsys( )
* Description  : CH554 clock selection and configuration function, Fsys 6MHz is used by default, FREQ_SYS can be passed
�����������������CLOCK_CFG configuration, the formula is as follows:
�����������������Fsys = (Fosc * 4 / (CLOCK_CFG & MASK_SYS_CK_SEL); the specific clock needs to be configured by yourself
*******************************************************************************/ 
inline void CfgFsys( )  
{
	SAFE_MOD = 0x55;
	SAFE_MOD = 0xAA;
//     CLOCK_CFG |= bOSC_EN_XT;                          // Enable external crystal
//     CLOCK_CFG & = ~ bOSC_EN_INT;                      // Turn off the internal crystal

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
* Function Name  : CH554UART0Alter()
* Description    : CH554 serial port 0 pin mapping, serial port mapping to P0.2 and P0.3

*******************************************************************************/
inline void CH554UART0Alter()
{
    PIN_FUNC |= bUART0_PIN_X;           //串口映射到P1.2和P1.3
}

/*******************************************************************************
* Function Name  : mInitSTDIO()
* Description    : CH554 serial port 0 is initialized, T1 is used as the baud rate generator of UART0 by default, T2 can also be used
                   As a baud rate generator
*******************************************************************************/
inline void	mInitSTDIO( )
{
    uint32_t x;
    uint8_t x2;

    // set UART0 to mode 1: 81N
    //SM0 = 0;
    SM1 = 1;
    //SM2 = 0;

    // UART0 receive clock default = T1
    // RCLK = 0;
    // UART0 transmit clock default = T1
    // TCLK = 0;

    PCON = SMOD;
    x = 10 * FREQ_SYS / UART0_BAUD / 16;                                       //If you change the main frequency, be careful not to overflow the value of x
    x2 = x % 10;
    x /= 10;
    if ( x2 >= 5 ) x ++;                                                       //rounding

    TMOD = TMOD & ~ bT1_GATE & ~ bT1_CT & ~ MASK_T1_MOD | bT1_M1;              //0X20, Timer1 as 8-bit auto-reload timer
    T2MOD = T2MOD | bTMR_CLK | bT1_CLK;                                        //Timer1 clock selection
    TH1 = 256-x;

    TR1 = 1;                                                                   //Start timer 1
    TI = 1;
    REN = 1;                                                                   //Serial 0 receive enable
}

/*******************************************************************************
* Function Name  : CH554UART0RcvByte()
* Description    : CH554UART0 receives a byte
* Return         : SBUF
*******************************************************************************/
inline uint8_t  CH554UART0RcvByte( )
{
    while(RI == 0);                     // wait for uart rx interrupt flag
    RI = 0;
    return SBUF;
}

/*******************************************************************************
* Function Name  : CH554UART0SendByte(uint8_t SendDat)
* Description    : CH554UART0 sends a byte
* Input          : uint8_t SendDat; the data to be sent
*******************************************************************************/
inline void CH554UART0SendByte(uint8_t SendDat)
{

        SBUF = SendDat;
        while(TI ==0);                  // wait for transmit to finish (TI == 1)
        TI = 0;
}

/*******************************************************************************
* Function Name  : CH554UART1Alter()
* Description    : Set the alternate pin mappings for UART1 (TX on P3.2, RX on P3.4)
*******************************************************************************/
inline void CH554UART1Alter()
{
    PIN_FUNC |= bUART1_PIN_X;
}

/*******************************************************************************
* Function Name  : UART1Setup()
* Description    : CH554串口1初始化
*******************************************************************************/
inline void	UART1Setup()
{
    U1SM0 = 0;                          //UART1选择8位数据位
    U1SMOD = 1;                         //快速模式
    U1REN = 1;                          //使能接收
    // should correct for rounding in SBAUD1 calculation 
    SBAUD1 = 256 - FREQ_SYS/16/UART1_BAUD;
}

/*******************************************************************************
* Function Name  : CH554UART1RcvByte()
* Description    : CH554UART1接收一个字节
* Return         : SBUF
*******************************************************************************/
inline uint8_t  CH554UART1RcvByte( )
{
    while(U1RI == 0);                   //查询接收，中断方式可不用
    U1RI = 0;
    return SBUF1;
}

/*******************************************************************************
* Function Name  : CH554UART1SendByte(uint8_t SendDat)
* Description    : CH554UART1发送一个字节
* Input          : uint8_t SendDat；要发送的数据
*******************************************************************************/
inline void CH554UART1SendByte(uint8_t SendDat)
{
        SBUF1 = SendDat;                //查询发送，中断方式可不用下面2条语句,但发送前需TI=0
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
inline void CH554WDTModeSelect(uint8_t mode)
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
inline void CH554WDTFeed(uint8_t tim)
{

   WDOG_COUNT = tim;                                                            // Watchdog counter assignment

}

// perform USB bus reset/disconnect
// set UDP to GPIO mode and hold low for device disconnect
inline void disconnectUSB()
{
    PIN_FUNC &= ~(bUSB_IO_EN);
    UDP = 0;
    mDelaymS(50);
    UDP = 1;
    PIN_FUNC |= bUSB_IO_EN;
}
