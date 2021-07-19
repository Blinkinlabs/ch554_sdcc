#pragma once

#include <stdint.h>


#define  SPI_CK_SET( n ) (SPI0_CK_SE = n)    //SPI时钟分频设置

#define SPIMasterAssertCS() (SCS = 0)
#define SPIMasterDeassertCS() (SCS = 1)

/*******************************************************************************
* Function Name  : SPIMasterModeSet( uint8_t mode )
* Description    : SPI主机模式初始化
* Input          : uint8_t mode
* Output         : None
* Return         : None
*******************************************************************************/
void SPIMasterModeSet(uint8_t mode);

/*******************************************************************************
* Function Name  : CH554SPIInterruptInit()
* Description    : CH554SPI中断初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH554SPIInterruptInit();

/*******************************************************************************
* Function Name  : CH554SPIMasterWrite(uint8_t dat)
* Description    : CH554硬件SPI写数据，主机模式
* Input          : uint8_t dat   数据
* Output         : None
* Return         : None
*******************************************************************************/
void CH554SPIMasterWrite(uint8_t dat);

/*******************************************************************************
* Function Name  : CH554SPIMasterRead( )
* Description    : CH554硬件SPI0读数据，主机模式
* Input          : None
* Output         : None
* Return         : uint8_t ret
*******************************************************************************/
uint8_t CH554SPIMasterRead();

/*******************************************************************************
* Function Name  : SPISlvModeSet( )
* Description    : SPI从机模式初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPISlvModeSet( );

/*******************************************************************************
* Function Name  : CH554SPISlvWrite(uint8_t dat)
* Description    : CH554硬件SPI写数据，从机模式
* Input          : uint8_t dat   数据
* Output         : None
* Return         : None
*******************************************************************************/
void CH554SPISlvWrite(uint8_t dat);

/*******************************************************************************
* Function Name  : CH554SPISlvRead( )
* Description    : CH554硬件SPI0读数据，从机模式
* Input          : None
* Output         : None
* Return         : uint8_t ret
*******************************************************************************/
uint8_t CH554SPISlvRead();
