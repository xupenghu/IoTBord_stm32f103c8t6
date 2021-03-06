/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2017-10-25     ZYH               first implementation
 * 2018-10-23    XXXXzzzz000        first implementation,referance:stm32f4xx-HAL/drv_rtc.h
 */

#ifndef __DRV_RTC_H__
#define __DRV_RTC_H__
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include <string.h>
#include <time.h>

extern int rt_hw_rtc_init(void);
extern RTC_HandleTypeDef hrtc;
#endif
