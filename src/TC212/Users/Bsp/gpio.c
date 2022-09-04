/*
 * gpio.c
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include <gpio.h>

#include "IfxPort.h"
#include "aurix_pin_mappings.h"

static void Init(void) {
    gpio_init_pins();
}

static void High(IfxPort_Pin gpio) {
    gpio.port->OMR.U = IfxPort_State_high << gpio.pinIndex;
}

static void Low(IfxPort_Pin gpio) {
    gpio.port->OMR.U = IfxPort_State_low << gpio.pinIndex;
}

static void Toggle(IfxPort_Pin gpio) {
    gpio.port->OMR.U = IfxPort_State_toggled << gpio.pinIndex;
}

static boolean Get(IfxPort_Pin gpio) {
    return (__getbit(&gpio.port->IN.U, gpio.pinIndex) != 0) ? TRUE : FALSE;
}

GPIO_TypeDef GPIO =
    {
        .Init = Init,
        .High = High,
        .Low = Low,
        .Toggle = Toggle,
        .Get = Get};
