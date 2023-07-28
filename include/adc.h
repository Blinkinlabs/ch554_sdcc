
#ifndef __ADC_H__
#define __ADC_H__

#include <stdint.h>

/*******************************************************************************
* Function Name  : ADCClkSet(uint8_t div)
* Description    :ADC sampling clock setting, module is turned on, interrupt is turned on
* Input          : uint8_t speed clock setting 
                   0 Slow 384 Fosc           								
                   1 fast 96 Fosc									 
* Output         : None
* Return         : None
*******************************************************************************/
extern void ADCInit(uint8_t speed);

/*******************************************************************************
* Function Name  : ADC_ChannelSelect(uint8_t ch)
* Description    : ADC sampling channel settings
* Input          : uint8_t ch uses channels 0-3
* Output         : None
* Return         : success SUCCESS
                   failure FAIL Channel setting out of range
*******************************************************************************/
extern uint8_t ADC_ChannelSelect(uint8_t ch);

/*******************************************************************************
* Function Name  : VoltageCMPModeInit()
* Description    : Voltage comparator mode initialization
* Input          : uint8_t fo Forward port 0\1\2\3
                   uint8_t re Reverse port 1\3
* Output         : None
* Return         : success SUCCESS
                   failure FAIL
*******************************************************************************/
extern uint8_t VoltageCMPModeInit(uint8_t fo,uint8_t re);

#endif