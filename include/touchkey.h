#pragma once

//#define INTERRUPT_TouchKey   0                                                 //Open TouchKey interrupt mode
#define KEY_FIRST            0                                                 //Sampling start channel                             
#define KEY_LAST             3                                                 //End of sampling channel
#define KEY_ACT              20                                                // Button is pressed, the channel sampling value decreases, the value decreases, the sensitivity is high, the value increases, the sensitivity is low
#define KEY_BASE_SAMPLE_TIME 5                                                 //Sampling reference value Sampling times, in order to obtain a stable channel reference value

extern uint16_t	KeyFree[KEY_LAST-KEY_FIRST+1];                                 //Touch idle value storage, used to compare the state of the key
extern volatile uint8_t	KeyBuf;                                                //Touch button status, 0 means no button, 1 means currently detected button is pressed

#define TouchKeyOFF() {TKEY_CTRL &= 0xF8;}                                     //Turn off the capacitance detection, only for 1ms or 2ms timer interrupt
#define TouchKeyON_NoChannel() {TKEY_CTRL = TKEY_CTRL & 0xF8 | 7;}             //Turn on capacitance detection, but do not connect the channel
#define TouchKeyQueryCyl1ms() {TKEY_CTRL &= ~bTKC_2MS;}                        //Touch button sampling period setting 1ms
#define TouchKeyQueryCyl2ms() {TKEY_CTRL |= bTKC_2MS;}                         //Touch button sampling period setting 2ms

/*******************************************************************************
* Function Name  : TouchKeyChannelSelect(UINT8 ch)
* Description    : Touch key channel selection
* Input          : UINT8 ch Use channel
                   0: Turn off the capacitance detection, only for 1ms or 2ms timer interrupt                   
                   1~6 Representing sampling channels
                   7: Turn on capacitance detection, but do not connect the channel
* Output         : None
* Return         : success1
                   failure 0
*******************************************************************************/
uint8_t TouchKeyChannelSelect(uint8_t ch);

/*******************************************************************************
* Function Name  : GetTouchKeyFree()
* Description    : Get the value of the touch button idle state
* Input          : None								 
* Output         : None
* Return         : None
*******************************************************************************/
void GetTouchKeyFree();  

#if !INTERRUPT_TouchKey
/*******************************************************************************
* Function Name  : TouchKeyChannelQuery()
* Description    : Touch button channel status query
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TouchKeyChannelQuery();
#endif

