/*
 * File      : qsdk_config.c
 * This file is part of config in qsdk
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-14     longmain     first version
 */

#ifndef __QSDK_CONFIG_H__

#define __QSDK_CONFIG_H__

/*********************************
*	NB-IOT模块以及功能选择
*********************************/
//#define QSDK_USING_M5310

//#define QSDK_USING_M5310A

#define QSDK_USING_ME3616

//模块串口选择
#define NB_COMM_PORT        "uart2"

//AT命令接收缓冲
#define AT_CLIENT_RECV_BUFF_LEN 512

/*********************************
*
*	ONENET 配置选项
*
*********************************/
//开启ONENET支持功能
#define QSDK_USING_ONENET

//定义objectID最大数量
#define QSDK_MAX_OBJECT_COUNT		10

//onenet服务器配置信息
#ifdef QSDK_USING_ME3616
#define QSDK_ONENET_ADDRESS      "183.230.40.39"
#define QSDK_USING_ME3616_GPS
#else
#define QSDK_ONENET_ADDRESS      "183.230.40.40"
#endif
#define QSDK_ONENET_PORT			"5683"

/*********************************
*
*	IOT配置选项
*
*********************************/

//开启电信IOT支持功能
//#define QSDK_USING_IOT
#ifdef QSDK_USING_IOT
#define QSDK_IOT_ADDRESS      "117.60.157.137"
#else
#define QSDK_IOT_ADDRESS      "0.0.0.0"
#endif
#define QSDK_IOT_PORT					"5683"

//ME3616注册超时时间
#define QSDK_IOT_REG_TIME_OUT			90

/*********************************
*
*	NET配置选项
*
*********************************/
//开启UDP/TCP功能
//#define QSDK_USING_NET

/*********************************
*
*	SYS配置选项
*
*********************************/
//开启LOG显示
#define QSDK_USING_LOG

//开启调试模式
#define QSDK_USING_DBUG
#define QSDK_USING_DEBUD
//定义TCP/UDP最大接收长度
#define QSDK_NET_REV_MAX_LEN		512

//定义模组最大发送数据长度
#define QSDK_CMD_REV_MAX_LEN		256

//定义时区差
#define QSDK_TIME_ZONE  8


#endif	//qsdk_config.h end


