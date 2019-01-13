/*
 * File      : main.c
 * This file is part of main.c in qsdk
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-14     longmain     first version
 */

#define LOG_TAG              "[main]"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>
#include <rtthread.h>
#include "hal_rgb_led.h"
#include "hal_temp_hum.h"
#include "qsdk.h"
#include "main.h"
//引入C库
#include <stdlib.h>
#include "stdio.h"
#include <string.h>


//定义任务控制块
static rt_thread_t led_thread_id=RT_NULL;
static rt_thread_t net_thread_id=RT_NULL;
static rt_thread_t dht11_thread_id=RT_NULL;
static rt_thread_t rtc_thread_id=RT_NULL;
static rt_thread_t data_thread_id=RT_NULL;
static rt_thread_t key_thread_id=RT_NULL;											 
											 
											 
void led1_thread_entry(void* parameter);
void net_thread_entry(void* parameter);
void dht11_thread_entry(void* parameter);
void rtc_thread_entry(void* parameter);
void data_thread_entry(void* parameter);
void key_thread_entry(void* parameter);

int min_temp=10,max_temp=50;
int min_hump=10,max_hump=50;

char temp[10]="";
char hump[10]="";
char exec_str[50]="7104f10400000000f20100f30100f40100";

//使用IOT的情况下，上报数据用的数组
char update[100]="";

//使用NET的情况下，上报信息标志
char net_update_status=0;

#ifdef QSDK_USING_ONENET
/********************************************
***     DEVICE成员值
***
*** 1  objectid         	object id
*** 2  instancecount    	instance 数量
*** 3  instancebitmap   	instance bitmap
*** 4  attributecount   	attribute count (具有Read/Write操作的object有attribute)
*** 5  actioncount      	action count (具有Execute操作的object有action)
*** 6  instanceid       	instance id
*** 7  resourceid					resource id
*** 8  valuetype					数据类型
*** 9  len								数据长度
*** 10 flge								消息标志:默认为0
*** 11 msgid							消息ID:默认为0
*** 12 up_status					数据上报开关
*** 13 value							数据值
*********************************************/
#ifdef QSDK_USING_ME3616
DEVICE onenet_device[]={											 
											 {temp_objectid,1,"1",1,1,temp_instanceid,temp_resourceid,qsdk_onenet_value_Float,0,0,0,0,temp},
											 {temp_objectid,1,"1",1,1,temp_exec_instanceid,temp_exec_resourceid,qsdk_onenet_value_Opaque,0,0,0,0,"012345"},
											 {hump_objectid,1,"1",1,1,hump_instanceid,hump_resourceid,qsdk_onenet_value_Float,0,0,0,0,hump},
											 {hump_objectid,1,"1",1,1,hump_exec_instanceid,hump_exec_resourceid,qsdk_onenet_value_Opaque,0,0,0,0,"012345"},
//											 {light0_objectid,3,"111",1,0,light0_instanceid,light0_resourceid,qsdk_onenet_value_Bool,0,0,0,0,led1_status},
//											 {light1_objectid,3,"111",1,0,light1_instanceid,light1_resourceid,qsdk_onenet_value_Bool,0,0,0,0,led2_status},
//											 {light2_objectid,3,"111",1,0,light2_instanceid,light2_resourceid,qsdk_onenet_value_Bool,0,0,0,0,led3_status},
											 
											 };
	
#else 
DEVICE onenet_device[]={											 
											 {temp_objectid,2,"11",1,1,temp_instanceid,temp_resourceid,qsdk_onenet_value_Float,0,0,0,0,temp},
											 {temp_objectid,2,"11",1,1,temp_exec_instanceid,temp_exec_resourceid,qsdk_onenet_value_Opaque,0,0,0,0,"012345"},
											 {hump_objectid,2,"11",1,1,hump_instanceid,hump_resourceid,qsdk_onenet_value_Float,0,0,0,0,hump},
											 {hump_objectid,2,"11",1,1,hump_exec_instanceid,hump_exec_resourceid,qsdk_onenet_value_Opaque,0,0,0,0,"012345"},
											 {light0_objectid,3,"111",1,0,light0_instanceid,light0_resourceid,qsdk_onenet_value_Bool,0,0,0,0,led1_status},
											 {light1_objectid,3,"111",1,0,light1_instanceid,light1_resourceid,qsdk_onenet_value_Bool,0,0,0,0,led2_status},
											 {light2_objectid,3,"111",1,0,light2_instanceid,light2_resourceid,qsdk_onenet_value_Bool,0,0,0,0,led3_status},
											 
											 };
#endif

