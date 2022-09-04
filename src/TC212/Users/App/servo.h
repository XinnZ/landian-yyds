/*
 * servo.h
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#ifndef USERS_APP_SERVO_H_
#define USERS_APP_SERVO_H_

#include "Ifx_Types.h"

#define PIN_SERVO 		IfxGtm_TOM1_3_TOUT97_P11_6_OUT

#define SERVO_MIN 		480
#define SERVO_MAX 		750


typedef struct
{
    void (*Init)(void);
    void (*Duty)(uint32 _duty);

    void (*MedianWrite)(uint32 _duty);
    void (*MedianRead)(void);

    uint32 median;
} Servo_TypeDef;

extern Servo_TypeDef Servo;


#endif /* USERS_APP_SERVO_H_ */
