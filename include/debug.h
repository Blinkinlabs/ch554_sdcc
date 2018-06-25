/* 调试 */
/* 提供printf子程序和延时函数 */

#pragma once

#include <stdint.h>

#ifndef  UART0_BAUD
#define  UART0_BAUD    9600
#endif

#ifndef UART1_BAUD
#define  UART1_BAUD    9600
#endif

void	CfgFsys( );                        //CH554时钟选择和配置

void	mDelayuS( uint16_t n );              // 以uS为单位延时
void	mDelaymS( uint16_t n );              // 以mS为单位延时


void  CH554UART0Alter();                 //CH554串口0引脚映射到P0.2/P0.3
void	mInitSTDIO( );                      //T1作为波特率发生器
uint8_t CH554UART0RcvByte( );              //CH554 UART0查询方式接收一个字节
void  CH554UART0SendByte(uint8_t SendDat); //CH554UART0发送一个字节

void  CH554UART1Alter();                 //CH554串口1引脚映射到P3.4/P3.2
void	UART1Setup( );                     //
uint8_t CH554UART1RcvByte( );              //CH554 UART1查询方式接收一个字节
void  CH554UART1SendByte(uint8_t SendDat); //CH554UART1发送一个字节

#if SDCC < 370
void putchar(char c);
char getchar();
#else
int putchar(int c);
int getchar(void);
#endif

void CH554WDTModeSelect(uint8_t mode);     //CH554看门狗模式设置
void CH554WDTFeed(uint8_t tim);            //CH554看门狗喂狗
