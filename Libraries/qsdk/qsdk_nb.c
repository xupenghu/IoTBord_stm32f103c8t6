/*
 * File      : qsdk_nb.c
 * This file is part of nb in qsdk
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-14     longmain     first version
 */
#define LOG_TAG              "qsdk_nb"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>
#include "qsdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//定义 AT 命令相应结构体指针
at_response_t at_resp = RT_NULL;

//定义任务控制块
rt_thread_t hand_thread_id=RT_NULL;

//声明 函数
void hand_thread_entry(void* parameter);

//定义邮箱控制块
rt_mailbox_t event_mail=NULL;


struct NB_DEVICE nb_device;

/*************************************************************
*	函数名称：	qsdk_at_send_cmd
*
*	函数功能：	向模组发送CMD命令
*
*	入口参数：	cmd:命令		result:需要判断的响应
*
*	返回参数：	0：成功   1：失败
*
*	说明：
*************************************************************/
int qsdk_at_send_cmd(char *cmd,char *result)
{
    if(at_exec_cmd(at_resp,"%s",cmd)!=RT_EOK)	return RT_ERROR;

    if(at_resp_get_line_by_kw(at_resp,result)==RT_NULL)	return RT_ERROR;

    return RT_EOK;
}

/*************************************************************
*	函数名称：	qsdk_at_resp_cmd
*
*	函数功能：	向模组发送CMD命令
*
*	入口参数：	cmd:命令		result:模组响应数据
*
*	返回参数：	0：成功   1：失败
*
*	说明：
*************************************************************/
int qsdk_at_resp_cmd(char *cmd,int line,char *result)
{
    if(at_exec_cmd(at_resp,"%s",cmd)!=RT_EOK)	return RT_ERROR;

    at_resp_parse_line_args(at_resp,line,"%s\r\n",result);

    return RT_EOK;
}
/*************************************************************
*	函数名称：	qsdk_at_send_data
*
*	函数功能：	向模组发送数据
*
*	入口参数：	data:需要发送的数据
*
*	返回参数：	0：成功   1：失败
*
*	说明：
*************************************************************/
int qsdk_at_send_data(char *data)
{
    if(at_exec_cmd(at_resp,"%s",data)!=RT_EOK)	return RT_ERROR;

    return RT_EOK;
}

