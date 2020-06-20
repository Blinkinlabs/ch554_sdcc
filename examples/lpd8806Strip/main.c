// Blink an LED connected to pin 1.7

#include <ch554.h>
#include <debug.h>
#include <stdint.h>
#include <stdio.h>

#define DATA_PIN 6
#define CLK_PIN  4
SBIT(DATA, 0x90, DATA_PIN);
SBIT(CLK, 0x90, CLK_PIN);
uint8_t numLEDs = 15;
uint8_t colouri;
uint8_t timeIndex;

void clk_out(uint8_t data){

    uint8_t biti;

;    for(biti = 0 ; biti < 8 ; biti++) {
        CLK = 1;
        mDelayuS( 50 );
        DATA = (data & (128 >> biti)) ;
        CLK = 0;
        mDelayuS( 50 );

    }
}



void show(uint8_t r, uint8_t g, uint8_t b, uint8_t l ){
    uint16_t i;

    uint8_t li;

        for (li = 0 ; li < l ; li++){
        clk_out(b |128);
        clk_out(r |128);
        clk_out(g |128);
        }
            for (i = ((l + 31) / 32); i > 0; i--){
                clk_out(0);
           
        }
        
}


void main() {
    CfgFsys();

    // Configure pin 1.7 as GPIO output
    P1_DIR_PU &= 0x0C;
    P1_MOD_OC = P1_MOD_OC & ~(1<<DATA_PIN);
    P1_MOD_OC = P1_MOD_OC & ~(1<<CLK_PIN);  //output mode 1 = Open Drain = 0 pull down 
    P1_DIR_PU = P1_DIR_PU |	(1<<DATA_PIN); //port direction 0 inputs 1 outputs
    P1_DIR_PU = P1_DIR_PU |	(1<<CLK_PIN);
 
    colouri = 0;



    for(timeIndex = 1 ; timeIndex < 120 ; timeIndex++) {
        
    	//mDelaymS(500);
        show(0,timeIndex,128 - timeIndex,numLEDs);
        
    }
    
    for(timeIndex = 1 ; timeIndex < 120 ; timeIndex++) {
        
    	//mDelaymS(500);
        show(timeIndex,128 - timeIndex,0,numLEDs);
        
    }
    for(timeIndex = 1 ; timeIndex < 120 ; timeIndex++) {
        
    	//mDelaymS(500);
        show(128 - timeIndex,0,timeIndex,numLEDs);
        
    }
  
  
  
}
