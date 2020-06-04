
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
=======
// Blink an LED connected to pin 1.7

#include <ch554.h>
#include <debug.h>

#define LED_PIN 7
SBIT(LED, 0x90, LED_PIN);


/*  ADC and comparator Registers  */
/*
SFR(ADC_CTRL,	0x80);	// ADC control
   SBIT(CMPO,	0x80, 7);	// ReadOnly: comparator result input
   SBIT(CMP_IF,	0x80, 6);	// flag for comparator result changed, direct bit address clear
   SBIT(ADC_IF,	0x80, 5);	// interrupt flag for ADC finished, direct bit address clear
   SBIT(ADC_START,	0x80, 4);	// set 1 to start ADC, auto cleared when ADC finished
   SBIT(CMP_CHAN,	0x80, 3);	// comparator IN- input channel selection: 0=AIN1, 1=AIN3
   SBIT(ADC_CHAN1,	0x80, 1);	// ADC/comparator IN+ channel selection high bit
   SBIT(ADC_CHAN0,	0x80, 0);	// ADC/comparator IN+ channel selection low bit
// ADC_CHAN1 & ADC_CHAN0: ADC/comparator IN+ channel selection
//   00: AIN0(P1.1)
//   01: AIN1(P1.4)
//   10: AIN2(P1.5)
//   11: AIN3(P3.2)
/*
SFR(ADC_CFG,	0x9A);	// ADC config
#define bADC_EN           0x08      // control ADC power: 0=shut down ADC, 1=enable power for ADC
#define bCMP_EN           0x04      // control comparator power: 0=shut down comparator, 1=enable power for comparator
#define bADC_CLK          0x01      // ADC clock frequency selection: 0=slow clock, 384 Fosc cycles for each ADC, 1=fast clock, 96 Fosc cycles for each ADC
SFR(ADC_DATA,	0x9F);	// ReadOnly: ADC data
*/

void setupADC(){
// setup requirments for adc 

AIN0 = 1;
AIN1 = 1;
AIN2 = 1;
AIN3 = 1;
ADC_CFG = ADC_CFG | bADC_EN & ~(bCMP_EN);
}


void main() {
    CfgFsys();
    setupADC();
    // Configure pin 1.7 as GPIO output
    P1_DIR_PU &= 0x0C;
    P1_MOD_OC = P1_MOD_OC & ~(1<<LED_PIN);
    P1_DIR_PU = P1_DIR_PU |	(1<<LED_PIN);

    while (1) {
    	mDelaymS(100);
        LED = !LED;
    }
}