#endif
/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
	
	led_thread_id=rt_thread_create("led_thread",
																 led1_thread_entry,
																 RT_NULL,
																 512,
																 22,
																 20);
		if(led_thread_id!=RT_NULL)
			rt_thread_startup(led_thread_id);
		else
			return -1;
	net_thread_id=rt_thread_create("net_thread",
																 net_thread_entry,
																 RT_NULL,
																 512,
																 10,
																 20);
		if(net_thread_id!=RT_NULL)
			rt_thread_startup(net_thread_id);
		else
			return -1;
	dht11_thread_id=rt_thread_create("dht11_thread",
																 dht11_thread_entry,
																 RT_NULL,
																 512,
																 19,
																 20);
		if(dht11_thread_id!=RT_NULL)
			rt_thread_startup(dht11_thread_id);
		else
			return -1;
	rtc_thread_id=rt_thread_create("rtc_thread",
																 rtc_thread_entry,
																 RT_NULL,
																 512,
																 18,
																 20);
		if(rtc_thread_id!=RT_NULL)
			rt_thread_startup(rtc_thread_id);
		else
			return -1;
	data_thread_id=rt_thread_create("data_thread",
																 data_thread_entry,
																 RT_NULL,
																 512,
																 12,
																 50);
		if(data_thread_id!=RT_NULL)
			rt_thread_startup(data_thread_id);
		else
			return -1;
		
//	key_thread_id=rt_thread_create("key_thread",
//																 key_thread_entry,
//																 RT_NULL,
//																 512,
//																 5,
//																 20);
//		if(key_thread_id!=RT_NULL)
//			rt_thread_startup(key_thread_id);
//		else
//			return -1;
	
}

 at_response_t resp = RT_NULL;
int result = 0;

void led1_thread_entry(void* parameter)
{
	rgbLedInit();
	ledRgbControl(0,0,0);
	while(1)
	{
		if(nb_device.net_connect_ok==0)
		{
			ledRgbControl(0,20,0);
			rt_thread_mdelay(200);
			ledRgbControl(0,0,0);
			rt_thread_mdelay(800);
		}
		else
		{
			ledRgbControl(20,0,0);
			rt_thread_mdelay(500);
			ledRgbControl(0,0,0);
			rt_thread_mdelay(500);
		}
	}
}


void net_thread_entry(void* parameter)
{
	int i=0;
	int net_socket;
	if(qsdk_nb_hw_init()!=RT_EOK)
	{
		LOG_D("module init failure\r\n");
		goto fail;
	}
#ifdef QSDK_USING_IOT

#ifdef QSDK_USING_ME3616
if(qsdk_iot_reg()==RT_EOK)
			LOG_D("iot reg success\r\n");
		else
		{
			LOG_D("iot reg failure\r\n");
			goto fail;
		}
		rt_thread_mdelay(1000);

#endif
	
#endif


#ifdef QSDK_USING_ONENET

#ifdef QSDK_USING_ME3616
		if(qsdk_onenet_init(onenet_device,sizeof(onenet_device)/sizeof(onenet_device[0]),3000)!=RT_EOK)	
#endif
	
#if (defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
		if(qsdk_onenet_init(onenet_device,sizeof(onenet_device)/sizeof(onenet_device[0]),3000)!=RT_EOK)	
#endif
		{
				LOG_D(" error=%d onenet init failure \r\n",data_stream.error);
				goto fail;
		}

		
#endif	
#ifdef QSDK_USING_ME3616_GPS
	if(qsdk_agps_config()!=RT_EOK)
	{
		LOG_D(" gps config failure\r\n");
		goto fail;
	}
	else LOG_D("gps run success\r\n");
#endif
	while(1)
	{
#ifdef QSDK_USING_ONENET
		//维持设备在线时间
		if(qsdk_time_onenet_update(0)!=RT_EOK)
		{
			LOG_D("qsdk update time failure\r\n");
			goto fail;
		}
		LOG_D("qsdk update time success\r\n");
		
#endif

#ifdef QSDK_USING_NET		
			if(i==0)
			{		
				i=1;	
#if (defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)	
					// 创建网络套子号
				if(qsdk_net_create_socket(2,5688,"47.92.236.118",9013)!=RT_EOK)	
				{
					LOG_D("\r\n create net socket failure");
					goto fail;
				}
				LOG_D("\r\n create net socket success");
				net_update_status=1;
			}

#elif (defined QSDK_USING_ME3616)


				if(qsdk_net_create_socket(QSDK_NET_TYPE_UDP,&net_socket)!=RT_EOK)
				{
					LOG_D("create net socket failure\r\n");
					goto fail;
				}
				LOG_D("crete net socket success\r\n");
				LOG_D("socket=%d\r\n",net_socket);
				if(qsdk_net_connect_to_server(net_socket,"47.92.236.118",9013)!=RT_EOK)
				{
					LOG_D("connect server failure\r\n");
					goto fail;
				}
				LOG_D("connect server success\r\n");
				if(qsdk_net_send_data(net_socket,"1234",2)!=RT_EOK)
				{
					LOG_D("net send data failure\r\n");
				}
				LOG_D("net send data success\r\n");
			}
			rt_thread_mdelay(30000);
#endif
#endif
#ifdef QSDK_USING_IOT

#if (defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
		if(nb_device.net_connect_ok)
			rt_thread_mdelay(5000);
			if(qsdk_iot_send_date("1234567890")!=RT_EOK)
			{
				LOG_D("iot updte failure\r\n");
				goto fail;
			}
			LOG_D("iot updte success\r\n");
		rt_thread_mdelay(15000);
#endif

#ifdef QSDK_USING_ME3616

		
		LOG_D("nb_init OK\r\n");
		if(nb_device.net_connect_ok)
		{
				if(qsdk_iot_update("1234567890,,,!!!...，，，。。。！！！")!=RT_EOK)
					LOG_D("iot update failure\r\n");
			
		}	
		rt_thread_mdelay(30000);
#endif

#endif
			
			
		//判断当前网络是否在线，如果不在线则关闭任务
		if(qsdk_nb_get_net_start()!=RT_EOK)
		{
			LOG_D("get net status failure\r\n");
			if(qsdk_nb_get_net_start()!=RT_EOK)
			{
				LOG_D("get net status failure\r\n");
				
				//挂起数据上报任务
				rt_thread_suspend(data_thread_id);
				goto fail;
				
			}
		}
		//如果当前网络为断开状态
		if(nb_device.net_connect_ok==0)
		{
			LOG_D("net connect failure\r\n");
			//挂起数据上报任务
			rt_thread_suspend(data_thread_id);
			goto fail;
		}
		LOG_D("net connect success\r\n");
		rt_thread_mdelay(200000);
	}
