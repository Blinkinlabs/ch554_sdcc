#pragma once

//#define INTERRUPT_TouchKey   0                                                 //开启TouchKey中断方式

#define KEY_FIRST            0                                                 //采样起始通道                                      
#define KEY_LAST             3                                                 //采样结束通道
#define KEY_ACT              20                                                //按键按下，通道采样值减小，该值减小，灵敏度高，该值增大，灵敏度低
#define KEY_BASE_SAMPLE_TIME 5                                                 //采样基准值采样次数，为了取到稳定的通道基准值

extern uint16_t	KeyFree[KEY_LAST-KEY_FIRST+1];                                 //触摸空闲值存储，用于比较按键状态，如果采样值小于基准值表明按键按下
extern volatile uint8_t	KeyBuf;                                                //触摸按键状态，为0表示无按键，为1表示当前检测按键被按下

#define TouchKeyOFF() {TKEY_CTRL &= 0xF8;}                                     //关闭电容检测，仅作1ms或者2ms定时中断
#define TouchKeyON_NoChannel() {TKEY_CTRL = TKEY_CTRL & 0xF8 | 7;}             //开启电容检测，但是不连接通道
#define TouchKeyQueryCyl1ms() {TKEY_CTRL &= ~bTKC_2MS;}                        //触摸按键采样周期设置1ms
#define TouchKeyQueryCyl2ms() {TKEY_CTRL |= bTKC_2MS;}                         //触摸按键采样周期设置2ms

/*******************************************************************************
* Function Name  : TouchKeyChannelSelect(UINT8 ch)
* Description    : 触摸按键通道选择
* Input          : UINT8 ch 采用通道
                   0: 关闭电容检测，仅作1ms或者2ms定时中断
                   1~6 分别代表采样通道
                   7: 开启电容检测，但是不连接通道
* Output         : None
* Return         : 成功 1
                   失败 0
*******************************************************************************/
uint8_t TouchKeyChannelSelect(uint8_t ch);

/*******************************************************************************
* Function Name  : GetTouchKeyFree()
* Description    : 获取触摸按键空闲状态键值
* Input          : None								 
* Output         : None
* Return         : None
*******************************************************************************/
void GetTouchKeyFree();  

#if !INTERRUPT_TouchKey
/*******************************************************************************
* Function Name  : TouchKeyChannelQuery()
* Description    : 触摸按键通道状态查询
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TouchKeyChannelQuery();
#endif

