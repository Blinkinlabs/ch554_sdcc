
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

void  CH554UART0Alter();                 //CH554 serial port 0 pin is mapped to P0.2 / P0.3
void	mInitSTDIO( );                      //T1 as a baud rate generator
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

void CH554WDTModeSelect(uint8_t mode);     //CH554 watchdog mode setting
void CH554WDTFeed(uint8_t tim);            //CH554 watchdog feeding dog
