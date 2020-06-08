/********************************** (C) COPYRIGHT *******************************
* File Name          : Main.C
* Author             : WCH
* Version            : V1.1
* Date               : 2017/07/05
* Description        : CH554Touch button interrupt and query mode to collect and report the current sampling channel button status, including demo functions such as initialization and button sampling
*******************************************************************************/
#include <stdint.h>
#include <stdio.h>

#include <ch554.h>
#include <debug.h>
#include <touchkey.h>

void main()
{
    uint8_t i;
    CfgFsys( );                                                               //CH554 clock selection configuration
    mDelaymS(5);                                                               //It is recommended to modify the main frequency with a slight delay to wait for the chip power supply to stabilize
    mInitSTDIO( );                                                             //Serial port 0 initialization
    UART1Setup();

    printf("start ...\n");

    P1_DIR_PU &= 0x0C;                                                         //All touch channels are set as floating input, and channels that are not used can be left unset
    TouchKeyQueryCyl2ms();                                                     //TouchKey query cycle 2ms
    GetTouchKeyFree();                                                         //Get sampling reference value
#if DE_PRINTF
    for(i=KEY_FIRST;i<(KEY_LAST+1);i++)                                        //Print sampling reference value
    {
        printf("Channel %02x base sample %04x\n",(uint16_t)i,KeyFree[i]);
    }
#endif

#if INTERRUPT_TouchKey
    EA = 1;
    while(1)
    {
        if(KeyBuf)                                                               //key_buf is non-zero, indicating that a key press was detected
        {
            printf("INT TouchKey Channel %02x \n",(uint16_t)KeyBuf);                 //Print current key status channel
            KeyBuf	= 0;                                                           //Clear key press sign
            mDelaymS(100);                                                         //Delay is meaningless, simulate single-chip to do button processing
        }
        mDelaymS(100);                                                           //Delay is meaningless, imitating microcontroller to do other things
    }
#else
    while(1)
    {
        TouchKeyChannelQuery();                                                  //Query the status of touch keys
        if(KeyBuf)                                                               //key_buf is non-zero, indicating that a key press was detected
        {
            printf("Query TouchKey Channel %02x \n",(uint16_t)KeyBuf);              //Print current key status channel
            KeyBuf = 0;                                                           //Clear key press sign
            mDelaymS(20);                                                         //Delay is meaningless, simulate single-chip to do button processing
        }
        //       mDelaymS(100);                                                           //Delay is meaningless, imitating microcontroller to do other things
    }
#endif
}
