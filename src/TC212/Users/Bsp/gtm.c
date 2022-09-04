/*
 * gtm.c
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include <gtm.h>

#include "IfxGtm_Tom_Pwm.h"
#include "Ifx_Types.h"

static void Init(IfxGtm_Tom_ToutMap* Tom_Channel, uint32 freq, uint32 dutyCycle) {
    IfxGtm_Tom_Pwm_Config g_tomConfig; /* Timer configuration structure                    */
    IfxGtm_Tom_Pwm_Driver g_tomDriver; /* Timer Driver structure                           */

    IFX_ASSERT(IFX_VERBOSE_LEVEL_ERROR, dutyCycle <= GTM_TOM_PWM_DUTY_MAX);

    IfxGtm_enable(&MODULE_GTM); /* Enable GTM                                       */
    if (!(MODULE_GTM.CMU.CLK_EN.U & 0x2)) {
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, CMU_CLK_FREQ);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, CMU_CLK_FREQ);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    IfxGtm_Tom_Pwm_initConfig(&g_tomConfig, &MODULE_GTM); /* Initialize default parameters    */

    g_tomConfig.tom = Tom_Channel->tom;            /* Select the tom depending on the LED     */
    g_tomConfig.tomChannel = Tom_Channel->channel; /* Select the channel depending on the LED  */
    g_tomConfig.period = CMU_CLK_FREQ / freq;      /* Set timer period                         */
    g_tomConfig.pin.outputPin = Tom_Channel;       /* Set LED as output                        */
    g_tomConfig.synchronousUpdateEnabled = TRUE;   /* Enable synchronous update                */

    g_tomConfig.dutyCycle = (uint32)((uint64)dutyCycle * g_tomConfig.period / GTM_TOM_PWM_DUTY_MAX);

    IfxGtm_Tom_Pwm_init(&g_tomDriver, &g_tomConfig); /* Initialize the PWM                       */
    IfxGtm_Tom_Pwm_start(&g_tomDriver, TRUE);        /* Start the PWM                            */
}

static void SetDuty(IfxGtm_Tom_ToutMap* Tom_Channel, uint32 dutyCycle) {
    uint32 duty, period;

    period = IfxGtm_Tom_Ch_getCompareZero(&MODULE_GTM.TOM[Tom_Channel->tom], Tom_Channel->channel);

    duty = (uint32)((uint64)dutyCycle * period / GTM_TOM_PWM_DUTY_MAX);

    IfxGtm_Tom_Ch_setCompareOneShadow(&MODULE_GTM.TOM[Tom_Channel->tom], Tom_Channel->channel, duty);
}

GTM_TypeDef PWM =
    {
        .Init = Init,
        .SetDuty = SetDuty};
