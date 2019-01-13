/*
 * File      : qsdk_io.c
 * This file is part of io in qsdk
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-14     longmain     first version
 */

#include "qsdk_io.h"
/*************************************************************
*	函数名称：	nb_hw_io_init
*
*	函数功能：	NB-IOT 模块控制引脚初始化
*
*	入口参数：	无
*
*	返回参数：	0 成功  1 失败
*
*	说明：
*************************************************************/
int qsdk_hw_io_init(void)
{
	rt_pin_mode(NB_POWER_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(NB_RESET_PIN, PIN_MODE_OUTPUT);
	
    //给开发板 pwrkey 引脚上电
	rt_pin_write(NB_POWER_PIN,PIN_HIGH);
    rt_thread_mdelay(500);
    //复位 NB-IOT 模块
    if(qsdk_hw_io_reboot()==RT_EOK)
        return RT_EOK;
    else return RT_ERROR;
}

/*************************************************************
*	函数名称：	qsdk_nb_hw_io_reboot
*
*	函数功能：	NB-IOT 模块重启
*
*	入口参数：	无
*
*	返回参数：	0 成功  1 失败
*
*	说明：
*************************************************************/
int qsdk_hw_io_reboot(void)
{
	rt_pin_write(NB_RESET_PIN, PIN_HIGH);
    rt_thread_mdelay(500);
    rt_pin_write(NB_RESET_PIN, PIN_LOW);
    rt_thread_mdelay(5000);
    return RT_EOK;
}
