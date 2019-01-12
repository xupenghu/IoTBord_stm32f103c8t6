/*
 * File      : qsdk_callback.h
 * This file is part of callback in qsdk
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-14     longmain     first version
 */
#ifndef __QSDK_CALLBACK_H__
#define __QSDK_CALLBACK_H__
#include "qsdk.h"

#define temp_objectid 					3303
#define temp_instanceid  				0
#define temp_resourceid					5700
#define temp_exec_instanceid 		0
#define temp_exec_resourceid		5605

#define hump_objectid 					3304
#define hump_instanceid  				0
#define hump_resourceid					5700
#define hump_exec_instanceid  	0
#define hump_exec_resourceid		5605

#define light0_objectid 		3311
#define light0_instanceid  	0
#define light0_resourceid		5850

#define light1_objectid 		3311
#define light1_instanceid  	1
#define light1_resourceid		5850

#define light2_objectid 		3311
#define light2_instanceid  	2
#define light2_resourceid		5850

void qsdk_rtc_set_time_callback(int year,char month,char day,char hour,char min,char sec,char week);

int qsdk_net_data_callback(int socket,char *data,int len);
int qsdk_iot_data_callback(char *data,int len);
int qsdk_gps_callback(double lon,double lat,double speed);


int qsdk_onenet_open_callback(void);
int qsdk_onenet_close_callback(void);

int qsdk_onenet_read_rsp_callback(int objectid,int instanceid,int resourceid);
int qsdk_onenet_write_rsp_callback(int objectid,int instanceid,int resourceid,int len,char* value);
int qsdk_onenet_exec_rsp_callback(int objectid,int instanceid,int resourceid,int len,char* cmd);

int reboot_callback(void);

#endif
