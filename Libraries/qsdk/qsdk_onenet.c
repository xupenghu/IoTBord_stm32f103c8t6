/*
 * File      : qsdk_onenet.c
 * This file is part of onenet in qsdk
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-14     longmain     first version
 */
#define LOG_TAG              "qsdk_onenet"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>
#include "qsdk.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef QSDK_USING_ONENET

extern at_response_t at_resp;

//引用任务控制块
extern rt_thread_t hand_thread_id;

//引用邮箱控制块
extern rt_mailbox_t event_mail;

//初始化数据流结构体
DATA_STREAM data_stream= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,NULL,NULL,NULL,NULL};

//注册码生成函数预定义
static int head_node_parse(char *str);
static int init_node_parse(char *str);
static int net_node_parse(char *str);
static int sys_node_parse(char *str);
/*
************************************************************
*	函数名称：	onenet_init
*
*	函数功能：	完成onenet初始化任务，并且连接到onenet平台
*
*	入口参数：	*device  设备数据流参数结构体
*
*	入口参数：	len 结构体长度
*
*	入口参数：	lifetime  设备登录超时时间
*
*   入口参数：	*config_t	设备注册码
*
*	返回参数：	0 成功		1  失败
*
*	说明：
************************************************************
*/
int qsdk_onenet_init(DEVICE *device,int len,int lifetime)
{
    int count;
    memset(&data_stream,0,sizeof(data_stream));		 //清空数据流结构体
    data_stream.dev=device;												 //设备参数映射
    data_stream.dev_len=len;											 //设备参数长度关联
    data_stream.connect_status=qsdk_onenet_status_init;			 //onenet连接状态初始化
    data_stream.initstep=0;												 //初始化步骤设置为0
    data_stream.write_callback=qsdk_onenet_write_rsp_callback; //write回调函数映射
    data_stream.read_callback=qsdk_onenet_read_rsp_callback;	 //read 回掉函数映射
    data_stream.execute_callback=qsdk_onenet_exec_rsp_callback;//exec 回调函数映射
    if(qsdk_create_onenet_instance()!=RT_EOK)	 //创建 instance
    {
        data_stream.error=1;
        goto failure;
    }
    data_stream.initstep=1;
    if(qsdk_create_onenet_object()!=RT_EOK)							//创建 object
    {
        data_stream.error=2;
        goto failure;
    }
//如果模组是M5310，需要上报数据
#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
    data_stream.initstep=2;
    if(qsdk_notify_data_to_onenet(0)!=RT_EOK)						// notify 设备参数到模组
    {
        data_stream.error=3;
        goto failure;
    }
#endif
    data_stream.initstep=3;
    if(qsdk_onenet_open(lifetime)!=RT_EOK)								// 执行 onenet 登录函数
    {
        data_stream.error=4;
        goto failure;
    }
    data_stream.initstep=4;
    data_stream.event_status=qsdk_onenet_status_init;
    count=300;
    do {
        count--;
        rt_thread_mdelay(100);
    } while(data_stream.event_status==qsdk_onenet_status_init&& count>0);		//设备进入 event初始化

    if(data_stream.event_status==qsdk_onenet_status_init||count<=0||data_stream.event_status==qsdk_onenet_status_failure)
    {
        data_stream.error=5;
        goto failure;
    }
    else
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("onenet reg success\r\n");
#endif
        data_stream.initstep=5;
        count=300;
    }
    do {
        count--;
        rt_thread_mdelay(100);
    } while(data_stream.event_status==qsdk_onenet_status_run&&count>0);	//等待模组返回 登录成功的 event事件
    if(data_stream.event_status==qsdk_onenet_status_run)
    {
        data_stream.error=6;
        goto failure;
    }
    else
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("open run success\r\n");
#endif
        data_stream.initstep=6;
        count=300;
    }

    if(data_stream.event_status==qsdk_onenet_status_success)					// 判断 onenet是否返回注册成功 event
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("open success\r\n");
#endif
        data_stream.initstep=7;
        count=300;
    }
    else
    {
        data_stream.error=7;
        goto failure;
    }

    do {
        count--;
        rt_thread_mdelay(100);
    } while(data_stream.observer_status==qsdk_onenet_status_init&& count>0);			//设备进入 observer初始化

    if(data_stream.observer_status==qsdk_onenet_status_init)
    {
        data_stream.error=8;
        goto failure;
    }
    else
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("observer init success\r\n");
#endif
        data_stream.initstep=8;
        count=300;
    }
    do {
        count--;
        rt_thread_mdelay(100);
    } while(data_stream.observer_status==qsdk_onenet_status_run&&count>0);		//判断设备是否收到 observer信息
    if(data_stream.observer_status==qsdk_onenet_status_run)
    {
        data_stream.error=9;
        goto failure;
    }
    else
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("observer run success\r\n");
#endif
        data_stream.initstep=9;
        count=300;
    }

    if(data_stream.observer_status==qsdk_onenet_status_success)				//判断 observer初始化是否成功
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("observer success\r\n");
#endif
        data_stream.initstep=10;
        count=300;
    }
    else
    {
        data_stream.error=10;
        goto failure;
    }
    do {
        count--;
        rt_thread_mdelay(100);
    } while(data_stream.discover_status==qsdk_onenet_status_init&& count>0);//等待否进入 discover	初始化

    if(data_stream.discover_status==qsdk_onenet_status_init)	//判断是否进入 discover初始化
    {
        data_stream.error=11;
        goto failure;
    }
    else
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("discover init success\r\n");
#endif
        data_stream.initstep=11;
        count=300;
    }
    do {
        count--;
        rt_thread_mdelay(100);
    } while(data_stream.discover_status==qsdk_onenet_status_run&&count>0);//等待模组返回 discover信息

    if(data_stream.discover_status==qsdk_onenet_status_run)
    {
        data_stream.error=12;
        goto failure;
    }
    else
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("discover run success\r\n");
#endif
        data_stream.initstep=12;
    }

    if(data_stream.discover_status==qsdk_onenet_status_success)	//判断	discover 是否初始化成功
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("discover success\r\n");
#endif
        data_stream.initstep=13;
        return RT_EOK;
    }
    else
    {
        data_stream.error=13;
        goto failure;
    }

