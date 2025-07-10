/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-11     chenyouqin       the first version
 */
#ifndef APPLICATIONS_SYN6288_C_
#define APPLICATIONS_SYN6288_C_
#include "syn6288.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "string.h"

;  // 手动定义u8类型
#define SYN6288_UART3       "uart3"    /* 串口设备名称 */
struct serial_configure uart3_config = RT_SERIAL_CONFIG_DEFAULT;
void SYN_FrameInfo(u8 Music, u8 *HZdata)
{

    uart3_config.baud_rate = BAUD_RATE_9600;        //修改波特率为 9600
    static rt_device_t serial;                /* 串口设备句柄 */
    serial = rt_device_find(SYN6288_UART3);
    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &uart3_config);
    rt_device_open(serial, RT_DEVICE_FLAG_INT_RX);

  unsigned  char  Frame_Info[50];
  unsigned  char  HZ_Length;
  unsigned  char  ecc  = 0;             //¶¨ÒåÐ£Ñé×Ö½Ú
  unsigned  int i = 0;
  HZ_Length = strlen((char*)HZdata);            //ÐèÒª·¢ËÍÎÄ±¾µÄ³¤¶È


  Frame_Info[0] = 0xFD ;            //¹¹ÔìÖ¡Í·FD
  Frame_Info[1] = 0x00 ;            //¹¹ÔìÊý¾ÝÇø³¤¶ÈµÄ¸ß×Ö½Ú
  Frame_Info[2] = HZ_Length + 3;        //¹¹ÔìÊý¾ÝÇø³¤¶ÈµÄµÍ×Ö½Ú
  Frame_Info[3] = 0x01 ;            //¹¹ÔìÃüÁî×Ö£ººÏ³É²¥·ÅÃüÁî
  Frame_Info[4] = 0x01 | Music << 4 ; //¹¹ÔìÃüÁî²ÎÊý£º±³¾°ÒôÀÖÉè¶¨


  for(i = 0; i < 5; i++)                //ÒÀ´Î·¢ËÍ¹¹ÔìºÃµÄ5¸öÖ¡Í·×Ö½Ú
  {
    ecc = ecc ^ (Frame_Info[i]);        //¶Ô·¢ËÍµÄ×Ö½Ú½øÐÐÒì»òÐ£Ñé
  }

  for(i = 0; i < HZ_Length; i++)        //ÒÀ´Î·¢ËÍ´ýºÏ³ÉµÄÎÄ±¾Êý¾Ý
  {
    ecc = ecc ^ (HZdata[i]);                //¶Ô·¢ËÍµÄ×Ö½Ú½øÐÐÒì»òÐ£Ñé
  }

  memcpy(&Frame_Info[5], HZdata, HZ_Length);
  Frame_Info[5 + HZ_Length] = ecc;

  rt_device_write(serial, 0, Frame_Info, 5 + HZ_Length + 1);
}



//void YS_SYN_Set(u8 *Info_data)
//{
//
//  u8 Com_Len;
//  Com_Len = strlen((char*)Info_data);
//
//  /* 发送字符串 */
//  rt_device_write(serial, 0, Info_data, Com_Len);
//}




#endif /* APPLICATIONS_SYN6288_C_ */
