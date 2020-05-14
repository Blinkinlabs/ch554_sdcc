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