//错误处理函数
failure:
    if(data_stream.initstep>3)				//如果错误值大于 3 将关闭设备 instance
    {
        if(qsdk_onenet_close()!=RT_EOK)
            LOG_D("close onenet instance failure\r\n");
    }
    if(qsdk_delete_onenet_instance()!=RT_EOK)				//删除 模组 instance
        LOG_D("delete onenet instance failure\r\n");
#ifdef QSDK_USING_LOG
    qsdk_onenet_dis_error();							//打印错误值信息
#endif
    rt_thread_delete(hand_thread_id);			//删除 hand处理函数
    rt_mb_delete(event_mail);			        //删除 event 邮箱
    return RT_ERROR;
}

/******************************************************
* 函数名称： create_onenet_instance
*
*	函数功能： 创建模组实例 instance
*
* 入口参数： config_t 设备注册码
*
* 返回值： 0 成功  1失败
*
********************************************************/
int qsdk_create_onenet_instance(void)
{
    char str[100]="";
    char config_t[120]="";
    if(head_node_parse(config_t)==RT_EOK)
    {
        if(init_node_parse(str)==RT_EOK)
        {
            if(net_node_parse(str)==RT_EOK)
            {
                if(sys_node_parse(str)!=RT_EOK)
                {
                    data_stream.error=qsdk_onenet_status_reg_code_failure;
                    return RT_ERROR;
                }
            }
            else
            {
                data_stream.error=qsdk_onenet_status_reg_code_failure;
                return RT_ERROR;
            }
        }
        else
        {
            data_stream.error=qsdk_onenet_status_reg_code_failure;
            return RT_ERROR;
        }

    }
    else
    {
        data_stream.error=qsdk_onenet_status_reg_code_failure;
        return RT_ERROR;
    }


    rt_sprintf(config_t,"%s%04x%s",config_t,strlen(str)/2+3,str);

#ifdef QSDK_USING_DEBUD
    LOG_D("注册码==%s\r\n",config_t);
#endif

    //发送注册命令到模组
    if(at_exec_cmd(at_resp,"AT+MIPLCREATE=%d,%s,0,%d,0",strlen(config_t)/2,config_t,strlen(config_t)/2)!=RT_EOK) return RT_ERROR;

    //解析模组返回的响应值
    at_resp_parse_line_args(at_resp,2,"+MIPLCREATE:%d",&data_stream.instance);

#ifdef QSDK_USING_LOG
    LOG_D("onenet create success,instance=%d\r\n",data_stream.instance);
#endif
    return RT_EOK;
}
/******************************************************
* 函数名称： delete_onenet_instance
*
*	函数功能： 删除模组实例 instance
*
* 入口参数： 无
*
* 返回值： 0 成功  1失败
*
********************************************************/
int qsdk_delete_onenet_instance(void)
{
    //发送删除instance 命令
    if(at_exec_cmd(at_resp,"AT+MIPLDELETE=%d",data_stream.instance)!=RT_EOK) return RT_ERROR;

#ifdef QSDK_USING_LOG
    LOG_D("onenet instace delete success\r\n");
#endif
    return RT_EOK;
}
/******************************************************
* 函数名称： create_onenet_object
*
*	函数功能： 添加 object 到模组
*
* 入口参数： 无
*
* 返回值： 0 成功  1失败
*
********************************************************/
int qsdk_create_onenet_object(void)
{
    int i=0,j=0,status=1;
    int str[QSDK_MAX_OBJECT_COUNT];

    //循环添加 object
    for(i = 0; i<data_stream.dev_len; i++)
    {
        //循环查询object 并且添加到数组，用于记录，防止重复添加
        for(j=0; j<data_stream.objectcount; j++)
        {
            if(str[j]!=data_stream.dev[i].objectid)
                status=1;
            else
            {
                status=0;
                break;
            }
        }
        //添加object 到模组
        if(status)
        {
            //记录当前object
            str[data_stream.objectcount]=data_stream.dev[i].objectid;

            //objectid 数量增加一次
            data_stream.objectcount++;
#ifdef QSDK_USING_ME3616
            data_stream.instancecount+=data_stream.dev[i].instancecount;
#else
            data_stream.instancecount=data_stream.dev_len;
#endif

            LOG_D("AT+MIPLADDOBJ=%d,%d,%d,\"%s\",%d,%d\r\n",data_stream.instance,data_stream.dev[i].objectid,data_stream.dev[i].instancecount,data_stream.dev[i].instancebitmap,data_stream.dev[i].attributecount,data_stream.dev[i].actioncount);
            //向模组添加 objectid
            if(at_exec_cmd(at_resp,"AT+MIPLADDOBJ=%d,%d,%d,\"%s\",%d,%d",data_stream.instance,data_stream.dev[i].objectid,data_stream.dev[i].instancecount,data_stream.dev[i].instancebitmap,data_stream.dev[i].attributecount,data_stream.dev[i].actioncount)!=RT_EOK) return RT_ERROR;
#ifdef QSDK_USING_LOG
            LOG_D("create object success id:%d\r\n",data_stream.dev[i].objectid);
#endif
        }
    }
#ifdef QSDK_USING_DEBUD
    LOG_D("object count=%d\r\n",data_stream.objectcount);
#endif

    return RT_EOK;
}
/******************************************************
* 函数名称： delete_onenet_object
*
*	函数功能： 删除模组内已注册 object
*
* 入口参数： objectid
*
* 返回值： 0 成功  1失败
*
********************************************************/
int qsdk_delete_onenet_object(int objectid)
{
    int i=0,j=0;
    //循环查询并且删除 object
    for(i=0; i<data_stream.dev_len; i++)
    {
        //判断当前 object 是否为要删除的object
        if(data_stream.dev[i].objectid==objectid)
        {
            //发送命令删除 object
            if(at_exec_cmd(at_resp,"AT+MIPLDELOBJ=%d,%d",data_stream.instance,data_stream.dev[i].objectid)!=RT_EOK) return RT_ERROR;

            //删除数据流设备信息中的 object
            for(j=i; j<data_stream.dev_len; j++)
            {
                //删除当前 object
                rt_memset(&data_stream.dev[j],0,sizeof(data_stream.dev[j]));
                //判断后面会都还有object，如果有拷贝到当前位置
                if(j<data_stream.dev_len-1)
                    rt_memcpy(&data_stream.dev[j],&data_stream.dev[j+1],sizeof(data_stream.dev[j+1]));
            }
            //数据流中设备长度-1
            data_stream.dev_len-=1;
#ifdef QSDK_USING_LOG
            LOG_D("delete onenet object success id:%d \r\n",objectid);
#endif
            return RT_EOK;
        }
    }
    return RT_ERROR;
}
/****************************************************
* 函数名称： onenet_open
*
* 函数作用： 设备登录到 onenet 平台
*
* 入口参数： lifetime 设备在onenet 维持时间
*
* 返回值： 0 成功	1失败
*****************************************************/
int qsdk_onenet_open(int lifetime)
{
    data_stream.lifetime=lifetime;
    if(at_exec_cmd(at_resp,"AT+MIPLOPEN=%d,%d",data_stream.instance,data_stream.lifetime)!=RT_EOK) return RT_ERROR;
#ifdef QSDK_USING_LOG
    LOG_D("onenet open success\r\n");
#endif
    return RT_EOK;
}
/****************************************************
* 函数名称： onenet_close
*
* 函数作用： 在onenet 平台注销设备
*
* 入口参数： 无
*
* 返回值： 0 成功	1失败
*****************************************************/
int qsdk_onenet_close(void)
{
    if(at_exec_cmd(at_resp,"AT+MIPLCLOSE=%d",data_stream.instance)!=RT_EOK) return RT_ERROR;
#ifdef QSDK_USING_LOG
    LOG_D("onenet instance close success\r\n");
#endif
    return RT_EOK;
}
/****************************************************
* 函数名称： time_onenet_update
*
* 函数作用： 更新onenet 设备维持时间
*
* 入口参数： 无
*
* 返回值： 0 成功	1失败
*****************************************************/
int qsdk_time_onenet_update(int flge)
{
    if(at_exec_cmd(at_resp,"AT+MIPLUPDATE=%d,%d,%d",data_stream.instance,data_stream.lifetime,flge)!=RT_EOK) return RT_ERROR;
#ifdef QSDK_USING_LOG
    LOG_D("onenet update time success\r\n");
#endif
    return RT_EOK;
}
/****************************************************
* 函数名称： rsp_onenet_read
*
* 函数作用： onenet read 响应
*
* 入口参数： msgid	消息ID
*
*							objectid	即将操作的objectid
*
*							instanceid	即将操作的 instanceid
*
*							resourceid 即将操作的 instanceid
*
* 返回值： 0 成功	1失败
*****************************************************/
int qsdk_rsp_onenet_read(int msgid,int objectid,int instanceid,int resourceid)
{
    int i=0,result=qsdk_onenet_status_result_Not_Found;

    //循环检测objectid
    for(; i<data_stream.dev_len; i++)
    {
        //判断当前objectid 是否为即将要读的信息
        if(data_stream.dev[i].objectid==objectid&&data_stream.dev[i].instanceid==instanceid&&data_stream.dev[i].resourceid==resourceid)
        {
            //进入读回调函数
            if(data_stream.read_callback(objectid,instanceid,resourceid)==RT_EOK)
            {
                result=qsdk_onenet_status_result_read_success;
#ifdef QSDK_USING_LOG
                LOG_D("onenet read rsp success\r\n");
#endif
            }
            else	// read 回调函数处理失败操作
            {
                result=qsdk_onenet_status_result_Bad_Request;
                LOG_D("onenet read rsp failure\r\n");
            }
            break;
        }
    }
    //返回 onenet read 响应
    if(at_exec_cmd(at_resp,"AT+MIPLREADRSP=%d,%d,%d,%d,%d,%d,%d,%d,\"%s\",0,0",data_stream.instance,msgid,result,data_stream.dev[i].objectid,
                   data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].valuetype,strlen(data_stream.dev[i].value),
                   data_stream.dev[i].value)!=RT_EOK) return RT_ERROR;

    return RT_EOK;
}
/****************************************************
* 函数名称： rsp_onenet_write
*
* 函数作用： onenet write 响应
*
* 入口参数： msgid	消息ID
*
*							objectid	即将操作的objectid
*
*							instanceid	即将操作的 instanceid
*
*							resourceid 即将操作的 instanceid
*
*							valuetype write值类型
*
*							len write值长度
*
*							value 本次write值
*
* 返回值： 0 成功	1失败
*****************************************************/
int qsdk_rsp_onenet_write(int msgid,int objectid,int instanceid,int resourceid,int valuetype,int len,char* value)
{
    int result=qsdk_onenet_status_result_Not_Found;;

    //进入 write 回调函数
    if(data_stream.write_callback(objectid,instanceid,resourceid,len,value)==RT_EOK)
    {
        result=qsdk_onenet_status_result_write_success;
#ifdef QSDK_USING_LOG
        LOG_D("onenet write rsp success\r\n");
#endif
    }
    else	//write回调函数处理失败操作
    {
        result=qsdk_onenet_status_result_Bad_Request;
        LOG_D("onenet write rsp failure\r\n");
    }

    //返回 write	操作响应
    if(at_exec_cmd(at_resp,"AT+MIPLWRITERSP=%d,%d,%d",data_stream.instance,msgid,result)!=RT_EOK) return RT_ERROR;

    return RT_EOK;
}
/****************************************************
* 函数名称： qsdk_rsp_onenet_execute
*
* 函数作用： onenet execute 响应
*
* 入口参数： msgid	消息ID
*
*							objectid	即将操作的objectid
*
*							instanceid	即将操作的 instanceid
*
*							resourceid 即将操作的 instanceid
*
*							len cmd值长度
*
*							cmd 本次cmd值
*
* 返回值： 0 成功	1失败
*****************************************************/
int qsdk_rsp_onenet_execute(int msgid,int objeceid,int instanceid,int resourceid,int len,char *cmd)
{
    int result=qsdk_onenet_status_result_Not_Found;;

    //进入 execute 回调函数
    if(data_stream.execute_callback(objeceid,instanceid,resourceid,len,cmd)==RT_EOK)
    {
        result=qsdk_onenet_status_result_write_success;
#ifdef QSDK_USING_LOG
        LOG_D("onenet execute rsp success\r\n");
#endif
    }
    else	//execute回调函数处理失败操作
    {
        result=qsdk_onenet_status_result_Bad_Request;
        LOG_D("onenet execute rsp failure\r\n");
    }

    //返回 execute	操作响应
    if(at_exec_cmd(at_resp,"AT+MIPLEXECUTERSP=%d,%d,%d",data_stream.instance,msgid,result)!=RT_EOK) return RT_ERROR;

    return RT_EOK;
}

