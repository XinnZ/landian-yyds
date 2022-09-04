/*
 * servo.c
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include "servo.h"

#include "flash.h"
#include "gtm.h"

static void Duty(uint32 _duty) {
    if (_duty < SERVO_MIN) {
        _duty = SERVO_MIN;
    }
    if (_duty > SERVO_MAX) {
        _duty = SERVO_MAX;
    }

    PWM.SetDuty(&PIN_SERVO, _duty);
}

static void MedianWrite(uint32 _duty) {
    uint32 write_buf = (*(uint32 *)&_duty);

    Flash.Erase(flash_work);
    Flash.Write(flash_work, 0, &write_buf);

    Servo.median = (*(uint32 *)(Flash.Read(flash_work, 0)));
}

static void MedianRead(void) {
    Servo.median = (*(uint32 *)(Flash.Read(flash_work, 0)));
}

static void Init(void) {
    MedianRead();

    PWM.Init(&PIN_SERVO, 50, Servo.median);
}

Servo_TypeDef Servo =
    {
        .Init = Init,
        .Duty = Duty,

        .MedianWrite = MedianWrite,
        .MedianRead = MedianRead,

        .median = 600};
