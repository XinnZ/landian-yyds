/*
 * motor.h
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#ifndef USERS_APP_MOTOR_H_
#define USERS_APP_MOTOR_H_

#include "Ifx_Types.h"
#include "aurix_pin_mappings.h"


#define PIN_SPEED 		IfxGtm_TOM0_4_TOUT95_P11_2_OUT
#define PIN_DIR 		IFXCFG_PORT_DIR
#define GPT12_T 		GPT12_T2
#define GPT12_IN 		IfxGpt120_T2INB_P33_7_IN
#define GPT12_EUD 		IfxGpt120_T2EUDB_P33_6_IN


typedef struct
{
    void (*Init)(void);
    void (*Speed)(sint32 value);
} Motor_TypeDef;

extern Motor_TypeDef Motor;


#endif /* USERS_APP_MOTOR_H_ */