int qsdk_rsp_onenet_parameter(int instance,int msgid,int result)
{


    return 0;
}
/****************************************************
* 函数名称： notify_data_to_onenet
*
* 函数作用： notify 设备数据到平台（无ACK）
*
* 入口参数： up_status 数据上报类型  0 全部上报   1 只上报带有上报标识的
*
* 返回值： 0 成功	1失败
*****************************************************/
int qsdk_notify_data_to_onenet(_Bool up_status)
{
    int i=0;
    if(up_status!=0)	//判断是否为只上报带有标识的数据
    {
        //循环检测设备数据
        for(i=0; i<data_stream.dev_len; i++)
        {
            //判断当前设备数据是否开启数据上报标识
            if(data_stream.dev[i].up_status==1)
            {
                //上报当前设备数据
                if(at_exec_cmd(at_resp,"AT+MIPLNOTIFY=%d,%d,%d,%d,%d,%d,%d,\"%s\",0,%d",data_stream.instance,data_stream.dev[i].msgid,data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].valuetype,strlen(data_stream.dev[i].value),data_stream.dev[i].value,data_stream.dev[i].flge)!=RT_EOK)  return RT_ERROR;

                //清空当前数据流上报标识
                data_stream.dev[i].up_status=0;
#ifdef QSDK_USING_LOG
                //LOG_D("\r\n notify to onenet success, objectid:%d 	instanceid:%d 	resourceid:%d		msgid=%d   flge:%d",data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].msgid,data_stream.dev[i].flge);
                LOG_D("AT+MIPLNOTIFY=%d,%d,%d,%d,%d,%d,%d,\"%s\",0,%d\r\n",data_stream.instance,data_stream.dev[i].msgid,data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].valuetype,strlen(data_stream.dev[i].value),data_stream.dev[i].value,data_stream.dev[i].flge);
#endif
                rt_thread_mdelay(100);
            }
        }
        //清空设备总上报标识
        data_stream.update_status=0;
    }
    else if(up_status==0)	//如果上报标识为全部信息上报
    {
        //循环上报信息到平台
        for(i=0; i<data_stream.dev_len; i++)
        {
            //发送AT命令上报信息到平台
            if(at_exec_cmd(at_resp,"AT+MIPLNOTIFY=%d,%d,%d,%d,%d,%d,%d,\"%s\",0,%d",data_stream.instance,data_stream.dev[i].msgid,data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].valuetype,strlen(data_stream.dev[i].value),data_stream.dev[i].value,data_stream.dev[i].flge)!=RT_EOK) return RT_ERROR;
#ifdef QSDK_USING_LOG
            //LOG_D("\r\n notify to onenet success, objectid:%d 	instanceid:%d 	resourceid:%d		msgid=%d   flge:%d",data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].msgid,data_stream.dev[i].flge);
            LOG_D("AT+MIPLNOTIFY=%d,%d,%d,%d,%d,%d,%d,\"%s\",0,%d\r\n",data_stream.instance,data_stream.dev[i].msgid,data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].valuetype,strlen(data_stream.dev[i].value),data_stream.dev[i].value,data_stream.dev[i].flge);
