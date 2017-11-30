/* 调试 */
/* 提供printf子程序和延时函数 */

#ifndef	__DEBUG_H__
#define __DEBUG_H__

//定义函数返回值
#ifndef  SUCCESS
#define  SUCCESS  0
#endif
#ifndef  FAIL
#define  FAIL    0xFF
#endif

//定义定时器起始
#ifndef  START
#define  START  1
#endif
#ifndef  STOP
#define  STOP    0
#endif

#ifndef  DE_PRINTF
#define  DE_PRINTF     0
#endif
#define	 FREQ_SYS	     12000000	         //系统主频12MHz
#ifndef  UART0_BUAD
#define  UART0_BUAD    57600
#define  UART1_BUAD    57600
#endif

void	CfgFsys( );                        //CH554时钟选择和配置
void	mDelayuS( UINT16 n );              // 以uS为单位延时
void	mDelaymS( UINT16 n );              // 以mS为单位延时
void  CH554UART0Alter();                 //CH554串口0引脚映射到P0.2/P0.3
void	mInitSTDIO( );                      //T1作为波特率发生器
UINT8 CH554UART0RcvByte( );              //CH554 UART0查询方式接收一个字节
void  CH554UART0SendByte(UINT8 SendDat); //CH554UART0发送一个字节

void	UART1Setup( );                     //
UINT8 CH554UART1RcvByte( );              //CH554 UART1查询方式接收一个字节
void  CH554UART1SendByte(UINT8 SendDat); //CH554UART1发送一个字节

void CH554WDTModeSelect(UINT8 mode);     //CH554看门狗模式设置 
void CH554WDTFeed(UINT8 tim);            //CH554看门狗喂狗
#endif
