
/********************************** (C) COPYRIGHT *******************************
* File Name          : Main.C
* Author             : WCH
* Version            : V1.1
* Date               : 2017/07/05
* Description        : CH554 Type-C use
Main mode (DFP): Supports the detection of the insertion and removal of Typc-C devices, and supports the forward and reverse insertion detection of the device,
              Support default power supply current, 1.5A power supply current and 3A power supply current setting;
Slave mode (UFP): The detection device is connected to the host, and the power supply capability of the host can be detected:
Default supply current, 1.5A supply current and 3A supply current
DFP (Downstream Facing Port) Host-end
UFP (Upstream Facing Port)   Dev-end						 				 
*******************************************************************************/

#include "ch554.h"                                                  
#include "debug.h"
#include "type_c.h"
#include "stdio.h"
#include <string.h>

#pragma  NOAREGS

void main( ) 
{
    UINT16 j = 0;
    CfgFsys( );                                                                // CH554 clock selection configuration
    mDelaymS(20);                                                              // Modify the main frequency and suggest a slight delay to wait for the chip power supply to stabilize
    mInitSTDIO( );                                                             // Serial 0 initialization
    printf("start ...\n"); 
#ifdef   TYPE_C_DFP                                                            //Type-C host detects positive and negative insertion, and informs the device of the host's power supply capability
    TypeC_DFP_Init(1);                                                         //Host power supply default current
		while(1){
			j = TypeC_DFP_Insert();
			if( j == UCC_DISCONNECT ){
				printf("UFP disconnect...\n");   
			}
			else if( j == UCC1_CONNECT ){
				printf("UFP forward insertion...\n");                                  //Forward insertion 
			}
			else if( j == UCC2_CONNECT ){
				printf("UFP reverse insertion...\n");                                  //Reverse insertion
			}
			else if( j == UCC_CONNECT ){
				printf("UFP connect...\n");   
			}
      mDelaymS(500);                                                           //延时无意义，模拟单片机执行其他操作
		}	
#endif
		
#ifdef   TYPE_C_UFP
		TypeC_UPF_PDInit();                                                        //UPF初始化
while(1){		
   j = TypeC_UPF_PDCheck();                                                    //检测主机的供电能力
   if(j == UPF_PD_Normal){
      printf("DFP defaultPower...\n");   
	 }	
   if(j == UPF_PD_1A5){
      printf("DFP 1.5A...\n");   
	 }	  
   if(j == UPF_PD_3A){
      printf("DFP 3.0...\n");   
	 }	
   if(j == UPD_PD_DISCONNECT){
      printf("disconnect...\n");   
	 }
		mDelaymS( 500 );                                                           //延时无意义，模拟单片机执行其他操作
 }
#endif
   while(1);
}