#endif
            rt_thread_mdelay(100);
        }
    }

    return RT_EOK;
}
/****************************************************
* 函数名称： notify_ack_data_to_onenet
*
* 函数作用： notify 数据到onenet平台（带ACK）
*
* 入口参数： up_status 数据上报类型  0 全部上报   1 只上报带有上报标识的
*
* 返回值： 0 成功	1失败
*****************************************************/
int qsdk_notify_ack_data_to_onenet(_Bool up_status)
{
    int i=0,count=300;
    if(up_status!=0)	//判断上报类型是否为只上报带上报标识的
    {
        //循环检测并且上报数据到平台
        for(i=0; i<data_stream.dev_len; i++)
        {
            //判断当前设备数据是否带上报标识
            if(data_stream.dev[i].up_status==1)
            {
                //发送AT命令上报数据到 onenet平台
                if(at_exec_cmd(at_resp,"AT+MIPLNOTIFY=%d,%d,%d,%d,%d,%d,%d,\"%s\",0,%d,%d",data_stream.instance,data_stream.dev[i].msgid,data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].valuetype,strlen(data_stream.dev[i].value),data_stream.dev[i].value,data_stream.dev[i].flge,data_stream.notify_ack)!=RT_EOK) return RT_ERROR;

                //等待平台返回ACK
                do {
                    count--;
                    rt_thread_mdelay(100);
                } while(data_stream.notify_status==qsdk_onenet_status_init&&count>0);

                //判断 notify 状态是否失败
                if(data_stream.notify_status==qsdk_onenet_status_failure||count<0)
                {
                    LOG_D("ack notify to onenet failure, objectid:%d 	instanceid:%d 	resourceid:%d		msgid:%d\r\n",data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].msgid);
                    return RT_ERROR;
                }
                else if(data_stream.notify_status==qsdk_onenet_status_success)	//notify 成功后操作
                {
#ifdef QSDK_USING_LOG
                    LOG_D("ack notify to onenet success, objectid:%d 	instanceid:%d 	resourceid:%d		msgid:%d\r\n",data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].msgid);
#endif
                    data_stream.notify_status=qsdk_onenet_status_init;
                }
                //数据流 ack增加一次  该objectid 上报标识清空
                data_stream.notify_ack++;
                data_stream.dev[i].up_status=0;
            }
        }
        //设备总上报标识清空
        data_stream.update_status=0;
    }
    else if(up_status==0) //判断上报状态是否为全部上报
    {
        //循环检测设备总数据流
        for(i=0; i<data_stream.dev_len; i++)
        {
            //发送 AT 命令上报数据到 onenet 平台
            if(at_exec_cmd(at_resp,"AT+MIPLNOTIFY=%d,%d,%d,%d,%d,%d,%d,\"%s\",0,%d,%d",data_stream.instance,data_stream.dev[i].msgid,data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].valuetype,strlen(data_stream.dev[i].value),data_stream.dev[i].value,data_stream.dev[i].flge,data_stream.notify_ack)!=RT_EOK) return RT_ERROR;

            //等待模组返回 ACK响应
            do {
                count--;
                rt_thread_mdelay(100);
            } while(data_stream.notify_status==qsdk_onenet_status_init&&count>0);

            //判断 ACK 响应是否为失败
            if(data_stream.notify_status==qsdk_onenet_status_failure||count<0)
            {
                LOG_D("ack notify to onenet failure, objectid:%d 	instanceid:%d 	resourceid:%d		msgid:%d\r\n",data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].msgid);
                return RT_ERROR;
            }
            else if(data_stream.notify_status==qsdk_onenet_status_success) // ACK响应成功后操作
            {
#ifdef QSDK_USING_LOG
                LOG_D("ack notify to onenet success, objectid:%d 	instanceid:%d 	resourceid:%d		msgid:%d\r\n",data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].msgid);
