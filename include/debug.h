
/* Debug */
/* Provide printf subroutine and delay function */

#pragma once

#include <stdint.h>

#ifndef  UART0_BAUD
#define  UART0_BAUD    9600
#endif

#ifndef UART1_BAUD
#define  UART1_BAUD    9600
#endif

void	CfgFsys( );                       // CH554 clock selection and configuration

void mDelayuS (uint16_t n); // Delay in units of uS
void mDelaymS (uint16_t n); // Delay in mS

/*******************************************************************************
* Function Name  : CH554UART0Alter()
* Description    : CH554 serial port 0 pin mapping, serial port mapping to P0.2 and P0.3

* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
inline void CH554UART0Alter()
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
inline void	mInitSTDIO( )
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

uint8_t CH554UART0RcvByte( );              //CH554 UART0 query mode receives a byte
void  CH554UART0SendByte(uint8_t SendDat); //CH554UART0 sends a byte

void  CH554UART1Alter();                 //CH554 serial port 1 pin is mapped to P3.4 / P3.2
void	UART1Setup( );                     //
uint8_t CH554UART1RcvByte( );              //CH554 UART1 query mode receives a byte
void  CH554UART1SendByte(uint8_t SendDat); // CH554UART1 sends a byte

#if SDCC < 370
void putchar(char c);
char getchar();
#else
int putchar(int c);
int getchar(void);
#endif

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
