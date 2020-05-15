
/********************************** (C) COPYRIGHT *******************************
* File Name          : Main.C
* Author             : WCH
* Version            : V1.0
* Date               : 2019/07/22
* Description        : CH554 ADC initialization, ADC interrupt and query mode collection demo example                
*******************************************************************************/
#include "ch554.H"                                                  
#include "debug.H"
#include "adc.h"
#include "stdio.h"
#include <string.h>



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
    while(1)                                                                   
    {
      for(i=0;i<4;i++){	
		  printf("AIN%02x: ",(uint16_t)i);			                                     //Print and display of ADC sampling channel for debugging	
        ADC_ChannelSelect( i );                                                //ADC sampling power on and channel setting, i (0-3) means sampling channel
        ADC_START = 1;                                                         //Start sampling, enter interrupt when sampling is completed
        mDelayuS(30);                                                          //Wait for the acquisition to complete before switching to the next channel
      }
    } 
#else	
    while(1)                                                                   // ADC query method                                                           
    {
      for(i=0;i<4;i++){				
		 printf("AIN%02x ",(uint16_t)i);		
        ADC_ChannelSelect( i );                                                // Initialize ADC sampling
        ADC_START = 1;                                                         //Start sampling, enter interrupt when sampling is completed
        while(ADC_START);                                                      //When ADC_START becomes 0, it means sampling is completed
        printf("DATA: %d\n",(uint16_t)ADC_DATA);
        mDelaymS(100);                                                         //Simulate MCU to do other things
      }	
    }		
#endif		
}