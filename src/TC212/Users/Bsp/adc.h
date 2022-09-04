/*
 * adc.h
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#ifndef USERS_BSP_ADC_H_
#define USERS_BSP_ADC_H_

#include "Ifx_Types.h"

typedef struct
{
    void (*Init)(void);
    void (*Read)(uint16 *);
} ADC_TypeDef;

extern ADC_TypeDef ADC;

#endif /* USERS_BSP_ADC_H_ */
