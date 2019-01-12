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

    //给开发板 pwrkey 引脚上电
    HAL_GPIO_WritePin(NB_POWER_GPIO_Port,NB_POWER_Pin,GPIO_PIN_SET);
    rt_thread_delay(500);
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

    HAL_GPIO_WritePin(NB_RESET_GPIO_Port,NB_RESET_Pin,GPIO_PIN_SET);
    rt_thread_delay(500);
    HAL_GPIO_WritePin(NB_RESET_GPIO_Port,NB_RESET_Pin,GPIO_PIN_RESET);
    rt_thread_delay(5000);
    return RT_EOK;
}
