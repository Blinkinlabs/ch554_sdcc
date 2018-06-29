#pragma once

#include <stdint.h>


#define  SPI_CK_SET( n ) (SPI0_CK_SE = n)                                     //SPIÊ±ÖÓÉèÖÃº¯Êý 

#define SPIMasterAssertCS() (SCS = 0)
#define SPIMasterDeassertCS() (SCS = 1)

/*******************************************************************************
* Function Name  : SPIMasterModeSet( uint8_t mode ) 
* Description    : SPIÖ÷»úÄ£Ê½³õÊ¼»¯
* Input          : uint8_t mode						 
* Output         : None
* Return         : None
*******************************************************************************/
void SPIMasterModeSet(uint8_t mode);

/*******************************************************************************
* Function Name  : CH554SPIInterruptInit()
* Description    : CH554SPIÖÐ¶Ï³õÊ¼»¯
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH554SPIInterruptInit();

/*******************************************************************************
* Function Name  : CH554SPIMasterWrite(uint8_t dat)
* Description    : CH554Ó²¼þSPIÐ´Êý¾Ý£¬Ö÷»úÄ£Ê½
* Input          : uint8_t dat   Êý¾Ý
* Output         : None
* Return         : None
*******************************************************************************/
void CH554SPIMasterWrite(uint8_t dat);

/*******************************************************************************
* Function Name  : CH554SPIMasterRead( )
* Description    : CH554Ó²¼þSPI0¶ÁÊý¾Ý£¬Ö÷»úÄ£Ê½
* Input          : None
* Output         : None
* Return         : uint8_t ret   
*******************************************************************************/
uint8_t CH554SPIMasterRead();

/*******************************************************************************
* Function Name  : SPISlvModeSet( ) 
* Description    : SPI´Ó»úÄ£Ê½³õÊ¼»¯
* Input          : None						 
* Output         : None
* Return         : None
*******************************************************************************/
void SPISlvModeSet( );

/*******************************************************************************
* Function Name  : CH554SPISlvWrite(uint8_t dat)
* Description    : CH554Ó²¼þSPIÐ´Êý¾Ý£¬´Ó»úÄ£Ê½
* Input          : uint8_t dat   Êý¾Ý
* Output         : None
* Return         : None
*******************************************************************************/
void CH554SPISlvWrite(uint8_t dat);

/*******************************************************************************
* Function Name  : CH554SPISlvRead( )
* Description    : CH554Ó²¼þSPI0¶ÁÊý¾Ý£¬´Ó»úÄ£Ê½
* Input          : None
* Output         : None
* Return         : uint8_t ret   
*******************************************************************************/
uint8_t CH554SPISlvRead();
