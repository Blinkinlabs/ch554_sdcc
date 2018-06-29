

/********************************** (C) COPYRIGHT *******************************
* File Name          : SPI.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/07/05
* Description        : CH554 SPIÖ÷¡¢´ÓÄ£Ê½½Ó¿Úº¯Êý
×¢£ºÆ¬Ñ¡ÓÐÐ§Ê±£¬´Ó»ú»á×Ô¶¯¼ÓÔØSPI0_S_PREµÄÔ¤ÖÃÖµµ½·¢ËÍÒÆÎ»»º³åÇø£¬ËùÒÔ×îºÃ¿ÉÒÔÔÚÆ¬Ñ¡
ÓÐÐ§Ç°ÏòSPI0_S_PRE¼Ä´æÆ÷Ð´ÈëÔ¤·¢Öµ£¬»òÕßÔÚÖ÷»ú¶Ë¶ªÆúÊ×¸ö½ÓÊÕ×Ö½Ú£¬·¢ËÍÊ±×¢ÒâÖ÷»ú»áÏÈ 
È¡×ßSPI0_S_PREÀïÃæµÄÖµ²úÉúÒ»¸öS0_IF_BYTEÖÐ¶Ï¡£
Èç¹ûÆ¬Ñ¡´ÓÎÞÐ§µ½ÓÐÐ§£¬´Ó»úÊ×ÏÈ½øÐÐ·¢ËÍµÄ»°£¬×îºÃ°ÑÊä³öµÄÊ××Ö½Ú·Åµ½SPI0_S_PRE¼Ä´æÆ÷ÖÐ£»
Èç¹ûÒÑ¾­´¦ÓÚÆ¬Ñ¡ÓÐÐ§µÄ»°£¬Êý¾ÝÊý¾ÝÊ¹ÓÃSPI0_DATA¾Í¿ÉÒÔ
*******************************************************************************/

#include <ch554.h>
#include "spi.h"

/*******************************************************************************
* Function Name  : SPIMasterModeSet( uint8_t mode ) 
* Description    : SPIÖ÷»úÄ£Ê½³õÊ¼»¯
* Input          : uint8_t mode						 
* Output         : None
* Return         : None
*******************************************************************************/
void SPIMasterModeSet(uint8_t mode)
{
    SPI0_SETUP = 0;                                                           //MasterÄ£Ê½,¸ßÎ»ÔÚÇ°
    if(mode == 0){
      SPI0_CTRL = 0x60;                                                       //Ä£Ê½0			
    }			
    else if(mode == 3){
      SPI0_CTRL = 0x68;                                                       //Ä£Ê½3
    }			
    P1_MOD_OC &= 0x0F;
    P1_DIR_PU |= 0xB0;                                                        //SCS,MOSI,SCKÉèÍÆÍìÊä³ö
    P1_DIR_PU &= 0xBF;                                                        //MISOÉè¸¡¿ÕÊäÈë

    // Set clock speed
    SPI0_CK_SE = 0x02;
}

/*******************************************************************************
* Function Name  : CH554SPIInterruptInit()
* Description    : CH554SPIÖÐ¶Ï³õÊ¼»¯
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH554SPIInterruptInit()
{
    //IP_EX |= bIP_SPI0;                                                       //SPI0ÖÐ¶ÏÓÅÏÈ¼¶ÉèÖÃ
    SPI0_SETUP |= bS0_IE_FIFO_OV | bS0_IE_BYTE;                                //Ê¹ÄÜ½ÓÊÕ1×Ö½ÚÖÐ¶Ï£¬Ê¹ÄÜFIFOÒç³öÖÐ¶Ï
    SPI0_CTRL |= bS0_AUTO_IF;                                                  //×Ô¶¯ÇåS0_IF_BYTEÖÐ¶Ï±êÖ¾
    SPI0_STAT |= 0xff;                                                         //Çå¿ÕSPI0ÖÐ¶Ï±êÖ¾
#ifdef SPI_Interrupt
    IE_SPI0 = 1;                                                               //Ê¹ÄÜSPI0ÖÐ¶Ï
#endif
}

/*******************************************************************************
* Function Name  : CH554SPIMasterWrite(uint8_t dat)
* Description    : CH554Ó²¼þSPIÐ´Êý¾Ý,Ö÷»úÄ£Ê½
* Input          : uint8_t dat   Êý¾Ý
* Output         : None
* Return         : None
*******************************************************************************/
void CH554SPIMasterWrite(uint8_t dat)
{
    SPI0_DATA = dat;                                                           
    while(S0_FREE == 0);													   //µÈ´ý´«ÊäÍê³É		
//Èç¹ûbS0_DATA_DIRÎª1£¬´Ë´¦¿ÉÒÔÖ±½Ó¶ÁÈ¡Ò»¸ö×Ö½ÚµÄÊý¾ÝÓÃÓÚ¿ìËÙ¶ÁÐ´	
}

/*******************************************************************************
* Function Name  : CH554SPIMasterRead( )
* Description    : CH554Ó²¼þSPI0¶ÁÊý¾Ý£¬Ö÷»úÄ£Ê½
* Input          : None
* Output         : None
* Return         : uint8_t ret   
*******************************************************************************/
uint8_t CH554SPIMasterRead()
{
    SPI0_DATA = 0xff;
    while(S0_FREE == 0);
    return SPI0_DATA;
}

/*******************************************************************************
* Function Name  : SPISlvModeSet( ) 
* Description    : SPI´Ó»úÄ£Ê½³õÊ¼»¯
* Input          : None						 
* Output         : None
* Return         : None
*******************************************************************************/
void SPISlvModeSet( )
{
    SPI0_SETUP = 0x80;                                                        //SlvÄ£Ê½,¸ßÎ»ÔÚÇ°
    SPI0_CTRL = 0x81;                                                         //¶ÁÐ´FIFO,×Ô¶¯ÇåS0_IF_BYTE±êÖ¾
    P1_MOD_OC &= 0x0F;
    P1_DIR_PU &= 0x0F;                                                        //SCS,MOSI,SCK,MISOÈ«ÉèÖÃ¸¡¿ÕÊäÈë
}

/*******************************************************************************
* Function Name  : CH554SPISlvWrite(uint8_t dat)
* Description    : CH554Ó²¼þSPIÐ´Êý¾Ý£¬´Ó»úÄ£Ê½
* Input          : uint8_t dat   Êý¾Ý
* Output         : None
* Return         : None
*******************************************************************************/
void CH554SPISlvWrite(uint8_t dat)
{
    SPI0_DATA = dat;
    while(S0_IF_BYTE==0);	
    S0_IF_BYTE = 0;		                                                     
}

/*******************************************************************************
* Function Name  : CH554SPISlvRead( )
* Description    : CH554Ó²¼þSPI0¶ÁÊý¾Ý£¬´Ó»úÄ£Ê½
* Input          : None
* Output         : None
* Return         : uint8_t ret   
*******************************************************************************/
uint8_t CH554SPISlvRead()
{
    while(S0_IF_BYTE==0);
    S0_IF_BYTE = 0;	
    return SPI0_DATA;
}

