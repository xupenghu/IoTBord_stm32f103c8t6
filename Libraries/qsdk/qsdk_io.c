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
*	�������ƣ�	nb_hw_io_init
*
*	�������ܣ�	NB-IOT ģ��������ų�ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1 ʧ��
*
*	˵����
*************************************************************/
int qsdk_hw_io_init(void)
{

    //�������� pwrkey �����ϵ�
    HAL_GPIO_WritePin(NB_POWER_GPIO_Port,NB_POWER_Pin,GPIO_PIN_SET);
    rt_thread_delay(500);
    //��λ NB-IOT ģ��
    if(qsdk_hw_io_reboot()==RT_EOK)
        return RT_EOK;
    else return RT_ERROR;
}

/*************************************************************
*	�������ƣ�	qsdk_nb_hw_io_reboot
*
*	�������ܣ�	NB-IOT ģ������
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1 ʧ��
*
*	˵����
*************************************************************/
int qsdk_hw_io_reboot(void)
{

    HAL_GPIO_WritePin(NB_RESET_GPIO_Port,NB_RESET_Pin,GPIO_PIN_SET);
    rt_thread_delay(500);
    HAL_GPIO_WritePin(NB_RESET_GPIO_Port,NB_RESET_Pin,GPIO_PIN_RESET);
    rt_thread_delay(5000);
    return RT_EOK;
}