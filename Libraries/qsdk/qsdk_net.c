/*
 * File      : qsdk_net.c
 * This file is part of net in qsdk
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-14     longmain     first version
 */
#define LOG_TAG              "qsdk_net"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>
#include "qsdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if	(defined QSDK_USING_NET)||(defined QSDK_USING_IOT)
char nb_buffer[QSDK_NET_REV_MAX_LEN];
#endif

#ifdef QSDK_USING_NET

#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
struct NET_INFO  net_info;
#endif

#endif


#ifdef QSDK_USING_NET

extern at_response_t at_resp;

#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
/*************************************************************
*	函数名称：	qsdk_net_create_socket
*
*	函数功能：	创建网络套子号 socket
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_net_create_socket(int type,int port,char *server_ip,unsigned short server_port)
{
    rt_memset(&net_info,0,sizeof(net_info));
    rt_strncpy(net_info.server_ip,server_ip,strlen(server_ip));
    net_info.server_port=server_port;
#ifdef QSDK_USING_DEBUD
    LOG_D("\r\n sever ip=%s ,sever port=%d",net_info.server_ip,net_info.server_port);
#endif
    if(type==1)
    {
#ifdef QSDK_USING_LOG
        LOG_D("AT+NSOCR=STREAM,6,%d,1",port);
#endif
        if(at_exec_cmd(at_resp,"AT+NSOCR=STREAM,6,%d,1",port)!=RT_EOK) return RT_ERROR;
    }
    else if(type==2)
    {
#ifdef QSDK_USING_LOG
        LOG_D("AT+NSOCR=DGRAM,17,%d,1",port);
#endif
        if(at_exec_cmd(at_resp,"AT+NSOCR=DGRAM,17,%d,1",port)!=RT_EOK)
            return RT_ERROR;
    }
    else return RT_ERROR;

    at_resp_parse_line_args(at_resp,2,"%d",&net_info.socket);
    return RT_EOK;
}
/*************************************************************
*	函数名称：	qsdk_net_connect_to_server
*
*	函数功能：	连接到TCP 服务器
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_net_connect_to_server(void)
{
    if(at_exec_cmd(at_resp,"AT+NSOCO=%d,%s,%d",net_info.socket,net_info.server_ip,net_info.server_port)!=RT_EOK) return RT_ERROR;

    return RT_EOK;
}
/*************************************************************
*	函数名称：	qsdk_net_send_data
*
*	函数功能：	发送数据到服务器
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_net_send_data(int type,char *str,unsigned int len)
{
    if(str==NULL||len>QSDK_NET_REV_MAX_LEN)
    {
#ifdef QSDK_USING_LOG
        LOG_D("\r\ndata too long");
#endif
        return RT_ERROR;
    }
    string_to_hex(str,len,nb_buffer);

    if(type==1)
    {
        if(at_exec_cmd(at_resp,"AT+NSOSD=%d,%d,%s",net_info.socket,len,nb_buffer)!=RT_EOK)
        {
            return RT_ERROR;
        }
        return RT_EOK;
    }
    else if(type==2)
    {
        if(at_exec_cmd(at_resp,"AT+NSOST=%d,%s,%d,%d,%s",net_info.socket,net_info.server_ip,net_info.server_port,len,nb_buffer)!=RT_EOK)
        {
            return RT_ERROR;
        }
        return RT_EOK;
    }
    else
    {
        return RT_ERROR;
    }
}
/*************************************************************
*	函数名称：	qsdk_net_rev_data
*
*	函数功能：	接收服务器返回的数据
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_net_rev_data(int port,int len)
{
    char *result;
    char *rev_socket;
    char *rev_ip;
    char *rev_port;
    char *rev_len;
    char *rev_data;

    rt_memset(nb_buffer,0,sizeof(nb_buffer));
    if(at_exec_cmd(at_resp,"AT+NSORF=%d,%d",port,len)!=RT_EOK) return RT_ERROR;
#ifdef QSDK_USING_DEBUD
    LOG_D("rev port=%d   rev len=%d",port,len);
#endif
    at_resp_parse_line_args(at_resp,2,"\r\n%s",nb_buffer);
    LOG_D("revdata=%s\r\n",nb_buffer);
#ifdef QSDK_USING_M5310A
    result=strtok(nb_buffer,":");
    rev_socket=strtok(NULL,",");
#else
    rev_socket=strtok(nb_buffer,",");
#endif

    rev_ip=strtok(NULL,",");
    rev_port=strtok(NULL,",");
    rev_len=strtok(NULL,",");
    rev_data=strtok(NULL,",");

    if(qsdk_net_data_callback(atoi(rev_socket),rev_data,atoi(rev_len))==RT_EOK)
        return RT_EOK;
    else return RT_ERROR;
}
/*************************************************************
*	函数名称：	qsdk_net_close_socket
*
*	函数功能：	关闭网络套子号 socket
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_net_close_socket(void)
{
    if(at_exec_cmd(at_resp,"AT+NSOCL=%d",net_info.socket)!=RT_EOK) return RT_ERROR;

    return RT_EOK;
}

#elif (defined QSDK_USING_ME3616)
/*************************************************************
*	函数名称：	qsdk_net_create_socket
*
*	函数功能：	创建网络套子号 socket
*
*	入口参数：	type : 网络连接类型		socket:创建的网络套子号
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_net_create_socket(int type,int *socket)
{
    if(type==QSDK_NET_TYPE_TCP)
    {
        if(at_exec_cmd(at_resp,"AT+ESOC=1,%d,1",QSDK_NET_TYPE_TCP)!=RT_EOK)	return RT_ERROR;
    }
    else if(type==QSDK_NET_TYPE_UDP)
    {
        if(at_exec_cmd(at_resp,"AT+ESOC=1,%d,1",QSDK_NET_TYPE_UDP)!=RT_EOK)	return RT_ERROR;
    }
    else	return RT_ERROR;
    LOG_D("socket=%d\r\n",*socket);
    at_resp_parse_line_args(at_resp,2,"+ESOC=%d",socket);
    LOG_D("socket=%d\r\n",*socket);

    return RT_EOK;
}
/*************************************************************
*	函数名称：	qsdk_net_connect_to_server
*
*	函数功能：	连接到服务器
*
*	入口参数：	socket:套子号  server_ip：服务器IP   server_port:服务器端口号
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_net_connect_to_server(int socket,char *server_ip,unsigned short server_port)
{
    if(at_exec_cmd(at_resp,"AT+ESOCON=%d,%d,\"%s\"",socket,server_port,server_ip)!=RT_EOK) return RT_ERROR;

    return RT_EOK;
}
/*************************************************************
*	函数名称：	qsdk_net_send_data
*
*	函数功能：	发送数据到服务器
*
*	入口参数：	socket:套子号  str:发送的数据   len：数据长度
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_net_send_data(int socket,char *str,unsigned int len)
{
    if(at_exec_cmd(at_resp,"AT+ESOSEND=%d,%d,%s",socket,len,str)!=RT_EOK)	return RT_ERROR;

    return RT_EOK;
}
/*************************************************************
*	函数名称：	qsdk_net_close_socket
*
*	函数功能：	关闭网络套子号 socket
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_net_close_socket(int socket)
{
    if(at_exec_cmd(at_resp,"AT+ESOCL=%d",socket)!=RT_EOK)	return RT_ERROR;

    return RT_EOK;
}
/*************************************************************
*	函数名称：	qsdk_net_set_data_mode
*
*	函数功能：	关闭网络套子号 socket
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_net_set_data_mode(void)
{
    if(at_exec_cmd(at_resp,"AT+ESOSETRPT=1")!=RT_EOK)	return RT_ERROR;

    return RT_EOK;
}
#endif

#endif

