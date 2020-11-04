
/********************************** (C) COPYRIGHT *******************************
* File Name          : USBHostHUB_KM.C
* Author             : WCH
* Version            : V2.0
* Date               : 2018/07/24
*******************************************************************************/

#include <ch554.h>
#include <debug.h>
#include "usbhost.h"
#include <ch554_usb.h>
#include <stdio.h>
#include <string.h>

__code uint8_t  SetupGetDevDescr[] = { USB_REQ_TYP_IN, USB_GET_DESCRIPTOR, 0x00, USB_DESCR_TYP_DEVICE, 0x00, 0x00, sizeof( USB_DEV_DESCR ), 0x00 };
__code uint8_t  SetupGetCfgDescr[] = { USB_REQ_TYP_IN, USB_GET_DESCRIPTOR, 0x00, USB_DESCR_TYP_CONFIG, 0x00, 0x00, 0x04, 0x00 };
__code uint8_t  SetupSetUsbAddr[] = { USB_REQ_TYP_OUT, USB_SET_ADDRESS, USB_DEVICE_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00 };
__code uint8_t  SetupSetUsbConfig[] = { USB_REQ_TYP_OUT, USB_SET_CONFIGURATION, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
__code uint8_t  SetupSetUsbInterface[] = { USB_REQ_RECIP_INTERF, USB_SET_INTERFACE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
__code uint8_t  SetupClrEndpStall[] = { USB_REQ_TYP_OUT | USB_REQ_RECIP_ENDP, USB_CLEAR_FEATURE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
__code uint8_t  SetupGetHubDescr[] = { HUB_GET_HUB_DESCRIPTOR, HUB_GET_DESCRIPTOR, 0x00, USB_DESCR_TYP_HUB, 0x00, 0x00, sizeof( USB_HUB_DESCR ), 0x00 };
__code uint8_t  SetupSetHIDIdle[]= { 0x21,HID_SET_IDLE,0x00,0x00,0x00,0x00,0x00,0x00 };
__code uint8_t  SetupGetHIDDevReport[] = { 0x81, USB_GET_DESCRIPTOR, 0x00, USB_DESCR_TYP_REPORT, 0x00, 0x00, 0xFF, 0x00 };
__code uint8_t  XPrinterReport[] = { 0xA1, 0, 0x00, 0, 0x00, 0x00, 0xF1, 0x03 };
__code uint8_t  GetProtocol[] = { 0xc0,0x33,0x00,0x00,0x00,0x00,0x02,0x00 };
__code uint8_t  TouchAOAMode[] = { 0x40,0x35,0x00,0x00,0x00,0x00,0x00,0x00 };

__code uint8_t  Sendlen[]= {0,4,16,35,39,53,67};
__code uint8_t  StringID[] = {'W','C','H',0x00,                                                                                //manufacturer name
                      'W','C','H','U','A','R','T','D','e','m','o',0x00,                                   //model name
                      0x57,0x43,0x48,0x20,0x41,0x63,0x63,0x65,0x73,0x73,0x6f,0x72,0x79,0x20,0x54,0x65,0x73,0x74,0x00,     //description
                      '1','.','0',0x00 ,                                                                       //version
                      0x68,0x74,0x74,0x70,0x3a,0x2f,0x2f,0x77,0x63,0x68,0x2e,0x63,0x6e,0,//URI
                      0x57,0x43,0x48,0x41,0x63,0x63,0x65,0x73,0x73,0x6f,0x72,0x79,0x31,0x00                               //serial number
                     };  
__code uint8_t  SetStringID[]= {0x40,0x34,0x00,0x00,0x00,0x00,0x04,0x00,
                        0x40,0x34,0x00,0x00,0x01,0x00,12,0x00,
                        0x40,0x34,0x00,0x00,0x02,0x00,19,0x00,
                        0x40,0x34,0x00,0x00,0x03,0x00,4,0x00,
                        0x40,0x34,0x00,0x00,0x04,0x00,0x0E,0x00,
                        0x40,0x34,0x00,0x00,0x05,0x00,0x0E,0x00
                       };

__xdata uint8_t  UsbDevEndp0Size;                                                       //* USB设备的端点0的最大包尺寸 */
__xdata __at (0x0000) uint8_t  RxBuffer[ MAX_PACKET_SIZE ];                              // IN, must even address
__xdata __at (0x0040) uint8_t  TxBuffer[ MAX_PACKET_SIZE ];                            // OUT, must even address

uint8_t Set_Port = 0;

__xdata _RootHubDev ThisUsbDev;                                                   //ROOT口
__xdata _DevOnHubPort DevOnHubPort[HUB_MAX_PORTS];                                // 假定:不超过1个外部HUB,每个外部HUB不超过HUB_MAX_PORTS个端口(多了不管)

__bit RootHubId;                                                                  // 当前正在操作的root-hub端口号:0=HUB0,1=HUB1
__bit FoundNewDev;

void main( )
{
    uint8_t   i, s,k, len, endp;
    uint16_t  loc;
	CfgFsys( );	
	mDelaymS(50);
    mInitSTDIO( );                                                              //为了让计算机通过串口监控演示过程
    printf( "Start @ChipID=%02X\n", (uint16_t)CHIP_ID );
    InitUSB_Host( );
    FoundNewDev = 0;
    printf( "Wait Device In\n" );
    while ( 1 )
    {
        s = ERR_SUCCESS;
        if ( UIF_DETECT ){                                                       // 如果有USB主机检测中断则处理
            UIF_DETECT = 0;                                                      // 清中断标志			
            s = AnalyzeRootHub( );                                               // 分析ROOT-HUB状态
            if ( s == ERR_USB_CONNECT ) FoundNewDev = 1;						
        }
        if ( FoundNewDev ){                                                      // 有新的USB设备插入
            FoundNewDev = 0;
//          mDelaymS( 200 );                                                     // 由于USB设备刚插入尚未稳定,故等待USB设备数百毫秒,消除插拔抖动
            s = EnumAllRootDevice( );                                            // 枚举所有ROOT-HUB端口的USB设备
            if ( s != ERR_SUCCESS ){						
                printf( "EnumAllRootDev err = %02X\n", (uint16_t)s );					
            }
        }

		/* 如果CH554下端连接的是HUB，则先枚举HUB */
		s = EnumAllHubPort( );                                                // 枚举所有ROOT-HUB端口下外部HUB后的二级USB设备
		if ( s != ERR_SUCCESS ){                                              // 可能是HUB断开了
			printf( "EnumAllHubPort err = %02X\n", (uint16_t)s );
		}

		/* 如果设备是鼠标 */
		loc = SearchTypeDevice( DEV_TYPE_MOUSE );                             // 在ROOT-HUB以及外部HUB各端口上搜索指定类型的设备所在的端口号
		if ( loc != 0xFFFF ){                                                 // 找到了,如果有两个MOUSE如何处理?
			printf( "Query Mouse @%04X\n", loc );
			i = (uint8_t)( loc >> 8 );
			len = (uint8_t)loc;
			SelectHubPort( len );                                             // 选择操作指定的ROOT-HUB端口,设置当前USB速度以及被操作设备的USB地址
			endp = len ? DevOnHubPort[len-1].GpVar[0] : ThisUsbDev.GpVar[0];        // 中断端点的地址,位7用于同步标志位
			if ( endp & USB_ENDP_ADDR_MASK ){                                 // 端点有效
				s = USBHostTransact( USB_PID_IN << 4 | endp & 0x7F, endp & 0x80 ? bUH_R_TOG | bUH_T_TOG : 0, 0 );// CH554传输事务,获取数据,NAK不重试
				if ( s == ERR_SUCCESS ){
					endp ^= 0x80;                                             // 同步标志翻转
					if ( len ) DevOnHubPort[len-1].GpVar[0] = endp;              // 保存同步标志位
					else ThisUsbDev.GpVar[0] = endp;
					len = USB_RX_LEN;                                         // 接收到的数据长度
					if ( len ) {
						printf("Mouse data: ");
						for ( i = 0; i < len; i ++ ){
							printf("x%02X ",(uint16_t)(RxBuffer[i]) );
						}
						printf("\n");
					}
				}
				else if ( s != ( USB_PID_NAK | ERR_USB_TRANSFER ) ) {
					printf("Mouse error %02x\n",(uint16_t)s);                   // 可能是断开了
				}
			}
			else {
				printf("Mouse no interrupt endpoint\n");
			}
			SetUsbSpeed( 1 );                                                 // 默认为全速
		}


		/* 如果设备是键盘 */
		loc = SearchTypeDevice( DEV_TYPE_KEYBOARD );                          // 在ROOT-HUB以及外部HUB各端口上搜索指定类型的设备所在的端口号
		if ( loc != 0xFFFF ){                                                 // 找到了,如果有两个KeyBoard如何处理?
			printf( "Query Keyboard @%04X\n", loc );
			i = (uint8_t)( loc >> 8 );
			len = (uint8_t)loc;
			SelectHubPort( len );                                             // 选择操作指定的ROOT-HUB端口,设置当前USB速度以及被操作设备的USB地址
			endp = len ? DevOnHubPort[len-1].GpVar[0] : ThisUsbDev.GpVar[0];        // 中断端点的地址,位7用于同步标志位
			printf("%02X  ",endp);
			if ( endp & USB_ENDP_ADDR_MASK ){                                 // 端点有效
				s = USBHostTransact( USB_PID_IN << 4 | endp & 0x7F, endp & 0x80 ? bUH_R_TOG | bUH_T_TOG : 0, 0 );// CH554传输事务,获取数据,NAK不重试
				if ( s == ERR_SUCCESS ){
					endp ^= 0x80;                                             // 同步标志翻转
					if ( len ) DevOnHubPort[len-1].GpVar[0] = endp;              // 保存同步标志位
					else ThisUsbDev.GpVar[0] = endp;
					len = USB_RX_LEN;                                         // 接收到的数据长度
					if ( len ){
						SETorOFFNumLock(RxBuffer);
						printf("keyboard data: ");
						for ( i = 0; i < len; i ++ ){
							printf("x%02X ",(uint16_t)(RxBuffer[i]) );
						}
						printf("\n");
					}
				}
				else if ( s != ( USB_PID_NAK | ERR_USB_TRANSFER ) ){
					printf("keyboard error %02x\n",(uint16_t)s);               // 可能是断开了
				}
			}
			else{
				printf("keyboard no interrupt endpoint\n");
			}
			SetUsbSpeed( 1 );                                                // 默认为全速
		}

		/* 操作USB打印机 */
		if(TIN0 == 0){                                                          //P10为低，开始打印
			memset(TxBuffer,0,sizeof(TxBuffer));
			TxBuffer[0]=0x1B;TxBuffer[1]=0x40;TxBuffer[2]=0x1D;TxBuffer[3]=0x55;TxBuffer[4]=0x42;TxBuffer[5]=0x02;TxBuffer[6]=0x18;TxBuffer[7]=0x1D;
			TxBuffer[8]=0x76;TxBuffer[9]=0x30;TxBuffer[10]=0x00;TxBuffer[11]=0x30;TxBuffer[12]=0x00;TxBuffer[13]=0x18;
			loc = SearchTypeDevice( USB_DEV_CLASS_PRINTER );                       // 在ROOT-HUB以及外部HUB各端口上搜索指定类型的设备所在的端口号
			if ( loc != 0xFFFF ){                                                  // 找到了,如果有两个打印机如何处理?
				printf( "Query Printer @%04X\n", loc );
				i = (uint8_t)( loc >> 8 );
				len = (uint8_t)loc;
				SelectHubPort( len );                                              // 选择操作指定的ROOT-HUB端口,设置当前USB速度以及被操作设备的USB地址
				endp = len ? DevOnHubPort[len-1].GpVar[0] : ThisUsbDev.GpVar[0];         // 端点的地址,位7用于同步标志位
				printf("%02X  ",endp);  
				if ( endp & USB_ENDP_ADDR_MASK ){                                  // 端点有效
					UH_TX_LEN = 64;                                                // 默认无数据故状态阶段为IN										
					s = USBHostTransact( USB_PID_OUT << 4 | endp & 0x7F, endp & 0x80 ? bUH_R_TOG | bUH_T_TOG : 0, 0xffff );// CH554传输事务,获取数据,NAK重试
					if ( s == ERR_SUCCESS ){
						endp ^= 0x80;                                               // 同步标志翻转
						memset(TxBuffer,0,sizeof(TxBuffer));
						UH_TX_LEN = 64;                                             // 默认无数据故状态阶段为IN										
						s = USBHostTransact( USB_PID_OUT << 4 | endp & 0x7F, endp & 0x80 ? bUH_R_TOG | bUH_T_TOG : 0, 0xffff );// CH554传输事务,获取数据,NAK重试   											
					}
					else if ( s != ( USB_PID_NAK | ERR_USB_TRANSFER ) ) printf("Printer error %02x\n",(uint16_t)s); // 可能是断开了
				}
			}
		}				

		/* 操作HID复合设备 */
		loc = SearchTypeDevice( USB_DEV_CLASS_HID );                          // 在ROOT-HUB以及外部HUB各端口上搜索指定类型的设备所在的端口号			
		if ( loc != 0xFFFF ){                                                 // 找到了
			printf( "Query USB_DEV_CLASS_HID @%04X\n", loc );	
			loc = (uint8_t)loc;                                                 //554只有一个USB，只需低八位即可

			for(k=0;k!=4;k++)
			{	
				//端点是否有效？
				endp = loc ? DevOnHubPort[loc-1].GpVar[k] : ThisUsbDev.GpVar[k];        // 中断端点的地址,位7用于同步标志位			
				if ( (endp & USB_ENDP_ADDR_MASK) == 0 ) break;

				printf("endp: %02X\n",(uint16_t)endp);
				SelectHubPort( loc );                                             // 选择操作指定的ROOT-HUB端口,设置当前USB速度以及被操作设备的USB地址
				s = USBHostTransact( USB_PID_IN << 4 | endp & 0x7F, endp & 0x80 ? bUH_R_TOG | bUH_T_TOG : 0, 0 );// CH554传输事务,获取数据,NAK不重试
				if ( s == ERR_SUCCESS ){
					endp ^= 0x80;                                             // 同步标志翻转
					if ( loc ) DevOnHubPort[loc-1].GpVar[k] = endp;              // 保存同步标志位
					else ThisUsbDev.GpVar[k] = endp;
					len = USB_RX_LEN;                                         // 接收到的数据长度
					if ( len ){
						printf("keyboard data: ");
						for ( i = 0; i < len; i ++ ){
							printf("x%02X ",(uint16_t)(RxBuffer[i]) );
						}
						printf("\n");
					}
				}
				else if ( s != ( USB_PID_NAK | ERR_USB_TRANSFER ) ){
					printf("keyboard error %02x\n",(uint16_t)s);               // 可能是断开了
				}

			}								
			SetUsbSpeed( 1 );                                                // 默认为全速
		}            					


		/* 操作厂商设备，可能是手机，会先尝试以AOA方式启动 */
		loc = SearchTypeDevice( DEF_AOA_DEVICE );                            // 查找AOA
		if ( loc != 0xFFFF ){                                                 // 找到了	
			loc = (uint8_t)loc;                                                 //目前USBHOST.C仅支持ROOTHUB下的Android操作,不用分析loc

			endp = ThisUsbDev.GpVar[0];                                       //准备对上传端点发IN包		
			if ( (endp & USB_ENDP_ADDR_MASK) != 0 )                           //端点有效
			{
				SelectHubPort( 0 );                                             // 选择操作指定的ROOT-HUB端口,设置当前USB速度以及被操作设备的USB地址
				s = USBHostTransact( USB_PID_IN << 4 | endp & 0x7F, endp & 0x80 ? bUH_R_TOG | bUH_T_TOG : 0, 0 );// CH554传输事务,获取数据,NAK不重试
				if ( s == ERR_SUCCESS ){
					endp ^= 0x80;                                             // 同步标志翻转  
					ThisUsbDev.GpVar[0] = endp;                               // 保存同步标志位
					len = USB_RX_LEN;                                         // 接收到的数据长度

					for ( i = 0; i < len; i ++ ){
						printf("x%02X ",(uint16_t)(RxBuffer[i]) );
					}
					printf("\n");					
					if ( len ){

						memcpy(TxBuffer,RxBuffer,len);                         //回传
						endp = ThisUsbDev.GpVar[2];                            //下传端点发OUT包
						UH_TX_LEN = len; 
						s = USBHostTransact( USB_PID_OUT << 4 | endp & 0x7F, endp & 0x80 ? bUH_R_TOG | bUH_T_TOG : 0, 0xffff ); //无限次重试下传
						if(s == ERR_SUCCESS)
						{
							endp ^= 0x80;                                       // 同步标志翻转  
							ThisUsbDev.GpVar[2] = endp;                         // 保存同步标志位						
							printf("send back\n");
						}

					}
				}
				else if ( s != ( USB_PID_NAK | ERR_USB_TRANSFER ) ){
					printf("transmit error %02x\n",(uint16_t)s);               // 可能是断开了
				}					
			}					
			SetUsbSpeed( 1 );                                                // 默认为全速
		} 


    }
}
