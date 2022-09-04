/*
 * ccu6.h
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#ifndef USERS_BSP_CCU6_H_
#define USERS_BSP_CCU6_H_

#include "Ifx_Types.h"

typedef enum {
    CCU6_0 = 0,
    CCU6_1,
} CCU6N_enum;

typedef enum {
    PIT_CH0 = 0,
    PIT_CH1,
} CCU6_CHN_enum;

typedef struct
{
    void (*Init)(CCU6N_enum ccu6n, CCU6_CHN_enum pit_ch, uint32 time);
    void (*Start)(CCU6N_enum ccu6n, CCU6_CHN_enum pit_ch);
    void (*Stop)(CCU6N_enum ccu6n, CCU6_CHN_enum pit_ch);
    void (*ClearFlag)(CCU6N_enum ccu6n, CCU6_CHN_enum pit_ch);
} CCU6_TypeDef;

extern CCU6_TypeDef CCU6;

#endif /* USERS_BSP_CCU6_H_ */