#endif
                //将当前设备数据流的上报状态设为初始化
                data_stream.notify_status=qsdk_onenet_status_init;
            }
            // ACK 增加一次
            data_stream.notify_ack++;
        }
    }
    //当ACK超过5000 时，清空一次
    if(data_stream.notify_ack>5000)
        data_stream.notify_ack=1;
    return RT_EOK;
}
/****************************************************
* 函数名称： notify_data_to_status
*
* 函数作用： notify 数据到程序缓存，方便后期一起上报
*
* 入口参数： objectid 需要写入缓存的 objectid
*
* 						instanceid 需要写入缓存的 instanceid
*
* 						resourceid 需要写入缓存的 resourceid
*
* 返回值： 0 成功	1失败
*****************************************************/
int qsdk_notify_data_to_status(int objectid,int instanceid,int resourceid)
{
    int i=0;

    //循环检测设备总数据流
    for(i=0; i<data_stream.dev_len; i++)
    {
        //判断当前 objectid 是否为写入缓存的 objectid
        if(data_stream.dev[i].objectid==objectid&&data_stream.dev[i].instanceid==instanceid&&data_stream.dev[i].resourceid==resourceid)
        {
            //数据流上报标识置 1
            data_stream.dev[i].up_status=1;
#ifdef QSDK_USING_LOG
            LOG_D("notify data to status success \r\n objectid=%d ,instanceid=%d ,resourcrid=%d \r\n",objectid,instanceid,resourceid);
#endif
            break;
        }
    }
    return RT_EOK;
}
/****************************************************
* 函数名称： notify_urgent_event
*
* 函数作用： 紧急发送数据到 onenet平台
*
* 入口参数： objectid 需要发送的 objectid
*
* 						instanceid 需要发送的 instanceid
*
* 						resourceid 需要发送的 resourceid
*
* 返回值： 0 成功	1失败
*****************************************************/
int qsdk_notify_urgent_event(int objectid,int instanceid,int resourceid)
{
    int i=0;

    //循环检测设备总数据流
    for(i=0; i<data_stream.dev_len; i++)
    {
        //判断当前 objectid 是否为需要上报的 objectid
        if(data_stream.dev[i].objectid==objectid&&data_stream.dev[i].instanceid==instanceid&&data_stream.dev[i].resourceid==resourceid)
        {
            //发送 AT 命令上报到 onenet 平台
            if(at_exec_cmd(at_resp,"AT+MIPLNOTIFY=%d,%d,%d,%d,%d,%d,%d,\"%s\",0,%d",data_stream.instance,data_stream.dev[i].msgid,data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].valuetype,strlen(data_stream.dev[i].value),data_stream.dev[i].value,data_stream.dev[i].flge)!=RT_EOK)  return RT_ERROR;

#ifdef QSDK_USING_LOG
            LOG_D("notify urgent event success \r\n objectid=%d ,instanceid=%d ,resourcrid=%d \r\n",objectid,instanceid,resourceid);
#endif
            break;
        }
    }
    return RT_EOK;
}
/****************************************************
* 函数名称： get_onenet_version
*
* 函数作用： 获取 onenet平台版本号
*
* 返回值： 0 成功	1失败
*****************************************************/
int qsdk_get_onenet_version(void)
{


    return 0;
}

