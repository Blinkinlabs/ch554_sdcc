
/********************************** (C) COPYRIGHT *******************************
* File Name          : Main.C
* Author             : WCH
* Version            : V1.0
* Date               : 2019/07/22
* Description        : CH554 ADC initialization, ADC interrupt and query mode collection demo example                
*******************************************************************************/
#include <ch554.h>
#include <debug.h>
#include <adc.h>
#include <stdio.h>
#include <string.h>

#define ADC_INTERRUPT  1



/*******************************************************************************
* Function Name  : ADCInterrupt(void)
* Description    : ADC Interrupt service routine
*******************************************************************************/
uint16_t UserData;

void	ADCInterrupt(void) __interrupt(INT_NO_ADC)                        //ADC interrupt service routine, using register set 1
{ 
    if(ADC_IF ==  1)                                                          //ADC complete interrupt
    { 
      UserData = ADC_DATA;                                                    //Take ADC sampling data
      ADC_IF = 0;		                                                          //Clear ADC interrupt flag
	  printf(" %d \n ",UserData);
    }
    if(CMP_IF ==  1)                                                          //Voltage comparison complete interrupt
    {	
//       UserData = ADC_CTRL&0x80 >> 7);	                                        //Save comparator result		
      CMP_IF = 0;		                                                          //Clear Comparator Complete Interrupt
    }
}


/*******************************************************************************
* Function Name  : Main Loop 
* Description    : Programm entry point 
*******************************************************************************/

void main( ) 
{
    uint16_t i;
    uint16_t j = 0;
    CfgFsys( );                                                                //CH554 clock selection configuration   
    mDelaymS(20);
    mInitSTDIO( );                                                             //Serial port 0 initialization
    printf("start ...\n"); 

    ADCInit( 0 );                                                              //ADC clock configuration, 0 (96clk) 1 (384clk), ADC module is on

#if ADC_INTERRUPT                                                              //ADC interrupt mode
    EA = 1;
    IE_ADC = 1;
    while(1)                                                                   
    {
      for(i=0;i<4;i++){	
		  printf("AIN%02x: ",(uint16_t)i);			                                     //Print and display of ADC sampling channel for debugging	
        ADC_ChannelSelect( i );                                                //ADC sampling power on and channel setting, i (0-3) means sampling channel
        ADC_START = 1;                                                         //Start sampling, enter interrupt when sampling is completed
        mDelayuS(30);                                                          //Wait for the acquisition to complete before switching to the next channel
      }
      mDelaymS(1000);
    } 
#else
	
    while(1)                                                                   // ADC query method                                                           
    {
      for(i=0;i<4;i++){				
		 printf("AIN%02x \n",(uint16_t)i);		
        ADC_ChannelSelect( i );                                                // Initialize ADC sampling
        ADC_START = 1;                                                         //Start sampling, enter interrupt when sampling is completed
        while(ADC_START){};                                                      //When ADC_START becomes 0, it means sampling is completed
        printf("DATA: %d \n",(uint16_t)ADC_DATA);
        mDelaymS(100);                                                         //Simulate MCU to do other things
      }
      mDelaymS(1000);	
    }		
#endif		
}