/*************************************************************
*	函数名称：	qsdk_init_environment
*
*	函数功能：	QSDK 运行环境初始化
*
*	入口参数：	无
*
*	返回参数：	0：成功   1：失败
*
*	说明：
*************************************************************/
int qsdk_init_environment(void)
{
#ifdef QSDK_USING_LOG
    LOG_D("\r\nWelcome to use QSDK. This SDK by longmain.\r\n Our official website is www.longmain.cn.\r\n\r\n");
#endif
    //创建 AT 命令响应结构体
    at_resp = at_create_resp(QSDK_CMD_REV_MAX_LEN+50, 0, rt_tick_from_millisecond(5000));

    //判断是否创建成功
    if (at_resp == RT_NULL)
    {
        nb_device.error=qsdk_nb_status_create_at_resp_failure;
        goto fail;;
    }

    //创建事件邮箱
    event_mail=rt_mb_create("event_mail",
                            10,
                            RT_IPC_FLAG_FIFO);
    if(event_mail==RT_NULL)
    {
        nb_device.error=qsdk_nb_status_create_event_mail_failure;
        goto fail;
    }

    //创建事件响应任务
    hand_thread_id=rt_thread_create("hand_thread",
                                    hand_thread_entry,
                                    RT_NULL,
                                    512,
                                    7,
                                    50);
    if(hand_thread_id!=RT_NULL)
        rt_thread_startup(hand_thread_id);
    else
    {
        nb_device.error=qsdk_nb_status_create_hand_fun_failure;
		LOG_D("creat hand_thread fail.");
        goto fail;
    }

    return RT_EOK;


fail:
#ifdef QSDK_USING_LOG
    qsdk_nb_dis_error();
#endif
    return RT_ERROR;
}
/*************************************************************
*	函数名称：	nb_hw_init
*
*	函数功能：	NB-IOT 模块联网初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：
*************************************************************/
int qsdk_nb_hw_init(void)
{
    static char status=0;
    int i=5;
	int res;
    //清空设备结构体
    rt_memset(&nb_device,0,sizeof(nb_device));
    if(status!=1)
    {
        //模块引脚初始化
        if(qsdk_hw_io_init()!=RT_EOK)
        {
            nb_device.error=qsdk_nb_status_io_init_failure;
            goto fail;
        }
        else
        {
			res = at_client_init(NB_COMM_PORT, AT_CLIENT_RECV_BUFF_LEN);   //初始化AT组件
			if(res!=RT_EOK) return res;
            if(qsdk_init_environment()!=RT_EOK) return RT_ERROR;

            status=1;
        }

//如果开启支持M5310连接IOT平台
#ifdef QSDK_USING_M5310A
start:
#endif	//QSDK_USING_M5310_IOT  END

        //等待模块连接
        if(qsdk_nb_wait_connect()!=RT_EOK)
            return RT_ERROR;
    }

//如果启用M5310连接IOT平台
#ifdef QSDK_USING_M5310A
    if(qsdk_iot_check_address()!=RT_EOK)
    {
#ifdef QSDK_USING_LOG
        LOG_D("ncdp地址不对，现在进行设置\r\n");
#endif

        if(qsdk_iot_set_address()==RT_EOK)
            goto start;
        else goto fail;
    }
#ifdef QSDK_USING_LOG
    LOG_D("ncdp address check success\r\n");
#endif

#endif

#if QSDK_USING_PSM_MODE
    if(qsdk_nb_set_psm_mode("01000111","10100100")!=RT_EOK)
    {
        nb_device.ERROR=qsdk_nb_status_set_low_power_failure;
        goto fail;
    }
#endif

//首先确定模块是否开机
    do {
        i--;
        if(qsdk_nb_sim_check()!=RT_EOK)
        {
            rt_thread_delay(500);
        }
#ifdef QSDK_USING_LOG
        LOG_D("+CFUN=%d\r\n",nb_device.sim_state);
#endif
        if(nb_device.sim_state!=1)
            rt_thread_delay(1000);

    }	while(nb_device.sim_state==0&&i>0);

    if(i<=0)
    {
        nb_device.error=qsdk_nb_status_module_start_failure;
        goto fail;
    }
    else {
        i=3;
        rt_thread_delay(1000);
    }

//获取SIM卡的IMSI号码
    do {
        i--;
        if(qsdk_nb_get_imsi()!=RT_EOK)
        {
            rt_thread_delay(500);
        }
        else
        {
#ifdef QSDK_USING_LOG
            LOG_D("IMSI=%s\r\n",nb_device.imsi);
#endif
            break;
        }
    } while(i>0);

    if(i<=0)
    {
        nb_device.error=qsdk_nb_status_no_find_sim;
        goto fail;

    }
    else
    {
        i=15;
        rt_thread_delay(100);
    }

//获取模块IMEI
    if(qsdk_nb_get_imei()!=RT_EOK)
    {
        nb_device.error=qsdk_nb_status_read_module_imei_failure;
        goto fail;
    }
    else
    {
#ifdef QSDK_USING_LOG
        LOG_D("IMEI=%s\r\n",nb_device.imei);
#endif
    }

//如果启用IOT平台支持
#ifdef QSDK_USING_IOT

//如果启用M5310连接IOT平台
#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
    rt_thread_delay(100);
    if(qsdk_iot_open_update_status()!=RT_EOK)
    {
        nb_device.error=qsdk_iot_status_open_update_dis_failure;
        goto fail;
    }
#ifdef QSDK_USING_DEBUG
    else LOG_D("qsdk open iot update status success\r\n");
#endif
    rt_thread_delay(100);
    if(qsdk_iot_open_down_date_status()!=RT_EOK)
    {
        nb_device.error=qsdk_iot_status_open_down_dis_failure;
        goto fail;
    }
#ifdef QSDK_USING_DEBUG
    else LOG_D("qsdk open iot down date status success\r\n");
#endif

#endif

#endif
//获取信号值
    do {
        i--;
        if(qsdk_nb_get_csq()!=RT_EOK)
        {
            rt_thread_delay(500);
        }
        else if(nb_device.csq!=99&&nb_device.csq!=0)
        {
            break;
        }
        else
        {
#ifdef QSDK_USING_LOG
            LOG_D("CSQ=%d\r\n",nb_device.csq);
#endif
            rt_thread_delay(3000);
        }

    } while(i>0);

    if(i<=0)
    {
        nb_device.error=qsdk_nb_status_module_no_find_csq;
        goto fail;
    }
    else
    {
#ifdef QSDK_USING_LOG
        LOG_D("CSQ=%d\r\n",nb_device.csq);
#endif
        i=30;
        rt_thread_delay(100);
    }
//手动附着网络
#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
    if(qsdk_nb_get_net_start()!=RT_EOK)
    {
        rt_thread_delay(500);
    }
    else if(!nb_device.net_connect_ok)
    {
        if(qsdk_nb_set_net_start()!=RT_EOK)
        {
            nb_device.error=qsdk_nb_status_set_net_start_failure;
            goto fail;
        }
    }
#endif
//获取网络附着状态
    do {
        i--;
        if(qsdk_nb_get_net_start()!=RT_EOK)
        {
            rt_thread_delay(500);
        }
        else if(nb_device.net_connect_ok)
        {
            break;
        }
        else
        {
#ifdef QSDK_USING_LOG
            LOG_D("CEREG=%d\r\n",nb_device.net_connect_ok);
#endif
            rt_thread_delay(1000);
        }

    } while(i>0);

    if(i<=0)
    {
        nb_device.error=qsdk_nb_status_fine_net_start_failure;
        goto fail;
    }
    rt_thread_delay(1000);
#ifdef QSDK_USING_LOG
    LOG_D("CEREG=%d\r\n",nb_device.net_connect_ok);
#endif

//获取ntp服务器时间

    if(qsdk_nb_get_time()!=RT_EOK)
    {
        nb_device.error=qsdk_nb_status_get_ntp_time_failure;
    }

#ifdef QSDK_USING_LOG
    LOG_D("net connect ok\r\n");
#endif

    return RT_EOK;

fail:

#ifdef QSDK_USING_LOG
    qsdk_nb_dis_error();
#endif
    return RT_ERROR;
}
/*************************************************************
*	函数名称：	qsdk_nb_wait_connect
*
*	函数功能：	等待模块连接
*
*	入口参数：	无
*
*	返回参数：	0 成功   1 失败
*
*	说明：
*************************************************************/
int qsdk_nb_wait_connect(void)
{
    if(at_client_wait_connect(2000)!=RT_EOK)
    {
        nb_device.error=qsdk_nb_status_no_find_nb_module;
        rt_thread_delete(hand_thread_id);			//删除 hand处理函数
        rt_mb_delete(event_mail);							//删除 event 邮箱
        at_delete_resp(at_resp);							//删除 AT 命令相应结构体
        goto fail;
    }

    return RT_EOK;

fail:
#ifdef QSDK_USING_LOG
    qsdk_nb_dis_error();
#endif
    return RT_ERROR;
}

