#pragma once

#if PWM_INTERRUPT
extern void  PWMInterruptEnable();                                            //PWM中断使能
#endif

#define SetPWMClk(CK_SE) (PWM_CK_SE = CK_SE)                                  //分频,默认时钟Fsys    

#define SetPWM1Dat(dat)  (PWM_DATA1 = dat)                                    //设置PWM输出占空比
#define SetPWM2Dat(dat)  (PWM_DATA2 = dat)

#define PWM1PinAlter( )  {PIN_FUNC |= bPWM1_PIN_X;}                           //PWM映射脚P30
#define PWM2PinAlter( )  {PIN_FUNC |= bPWM2_PIN_X;}                           //PWM映射脚P31

#define ForceClearPWMFIFO( ) {PWM_CTRL |= bPWM_CLR_ALL;}                      //强制清除PWM FIFO和COUNT
#define CancelClearPWMFIFO( ) {PWM_CTRL &= ~bPWM_CLR_ALL;}                    //取消清除PWM FIFO和COUNT

#define PWM1OutEnable()  (PWM_CTRL |= bPWM1_OUT_EN)                           //允许PWM1输出                           
#define PWM2OutEnable()  (PWM_CTRL |= bPWM2_OUT_EN)                           //允许PWM2输出  
#define DsiablePWM1Out() (PWM_CTRL &= ~bPWM1_OUT_EN)                          //关闭PWM1输出                           
#define DisablePWM2Out() (PWM_CTRL &= ~bPWM2_OUT_EN)                          //关闭PWM2输出  

#define PWM1OutPolarHighAct()(PWM_CTRL &= ~bPWM1_POLAR)                       //PWM1输出默认低，高有效                           
#define PWM2OutPolarHighAct()(PWM_CTRL &= ~bPWM2_POLAR)                       //PWM2输出默认低，高有效  
#define PWM1OutPolarLowAct() (PWM_CTRL |= bPWM1_POLAR)                        //PWM1输出默认高，低有效                         
#define PWM2OutPolarLowAct() (PWM_CTRL |= bPWM2_POLAR)                        //PWM2输出默认高，低有效   

//PWM中断使能
#define PWMInterruptEnable() {PWM_CTRL |= bPWM_IF_END | bPWM_IE_END; IE_PWMX = 1;}
#define PWMInterruptDisable() {IE_PWMX = 0;}