/****************************************************
* 函数名称： head_node_parse
*
* 函数作用： 生成注册码协议 head 部分
*
* 入口参数： str:生成的协议文件
*
* 返回值： 0 成功	1失败
*****************************************************/
static int head_node_parse(char *str)
{
    int ver=1;
    int config=3;
    rt_sprintf(str,"%d%d",ver,config);
    //LOG_D("%s\r\n",str);
    return RT_EOK;
}
/****************************************************
* 函数名称： init_node_parse
*
* 函数作用： 生成注册码协议 init 部分
*
* 入口参数： str:生成的协议文件
*
* 返回值： 0 成功	1失败
*****************************************************/
static int init_node_parse(char *str)
{
    char *head="F";
    int config=1;
    int size=3;
    rt_sprintf(str+strlen(str),"%s%d%04x",head,config,size);
    //LOG_D("%s\r\n",str);

    return RT_EOK;
}
/****************************************************
* 函数名称： net_node_parse
*
* 函数作用： 生成注册码协议 net 部分
*
* 入口参数： str:生成的协议文件
*
* 返回值： 0 成功	1失败
*****************************************************/
static int net_node_parse(char *str)
{
    char buffer[100];
    char *head="F";
    int config=2;
#ifdef QSDK_USING_ME3616
    int mtu_size=1280;
#else
    int mtu_size=1024;
#endif
    int link_t=1;
    int band_t=1;
    int boot_t=00;
    int apn_len=0;
    int apn=0;
    int user_name_len=0;
    int user_name=0;
    int passwd_len=0;
    int passwd=0;
    int host_len;
    char host[38];
#ifdef QSDK_USING_ME3616
    char *user_data="4e554c4c";
#else
    char *user_data="31";
#endif
    int user_data_len=strlen(user_data)/2;
#ifdef QSDK_USING_ME3616
    string_to_hex(QSDK_ONENET_ADDRESS,strlen(QSDK_ONENET_ADDRESS),host);
#else
    LOG_D(buffer,"%s:%s",QSDK_ONENET_ADDRESS,QSDK_ONENET_PORT);
    string_to_hex(buffer,strlen(buffer),host);
#endif
    host_len=strlen(host)/2;
    //LOG_D("%s  %d\r\n",host,host_len);
    rt_sprintf(buffer,"%04x%x%x%02x%02x%02x%02x%02x%02x%02x%02x%s%04x%s",mtu_size,link_t,band_t,boot_t,apn_len,apn,user_name_len,user_name,passwd_len,passwd,host_len,host,user_data_len,user_data);
    //LOG_D("%s\r\n",buffer);
    rt_sprintf(str+strlen(str),"%s%d%04x%s",head,config,strlen(buffer)/2+3,buffer);
    //LOG_D("%s\r\n",str);

    return 0;

}
/****************************************************
* 函数名称： sys_node_parse
*
* 函数作用： 生成注册码协议 sys 部分
*
* 入口参数： str:生成的协议文件
*
* 返回值： 0 成功	1失败
*****************************************************/
static int sys_node_parse(char *str)
{
    char buffer[20];
    char *head="F";
    int config=3;
    int size;
#ifdef QSDK_USING_ME3616
    char *debug="ea0400";
#else
    char *debug="C00000";
#endif

#ifdef QSDK_USING_ME3616
    char *user_data="4e554c4c";
    int user_data_len=strlen(user_data)/2;
#else
    char *user_data="00";
    int user_data_len=0;
#endif

    if(user_data_len)
        rt_sprintf(buffer,"%s%04x%s",debug,user_data_len,user_data);
    else
        rt_sprintf(buffer,"%s%04x",debug,user_data_len);
    rt_sprintf(str+strlen(str),"%s%x%04x%s",head,config,strlen(buffer)/2+3,buffer);
    //LOG_D("%s\r\n",str);

    return 0;

}
/****************************************************
* 函数名称： onenet_dis_error
*
* 函数作用： 打印错误信息
*
* 返回值： 无
*****************************************************/
void qsdk_onenet_dis_error(void)
{
    switch(data_stream.error)
    {
    case 0:
        LOG_D("onenet sdk init success\r\n");
        break;
    case 1:
        LOG_D("onenet create instance failure\r\n");
        break;
    case 2:
        LOG_D("onenet add objece failure\r\n");
        break;
    case 3:
        LOG_D("onenet notify failure\r\n");
        break;
    case 4:
        LOG_D("onener open failure\r\n");
        break;
    case 5:
        LOG_D("onenet open init failure\r\n");
        break;
    case 6:
        LOG_D("onenet open run failure\r\n");
        break;
    case 7:
        LOG_D("onenet open failure\r\n");
        break;
    case 8:
        LOG_D("observe init failure\r\n");
        break;
    case 9:
        LOG_D("observe run failure\r\n");
        break;
    case 10:
        LOG_D("observe  failure\r\n");
        break;
    case 11:
        LOG_D("discover init failure\r\n");
        break;
    case 12:
        LOG_D("discover run failure\r\n");
        break;
    case 13:
        LOG_D("discover failure\r\n");
        break;
    case 14:
        LOG_D("no sim\r\n");
        break;
    case 15:
        LOG_D("no sim\r\n");
        break;
    case 16:
        LOG_D("no sim\r\n");
        break;
    case 17:
        LOG_D("no sim\r\n");
        break;
    case 18:
        LOG_D("no sim\r\n");
        break;
    default:
        break;
    }
}

#endif
