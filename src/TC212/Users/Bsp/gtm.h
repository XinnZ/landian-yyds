/*
 * gtm.h
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#ifndef USERS_BSP_GTM_H_
#define USERS_BSP_GTM_H_

#include "IfxGtm_PinMap.h"
#include "Ifx_Types.h"

#define CMU_CLK_FREQ 3125000.0f    /* CMU clock frequency, in Hertz                    */
#define GTM_TOM_PWM_DUTY_MAX 10000 /* GTM_ATOM PWM maximum duty cycle                  */

typedef struct
{
    void (*Init)(IfxGtm_Tom_ToutMap* Tom_Channel, uint32 freq, uint32 dutyCycle);
    void (*SetDuty)(IfxGtm_Tom_ToutMap* Tom_Channel, uint32 dutyCycle);
} GTM_TypeDef;

extern GTM_TypeDef PWM;

#endif /* USERS_BSP_GTM_H_ */