/*************************************************************
*	函数名称：	nb_sim_check
*
*	函数功能：	检测模块是否已经开机
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_nb_sim_check(void)
{
    if(at_exec_cmd(at_resp,"AT+CFUN?")!=RT_EOK)	return RT_ERROR;
    at_resp_parse_line_args(at_resp,2,"+CFUN:%d",&nb_device.sim_state);

    return  RT_EOK;
}
/*************************************************************
*	函数名称：	nb_set_psm_mode
*
*	函数功能：	模块 PSM 模式设置
*
*	入口参数：	tau_time	TAU 时间		active_time active时间
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_nb_set_psm_mode(char *tau_time,char *active_time)
{
    if(at_exec_cmd(at_resp,"AT+CPSMS=1,,,%s,%s",tau_time,active_time)!=RT_EOK)	return RT_ERROR;

    return  RT_EOK;
}
/*************************************************************
*	函数名称：	nb_get_imsi
*
*	函数功能：	获取 SIM 卡的 imsi
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_nb_get_imsi(void)
{
    if(at_exec_cmd(at_resp,"AT+CIMI")!=RT_EOK)	return RT_ERROR;

    at_resp_parse_line_args(at_resp,2,"%s\r\n",nb_device.imsi);
    return  RT_EOK;
}
/*************************************************************
*	函数名称：	nb_get_imei
*
*	函数功能：	获取模块的 imei
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_nb_get_imei(void)
{
    if(at_exec_cmd(at_resp,"AT+CGSN=1")!=RT_EOK)	return RT_ERROR;

    at_resp_parse_line_args(at_resp,2,"+CGSN:%s",nb_device.imei);
    return  RT_EOK;
}
/*************************************************************
*	函数名称：	nb_get_time
*
*	函数功能：	获取网络时间
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_nb_get_time(void)
{
    int year,mouth,day,hour,min,sec;
    if(at_exec_cmd(at_resp,"AT+CCLK?")!=RT_EOK)	return RT_ERROR;

    at_resp_parse_line_args(at_resp,2,"+CCLK:%d/%d/%d,%d:%d:%d+",&year,&mouth,&day,&hour,&min,&sec);

#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
    qsdk_nb_set_rtc_time(year,mouth,day,hour,min,sec);
#else
    qsdk_nb_set_rtc_time(year%100,mouth,day,hour,min,sec);
#endif
    return  RT_EOK;
}

/*************************************************************
*	函数名称：	nb_get_csq
*
*	函数功能：	获取当前信号值
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_nb_get_csq(void)
{
    if(at_exec_cmd(at_resp,"AT+CSQ")!=RT_EOK)	return RT_ERROR;

    at_resp_parse_line_args(at_resp,2,"+CSQ:%d\r\n",&nb_device.csq);
    return  RT_EOK;

}
/*************************************************************
*	函数名称：	nb_set_net_start
*
*	函数功能：	手动附着网络
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_nb_set_net_start(void)
{
    if(at_exec_cmd(at_resp,"AT+CGATT=1")!=RT_EOK)	return RT_ERROR;

    return  RT_EOK;
}
/*************************************************************
*	函数名称：	nb_get_net_start
*
*	函数功能：	获取当前网络状态
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_nb_get_net_start(void)
{
    int i,j;
    if(at_exec_cmd(at_resp,"AT+CEREG?")!=RT_EOK)	return RT_ERROR;

    at_resp_parse_line_args(at_resp,2,"+CEREG:%d,%d\r\n",&i,&j);

    if(j==1) nb_device.net_connect_ok=1;
    else nb_device.net_connect_ok=0;
    return  RT_EOK;
}

#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
/*************************************************************
*	函数名称：	nb_query_ip
*
*	函数功能：	查询模块在核心网的IP地址
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_nb_query_ip(void)
{
    if(at_exec_cmd(at_resp,"AT+CGPADDR")!=RT_EOK) return RT_ERROR;

    at_resp_parse_line_args(at_resp,2,"+CGPADDR:0,%s",nb_device.ip);
    return RT_EOK;
}
#endif	//qsdk_enable_m5310	end



#ifdef QSDK_USING_ME3616_GPS
/*************************************************************
*	函数名称：	qsdk_gps_config
*
*	函数功能：	配置并且启用AGPS
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_agps_config(void)
{
    int i=90;
    char str[20];

    if(at_exec_cmd(at_resp,"AT+ZGMODE=1")!=RT_EOK)	return RT_ERROR;
    at_resp_parse_line_args(at_resp,2,"%s",str);
    if(rt_strstr(str,"+ZGPS: DATA DOWNLOAD SUCCESS")==NULL)
    {
#ifdef QSDK_USING_DEBUG
        LOG_D("seting gps \r\n");
#endif
        rt_thread_delay(100);
        if(at_exec_cmd(at_resp,"AT+ZGDATA")!=RT_EOK)	return RT_ERROR;

        do {
            i--;
            rt_thread_delay(500);
            if(at_exec_cmd(at_resp,"AT+ZGDATA?")!=RT_EOK)	return RT_ERROR;
            at_resp_parse_line_args(at_resp,2,"+ZGDATA: %s\r\n",str);
            if(rt_strstr(str,"READY")!=NULL)
                if(rt_strstr(str,"NO READY")==NULL)
                    break;
        } while(i>0);
        if(i<=0)
            return RT_ERROR;
    }
#ifdef QSDK_USING_DEBUG
    LOG_D("gps runing\r\n");
#endif
    rt_thread_delay(100);
    if(at_exec_cmd(at_resp,"AT+ZGNMEA=2")!=RT_EOK)	return RT_ERROR;
    rt_thread_delay(100);
    if(at_exec_cmd(at_resp,"AT+ZGRUN=1")!=RT_EOK)	return RT_ERROR;

    return	RT_EOK;
}
/*************************************************************
*	函数名称：	qsdk_gps_config
*
*	函数功能：	配置并且启用GPS
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_gps_config(void)
{
    int i=90;
    char str[20];
    if(at_exec_cmd(at_resp,"AT+ZGMODE=2")!=RT_EOK)	return RT_ERROR;
    rt_thread_delay(100);
    if(at_exec_cmd(at_resp,"AT+ZGNMEA=2")!=RT_EOK)	return RT_ERROR;
    rt_thread_delay(100);
    if(at_exec_cmd(at_resp,"AT+ZGRUN=2")!=RT_EOK)	return RT_ERROR;

    return	RT_EOK;
}
#endif	//qsdk_enable_me3616_gps	end

/*************************************************************
*	函数名称：	string_to_hex
*
*	函数功能：	字符串转hex
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int string_to_hex(const char *pString, int len, char *pHex)
{
    int i = 0;
    if (NULL == pString || len <= 0 || NULL == pHex)
    {
        return RT_ERROR;
    }
    for(i = 0; i < len; i++)
    {
        rt_sprintf(pHex+i*2, "%02X", pString[i]);
    }
    return RT_EOK;
}
/*************************************************************
*	函数名称：	nb_set_rtc_time
*
*	函数功能：	设置RTC时间为当前时间
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
void qsdk_nb_set_rtc_time(int year,int month,int day,int hour,int min,int sec)
{
    int week,lastday;
    hour+=QSDK_TIME_ZONE;
    if ((0==year%4 && 0!=year%100) || 0==year%400)
        lastday=29;
    else if(month==1||month==3||month==5||month==7||month==8||month==10||month==12)
        lastday=31;
    else if(month==4||month==6||month==9||month==11)
        lastday=30;
    else
        lastday=28;
    if(hour>24)
    {
        hour-=24;
        day++;
        if(day>lastday)
        {
            day-=lastday;
            month++;
        }
        if(month>12)
        {
            month-=12;
            year++;
        }
    }
    week=(day+2*month+3*(month+1)/5+year+year/4-year/100+year/400)%7+1;
#ifdef QSDK_USING_DEBUG
    LOG_D("time=%d-%d-%d,%d-%d-%d,week=%d\r\n",year,month,day,hour,min,sec,week);
#endif
    qsdk_rtc_set_time_callback(year,month,day,hour,min,sec,week);

}
/*************************************************************
*	函数名称：	hand_thread_entry
*
*	函数功能：	模组主动上报数据处理函数
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：
*************************************************************/
void hand_thread_entry(void* parameter)
{
    rt_err_t status=RT_EOK;
    char *event;
    char *result=NULL;
#ifdef QSDK_USING_ONENET
    char *instance=NULL;
    char *msgid=NULL;
    char *objectid=NULL;
    char *instanceid=NULL;
    char *resourceid=NULL;
#endif
    while(1)
    {
        //等待事件邮件 event_mail
        status=rt_mb_recv(event_mail,(rt_uint32_t *)&event,RT_WAITING_FOREVER);

        //判断是否接收成功
        if(status==RT_EOK)
        {
#ifdef QSDK_USING_NET

#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
            //判断是不是M5310 tcp 或者 udp 消息
            if(rt_strstr(event,"+NSONMI:")!=RT_NULL)
            {
                char *eventid=NULL;
                char *socket=NULL;
                char *len=NULL;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n ",event);
#endif
                eventid=strtok((char*)event,":");
                socket=strtok(NULL,",");
                len=strtok(NULL,",");

                //调用网络数据处理回调函数
                if(qsdk_net_rev_data(atoi(socket),atoi(len))!=RT_EOK)
                    LOG_D("rev net data failure\r\n");
            }
#elif (defined QSDK_USING_ME3616)
            //判断是不是 tcp 或者 udp 消息
            if(rt_strstr(event,"+ESONMI=")!=RT_NULL)
            {
                char *result;
                char *socket;
                char *rev_len;
                char *rev_data;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                result=strtok(event,"=");
                socket=strtok(NULL,",");
                rev_len=strtok(NULL,",");
                rev_data=strtok(NULL,"\r\n");

                if(qsdk_net_data_callback(atoi(socket),rev_data,atoi(rev_len))!=RT_EOK)
                    LOG_D("QSDK net data callback failure\r\n");

            }
#endif	//QSDK_USING_ME3616_NET	END

#endif	//QSDK_USING_NET END
#ifdef QSDK_USING_IOT

#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
            if(rt_strstr(event,"+NNMI:")!=RT_NULL)
            {
                char *len;
                char *str;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                result=strtok(event,":");
                len=strtok(NULL,",");
                str=strtok(NULL,",");

                if(qsdk_iot_data_callback(str,atoi(len))!=RT_EOK)
                    LOG_D("qsdk iot data callback failure\r\n");
            }
            else if(rt_strstr(event,"+NSMI:")!=RT_NULL)
            {
#ifdef QSDK_USING_DEBUD
                LOG_D("%s\r\n",event);
#endif
                nb_device.notify_status=qsdk_iot_status_notify_success;
            }

#elif (defined QSDK_USING_ME3616)
            if(rt_strstr(event,"+M2MCLI:")!=RT_NULL)
            {
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                if(rt_strstr(event,"+M2MCLI:register failure"))
                    nb_device.iot_connect_status=qsdk_iot_status_reg_failure;
                if(rt_strstr(event,"+M2MCLI:register success"))
                    nb_device.iot_connect_status=qsdk_iot_status_reg_success;
                if(rt_strstr(event,"+M2MCLI:observe success"))
                    nb_device.iot_connect_status=qsdk_iot_status_observer_success;
                if(rt_strstr(event,"+M2MCLI:deregister success"))
                    nb_device.iot_connect_status=qsdk_iot_status_reg_init;
                if(rt_strstr(event,"+M2MCLI:notify success"))
                    nb_device.notify_status=qsdk_iot_status_notify_success;
            }
            if(rt_strstr(event,"+M2MCLIRECV:")!=RT_NULL)
            {
                char *str;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                result=strtok(event,":");
                str=strtok(NULL,",");
                if(qsdk_iot_data_callback(str,(strlen(str)/2)-1)!=RT_EOK)
                {
#ifdef QSDK_USING_DEBUD
                    LOG_D("iot data rev failure\r\n");
#endif
                }
            }
            if(rt_strstr(event,"$GNRMC")!=RT_NULL)
            {
                char *gps_time;
                char *gps_status;
                char *gps_lat;
                char *result4;
                char *gps_lon;
                char *result6;
                char *gps_speed;
                double temp = 0;
                uint32_t dd = 0;
#ifdef QSDK_USING_LOG
                LOG_D("%s,  len=%d",event,strlen(event));
#endif
                if(strlen(event)>27)
                {
                    double lat;
                    double lon;
                    double speed;
                    result=strtok(event,",");
                    gps_time=strtok(NULL,",");
                    gps_status=strtok(NULL,",");
                    if(*gps_status=='A')
                    {
                        gps_lat=strtok(NULL,",");
                        result4=strtok(NULL,",");
                        gps_lon=strtok(NULL,",");
                        result6=strtok(NULL,",");
                        gps_speed=strtok(NULL,",");

                        //GPRMC的纬度值格式为ddmm.mmmm,要转换成dd.dddddd
                        temp = atof(gps_lat);
                        dd = (uint32_t)(temp / 100);  //取度数整数
                        lat = dd + ((temp - dd * 100)/60);

                        //GPRMC的经度格式为dddmm.mmmm，要转换成dd.dddddd
                        temp = atof(gps_lon);
                        dd = (uint32_t)(temp / 100);
                        lon = dd + ((temp - dd * 100)/60);

                        speed=atof(gps_speed);

                        if(qsdk_gps_callback(lon,lat,speed)!=RT_EOK)
                            LOG_D("GPS callback fallure");

                    }
                }
            }
#endif

#endif
#ifdef QSDK_USING_ONENET
            //判断是否为事件消息
            if(rt_strstr(event,"+MIPLEVENT:")!=RT_NULL)
            {
                char *eventid=NULL;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n ",event);
#endif
                result=strtok((char*)event,":");
                instance=strtok(NULL,",");
                eventid=strtok(NULL,",");
#ifdef QSDK_USING_DEBUD
                LOG_D("instance=%d,eventid=%d\r\n",atoi(instance),atoi(eventid));
#endif
                //进入事件处理函数
                switch(atoi(eventid))
                {
                case  1:
#ifdef QSDK_USING_LOG
                    LOG_D("Bootstrap start    \r\n");
#endif
                    data_stream.event_status=qsdk_onenet_status_run;
                    break;
                case  2:
#ifdef QSDK_USING_LOG
                    LOG_D("Bootstrap success  \r\n");
#endif
                    data_stream.event_status=qsdk_onenet_status_run;
                    break;
                case  3:
#ifdef QSDK_USING_LOG
                    LOG_D("Bootstrap failure\r\n");
#endif
                    data_stream.event_status=qsdk_onenet_status_failure;
                    break;
                case  4:
#ifdef QSDK_USING_LOG
                    LOG_D("Connect success\r\n");
#endif
                    data_stream.event_status=qsdk_onenet_status_run;
                    break;
                case  5:
#ifdef QSDK_USING_LOG
                    LOG_D("Connect failure\r\n");
#endif
                    data_stream.event_status=qsdk_onenet_status_failure;
                    break;
                case  6:
#ifdef QSDK_USING_LOG
                    LOG_D("Reg onenet success\r\n");
#endif
                    data_stream.event_status=qsdk_onenet_status_success;
                    break;
                case  7:
#ifdef QSDK_USING_LOG
                    LOG_D("Reg onenet failure\r\n");
#endif
                    data_stream.event_status=qsdk_onenet_status_failure;
                    break;
                case  8:
#ifdef QSDK_USING_LOG
                    LOG_D("Reg onenet timeout\r\n");
#endif
                    data_stream.event_status=qsdk_onenet_status_failure;
                    break;
                case  9:
#ifdef QSDK_USING_LOG
                    LOG_D("Life_time timeout\r\n");
#endif
                    break;
                case 10:
#ifdef QSDK_USING_LOG
                    LOG_D("Status halt\r\n");
#endif
                    break;
                case 11:
#ifdef QSDK_USING_LOG
                    LOG_D("Update success\r\n");
#endif
                    data_stream.update_status=qsdk_onenet_status_update_success;
                    break;
                case 12:
#ifdef QSDK_USING_LOG
                    LOG_D("Update failure\r\n");
#endif
                    data_stream.update_status=qsdk_onenet_status_update_failure;
                    break;
                case 13:
#ifdef QSDK_USING_LOG
                    LOG_D("Update timeout\r\n");
#endif
                    data_stream.update_status=qsdk_onenet_status_update_timeout;
                    break;
                case 14:
#ifdef QSDK_USING_LOG
                    LOG_D("Update need\r\n");
#endif
                    data_stream.update_status=qsdk_onenet_status_update_need;
                    break;
                case 15:
#ifdef QSDK_USING_LOG
                    LOG_D("Unreg success\r\n");
#endif
                    data_stream.connect_status=qsdk_onenet_status_failure;
                    break;
                case 20:
#ifdef QSDK_USING_LOG
                    LOG_D("Response failure\r\n");
#endif
                    break;
                case 21:
#ifdef QSDK_USING_LOG
                    LOG_D("Response success\r\n");
#endif
                    break;
                case 25:
#ifdef QSDK_USING_LOG
                    LOG_D("Notify failure\r\n");
#endif
                    data_stream.notify_status=qsdk_onenet_status_failure;
                    break;
                case 26:
#ifdef QSDK_USING_LOG
                    LOG_D("Notify success\r\n");
#endif
                    data_stream.notify_status=qsdk_onenet_status_success;
                    break;
                default:
                    break;
                }
            }
            //判断是否为 onenet 平台 read 事件
            else if(rt_strstr(event,"+MIPLREAD:")!=RT_NULL)
            {
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                result=strtok(event,":");
                instance=strtok(NULL,",");
                msgid=strtok(NULL,",");
                objectid=strtok(NULL,",");
                instanceid=strtok(NULL,",");
                resourceid=strtok(NULL,",");

                //进入 onenet read 响应函数
                status=qsdk_rsp_onenet_read(atoi(msgid),atoi(objectid),atoi(instanceid),atoi(resourceid));
                //判断是否响应成功
                if(status!=RT_EOK)
                    LOG_D("rsp onener read failure\r\n");
#ifdef QSDK_USING_LOG
                else
                {
                    LOG_D("rsp onener read success\r\n");
                }
#endif
            }
            //判断是否为 onenet write 事件
            else if(rt_strstr(event,"+MIPLWRITE:")!=RT_NULL)
            {
                char *valuetype=NULL;
                char *value_len=NULL;
                char *value=NULL;
                char *flge=NULL;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                result=strtok(event,":");
                instance=strtok(NULL,",");
                msgid=strtok(NULL,",");
                objectid=strtok(NULL,",");
                instanceid=strtok(NULL,",");
                resourceid=strtok(NULL,",");
                valuetype=strtok(NULL,",");
                value_len=strtok(NULL,",");
                value=strtok(NULL,",");
                flge=strtok(NULL,",");
                //判断标识是否为0
                if(atoi(flge)==0)
                {
                    //执行 onenet write 响应函数
                    if(qsdk_rsp_onenet_write(atoi(msgid),atoi(objectid),atoi(instanceid),atoi(resourceid),atoi(valuetype),atoi(value_len),value)!=RT_EOK)
                        LOG_D("rsp onenet write failure\r\n");
#ifdef QSDK_USING_LOG
                    else LOG_D("rsp onenet write success\r\n");
#endif
                }
            }
            //判断是否为 exec 事件
            else if(rt_strstr(event,"+MIPLEXECUTE:")!=RT_NULL)
            {
                char *value_len=NULL;
                char *value=NULL;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                result=strtok(event,":");
                instance=strtok(NULL,",");
                msgid=strtok(NULL,",");
                objectid=strtok(NULL,",");
                instanceid=strtok(NULL,",");
                resourceid=strtok(NULL,",");
                value_len=strtok(NULL,",");
                value=strtok(NULL,",");

                //执行 onenet write 响应函数
                if(qsdk_rsp_onenet_execute(atoi(msgid),atoi(objectid),atoi(instanceid),atoi(resourceid),atoi(value_len),value)!=RT_EOK)
                    LOG_D("rsp onenet execute failure\r\n");
#ifdef QSDK_USING_LOG
                else LOG_D("rsp onenet execute success\r\n");
#endif
            }
            //判断是否为 observe 事件
            else if(rt_strstr(event,"+MIPLOBSERVE:")!=RT_NULL)
            {
                char *oper=NULL;
                int i=0,j=0,status=1,resourcecount=0;
                data_stream.observercount++;
                data_stream.observer_status=qsdk_onenet_status_run;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                result=strtok(event,":");
                instance=strtok(NULL,",");
                msgid=strtok(NULL,",");
                oper=strtok(NULL,",");
                objectid=strtok(NULL,",");
                instanceid=strtok(NULL,",");

                for(; i<data_stream.dev_len; i++)
                {
                    if(data_stream.dev[i].objectid==atoi(objectid)&&data_stream.dev[i].instanceid==atoi(instanceid))
                    {
                        data_stream.dev[i].msgid=atoi(msgid);
#ifdef QSDK_USING_DEBUG
                        LOG_D("objece=%d,instanceid=%d msg=%d\r\n",data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].msgid);
#endif
                    }
                }
#ifdef QSDK_USING_ME3616
                LOG_D("AT+MIPLOBSERVERSP=%d,%d,%d\r\n",data_stream.instance,atoi(msgid),atoi(oper));
                if(at_exec_cmd(at_resp,"AT+MIPLOBSERVERSP=%d,%d,%d",data_stream.instance,atoi(msgid),atoi(oper))!=RT_EOK)
                {
                    LOG_D("+MIPLOBSERVERSP  failure \r\n");
                    data_stream.observer_status=qsdk_onenet_status_failure;
                }
#endif

#ifdef QSDK_USING_DEBUD
                LOG_D("observercount=%d\r\n",data_stream.observercount);
#endif
                //判断 obcerve 事件是否执行完成
                if(data_stream.observercount==data_stream.instancecount)
                {
                    //observe 事件执行完成
                    data_stream.observer_status=qsdk_onenet_status_success;
#ifdef QSDK_USING_DEBUD
                    LOG_D("+MIPLOBSERVERSP  success\r\n ");
#endif
                }
            }
            //判断是否为 discover 事件
            else if(rt_strstr(event,"+MIPLDISCOVER:")!=RT_NULL)
            {
                char resourcemap[50]="";
                int str[50];
                int i=0,j=0,status=1,resourcecount=0;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                result=strtok(event,":");
                instance=strtok(NULL,",");
                msgid=strtok(NULL,",");
                objectid=strtok(NULL,",");

                //循环检测设备总数据流
                for(; i<data_stream.dev_len; i++)
                {
                    //判断当前objectid 是否为 discover 回复的 objectid
                    if(data_stream.dev[i].objectid==atoi(objectid))
                    {
                        j=0;
                        //判断当前 resourceid 是否上报过
                        for(; j<resourcecount; j++)
                        {
                            //如果 resourceid没有上报过
                            if(str[j]!=data_stream.dev[i].resourceid)
                                status=1;		//上报标识置一
                            else
                            {
                                status=0;
                                break;
                            }
                        }
                        //判断该 resourceid 是否需要上报
                        if(status)
                        {
                            //记录需要上报的 resourceid
                            str[resourcecount++]=data_stream.dev[i].resourceid;

                            //判断本次上报的 resourceid 是否为第一次找到
                            if(resourcecount-1)
                                rt_sprintf(resourcemap,"%s;%d",resourcemap,data_stream.dev[i].resourceid);
                            else
                                rt_sprintf(resourcemap,"%s%d",resourcemap,data_stream.dev[i].resourceid);
                        }
                    }
                    //			}
                }
#ifdef QSDK_USING_DEBUD
                j=0;
                //循环打印已经记录的 resourceid
                for(; j<resourcecount; j++)
                    LOG_D("resourcecount=%d\r\n",str[j]);

                //打印 resourceid map
                LOG_D("map=%s\r\n",resourcemap);
#endif

#ifdef QSDK_USING_ME3616
                LOG_D("AT+MIPLDISCOVERRSP=%d,%d,1,%d,\"%s\"\r\n",data_stream.instance,atoi(msgid),strlen(resourcemap),resourcemap);
                if(at_exec_cmd(at_resp,"AT+MIPLDISCOVERRSP=%d,%d,1,%d,\"%s\"",data_stream.instance,atoi(msgid),strlen(resourcemap),resourcemap)!=RT_EOK)
                {
                    LOG_D("+MIPLDISCOVERRSP  failure \r\n");
                    data_stream.discover_status=qsdk_onenet_status_failure;
                }
#endif
                data_stream.discovercount++;
                data_stream.discover_status=qsdk_onenet_status_run;

//				LOG_D("discover_count=%d    objcoun=%d",data_stream.discovercount,data_stream.objectcount);
                //判断 discover 事件是否完成
                if(data_stream.discovercount==data_stream.objectcount)
                {
                    //discover 事件已经完成
#ifdef QSDK_USING_LOG
                    LOG_D("onenet connect success\r\n");
#endif
                    //onenet 连接成功
                    data_stream.discover_status=qsdk_onenet_status_success;
                    data_stream.connect_status=qsdk_onenet_status_success;
                }
            }
#endif
        }
        else
            LOG_D("event_mail recv fail\r\n");
    }
}
/*************************************************************
*	函数名称：	nb_dis_error
*
*	函数功能：	打印模块初始化错误信息
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：
*************************************************************/
void qsdk_nb_dis_error(void)
{
    switch(nb_device.error)
    {
    case qsdk_nb_status_module_init_ok:
        LOG_D("模块初始化成功\r\n");
        break;
    case qsdk_nb_status_io_init_failure:
        LOG_D("模块引脚初始化失败\r\n");
        break;
    case qsdk_nb_status_create_at_resp_failure:
        LOG_D("创建AT响应结构体失败\r\n");
        break;
    case qsdk_nb_status_create_event_mail_failure:
        LOG_D("创建事件邮箱失败\r\n");
        break;
    case qsdk_nb_status_create_hand_fun_failure:
        LOG_D("创建事件响应函数失败\r\n");
        break;
    case qsdk_nb_status_no_find_nb_module:
        LOG_D("没有找到NB-IOT模块\r\n");
        break;
    case qsdk_nb_status_set_low_power_failure:
        LOG_D("设置低功耗模式失败\r\n");
        break;
    case qsdk_nb_status_module_start_failure:
        LOG_D("模块开机失败\r\n");
        break;
    case qsdk_nb_status_no_find_sim:
        LOG_D("没有识别到SIM卡\r\n");
        break;
    case qsdk_nb_status_read_module_imei_failure:
        LOG_D("读取模块IMEI失败\r\n");
        break;
    case qsdk_nb_status_module_no_find_csq:
        LOG_D("模块没有找到信号\r\n");
        break;
    case qsdk_nb_status_set_net_start_failure:
        LOG_D("手动附着网络失败\r\n");
        break;
    case qsdk_nb_status_fine_net_start_failure:
        LOG_D("模块附着网络失败\r\n");
        break;
    case qsdk_nb_status_get_ntp_time_failure:
        LOG_D("获取NTP时间失败\r\n");
        break;
#ifdef QSDK_USING_IOT
    case qsdk_iot_status_set_mini_sim_failure:
        LOG_D("开启模块最小功能失败\r\n");
        break;
    case qsdk_iot_status_set_address_failure:
        LOG_D("设置NCDP服务器失败\r\n");
        break;
    case qsdk_iot_status_open_update_dis_failure:
        LOG_D("开启上报回显失败\r\n");
        break;
    case qsdk_iot_status_open_down_dis_failure:
        LOG_D("开启下发回显失败\r\n");
        break;
    case qsdk_iot_status_notify_failure:
        LOG_D("发送数据到IOT平台失败\r\n");
        break;
#endif
    default:
        break;
    }
}

