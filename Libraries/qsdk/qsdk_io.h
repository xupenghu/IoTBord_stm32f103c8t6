/*
 * File      : qsdk_io.h
 * This file is part of io in qsdk
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-14     longmain     first version
 */
#ifndef __QSDK_IO_H__

#define __QSDK_IO_H__

#include "qsdk.h"

#include <drivers/pin.h>

#define NB_POWER_PIN	38	//PA15  D10
#define NB_RESET_PIN	22	//PB11  D9


int qsdk_hw_io_init(void);
int qsdk_hw_io_reboot(void);


#endif

