/*
 * HeadFiles.h
 *
 *  Created on: 2022-1-1
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#ifndef USERS_HEADFILES_H_
#define USERS_HEADFILES_H_

#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "IfxAsclin_Asc.h"
#include "IfxCpu.h"
#include "IfxPort.h"
#include "IfxScuCcu.h"
#include "IfxScuWdt.h"
#include "IfxStdIf_DPipe.h"
#include "IfxStm.h"
#include "Ifx_Types.h"

#include "SysSe/Bsp/Bsp.h"
#include "SysSe/Time/Ifx_DateTime.h"

#include "aurix_pin_mappings.h"


#include "App/attitude.h"
#include "App/edgeboard.h"
#include "App/icm20602.h"
#include "App/motor.h"
#include "App/pid.h"
#include "App/rgb.h"
#include "App/run.h"
#include "App/servo.h"

#include "Bsp/adc.h"
#include "Bsp/ccu6.h"
#include "Bsp/flash.h"
#include "Bsp/gpio.h"
#include "Bsp/gpt12.h"
#include "Bsp/gtm.h"
#include "Bsp/qspi.h"
#include "Bsp/scu.h"
#include "Bsp/uart.h"

#include "Xcmd/xcmd.h"

#include "Isr.h"


#define delay_us(time) waitTime(IfxStm_getTicksFromMicroseconds(BSP_DEFAULT_TIMER, time))
#define delay_ms(time) waitTime(IfxStm_getTicksFromMilliseconds(BSP_DEFAULT_TIMER, time))


#endif /* USERS_HEADFILES_H_ */
