/*
 * gpio.h
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#ifndef USERS_BSP_GPIO_H_
#define USERS_BSP_GPIO_H_

#include "IfxPort.h"
#include "IfxPort_PinMap.h"
#include "aurix_pin_mappings.h"

typedef struct
{
    void (*Init)(void);
    void (*High)(IfxPort_Pin gpio);
    void (*Low)(IfxPort_Pin gpio);
    void (*Toggle)(IfxPort_Pin gpio);
    boolean (*Get)(IfxPort_Pin gpio);
} GPIO_TypeDef;

extern GPIO_TypeDef GPIO;

#endif /* USERS_BSP_GPIO_H_ */