fail:
	LOG_D("net thread close success\r\n");
	rt_thread_delete(net_thread_id);
	rt_schedule();
}

void dht11_thread_entry(void* parameter)
{
	uint8_t current_temp, current_hum;
	LOG_D("dht11 task is open\r\n");
	dht11Init();
	while(1)
	{
		dht11Read(&current_temp,&current_hum);
		sprintf(temp,"%0.2f",(float)DHT11Info.tempreture);
		sprintf(hump,"%0.2f",(float)DHT11Info.humidity);

		rt_thread_mdelay(1000);
	}
}

void rtc_thread_entry(void* parameter)
{
	LOG_D("RTC task is open\r\n");
	while(1)
	{
		if(nb_device.net_connect_ok)
		{
			rt_thread_mdelay(1000);
//			rt_enter_critical();
//#ifdef QSDK_USING_IOT		
//			//把需要上传到平台的参数进行封装
//			memset(update,0,sizeof(update));
//			sprintf(update,"temp=%0.2f,hump=%0.2f",sht20Info.tempreture,sht20Info.humidity);
//#endif
//			HAL_RTC_GetTime(&hrtc,&RTC_TimeStructure,RTC_FORMAT_BIN);
//			HAL_RTC_GetDate(&hrtc,&RTC_DateStructure,RTC_FORMAT_BIN);
//		
//			LOG_D("\r\ndata:20%02d-%02d-%02d,time:%02d:%02d:%02d\r\n",RTC_DateStructure.Year,RTC_DateStructure.Month,RTC_DateStructure.Date,RTC_TimeStructure.Hours,RTC_TimeStructure.Minutes,RTC_TimeStructure.Seconds);
//			LOG_D("温度：%0.1f  湿度：%0.1f\r\n",sht20Info.tempreture,sht20Info.humidity);
//			rt_exit_critical();
		}
		else 
		{
			rt_thread_mdelay(1000);
		}
	}
}

void data_thread_entry(void* parameter)
{

	LOG_D("data task is open\r\n");
	while(1)
	{
#ifdef QSDK_USING_ONENET
		//判断onenet时候连接成功，连接成功便上报所有数据
		if(data_stream.connect_status==qsdk_onenet_status_success)
		{
			LOG_D("data UP is open\r\n");	
			if(qsdk_notify_data_to_onenet(0)==RT_EOK)
					LOG_D("notify data success\r\n ");
			else	LOG_D("notify data failure\r\n ");
			
		}
#endif
	
#ifdef QSDK_USING_IOT
	if(nb_device.net_connect_ok)
		{
			if(qsdk_iot_send_date(update)!=RT_EOK)
			{
				LOG_D("iot updte failure\r\n");
				goto fail;
			}
			LOG_D("iot updte success\r\n");
		}
#endif

#ifdef QSDK_USING_NET

			if(net_update_status)
			{
			//发送数据到服务器， 此处为字符串，程序中已经转换为HEX
				if(qsdk_net_send_data(2,"1234567890",10)!=RT_EOK)
				{
					LOG_D("\r\n send net data failure");	
					goto fail;
				}
				LOG_D("\r\n send net data success");
			}

#endif
		rt_thread_mdelay(20000);
	}
fail:
	LOG_D("data thread close success\r\n");
	rt_thread_delete(data_thread_id);
	rt_schedule();
	
}

void key_thread_entry(void* parameter)
{

	LOG_D("key task is open\r\n");
	while(1)
	{
		//qsdk_hw_key_Handle((keysTypedef_t *)&keys);
		rt_thread_mdelay(1000);
#ifdef QSDK_USING_ONENET

#if (defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
//判断是否有需要上报的数据，有的话上报数据
		if(data_stream.update_status)
		{
			if(qsdk_notify_data_to_onenet(1)==RT_EOK)
				LOG_D("notify ack data success\r\n ");
			else	LOG_D("notify ack data failure\r\n ");
		}
#endif
		
#endif
	}
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: LOG_D("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
