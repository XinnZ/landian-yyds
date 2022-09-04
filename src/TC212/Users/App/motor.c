/*
 * motor.c
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include "motor.h"

#include "gpio.h"
#include "gpt12.h"
#include "gtm.h"

static void Init(void) {
    GPT12.Init(GPT12_T, &GPT12_IN, &GPT12_EUD);

    GPIO.Low(PIN_DIR);
    PWM.Init(&PIN_SPEED, 17000, 0);
}

static void Speed(sint32 value) {
    if (value > 0 + GTM_TOM_PWM_DUTY_MAX) {
        value = 0 + GTM_TOM_PWM_DUTY_MAX;
    }
    if (value < 0 - GTM_TOM_PWM_DUTY_MAX) {
        value = 0 - GTM_TOM_PWM_DUTY_MAX;
    }

    if (value >= 0) {
        GPIO.High(PIN_DIR);
        PWM.SetDuty(&PIN_SPEED, (uint32)(0 + value));
    } else {
        GPIO.Low(PIN_DIR);
        PWM.SetDuty(&PIN_SPEED, (uint32)(0 - value));
    }
}

Motor_TypeDef Motor =
    {
        .Init = Init,
        .Speed = Speed};
