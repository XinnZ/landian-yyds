/*
 * ccu6.c
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include <ccu6.h>
#include <Isr.h>

#include "IfxCcu6.h"
#include "IfxCcu6_Timer.h"
#include "Scu/Std/IfxScuCcu.h"
#include "SysSe/Bsp/Bsp.h"

static void Init(CCU6N_enum ccu6n, CCU6_CHN_enum pit_ch, uint32 time) {
    uint8 i = 0;
    uint32 timer_period;
    uint64 timer_input_clk;

    volatile Ifx_CCU6 *module;        /* CCU6 will be used                                        */
    IfxCcu6_Timer g_Ccu6Timer;        /* Timer structure                                          */
    IfxCcu6_Timer_Config timerConfig; /* Structure for timer configuration                        */

    module = IfxCcu6_getAddress((IfxCcu6_Index)ccu6n);
    IfxCcu6_Timer_initModuleConfig(&timerConfig, module); /* Initialize the timer module structure with default values */

    /* The Serial Peripheral Bus has a default Frequency of Fcc6 = 100000000 Hz = 100 MHz
     * Possible frequencies for the CCU6 timer are:
     *       - 100000000 Hz = 100 MHz   (Fcc6)
     *       - 50000000 Hz  = 50 MHz    (Fcc6/2)
     *       - 25000000 Hz  = 25 MHz    (Fcc6/4)
     *       - 12500000 Hz  = 12.5 MHz  (Fcc6/8)
     *       - 6250000 Hz   = 6.25 MHz  (Fcc6/16)
     *       - 3125000 Hz   ~ 3 MHz     (Fcc6/32)
     *       - 1562500 Hz   ~ 1.5 MHz   (Fcc6/64)
     *       - 781250 Hz    ~ 780 KHz   (Fcc6/128)
     *       - 390625 Hz    ~ 390 KHz   (Fcc6/256)
     *       - 195312.5 Hz  ~ 200 KHz   (Fcc6/512)
     *       - 97656.25 Hz  ~ 100 KHz   (Fcc6/1024)
     *       - 48828.12 Hz  ~ 50 KHz    (Fcc6/2048)
     *       - 24414.06 Hz  ~ 25 KHz    (Fcc6/4096)
     *       - 12207.03 Hz  ~ 12.5 KHz  (Fcc6/8192)
     *       - 6103.51 Hz   ~ 6 KHz     (Fcc6/16384)
     *       - 3051.75 Hz   ~ 3 KHz     (Fcc6/32768)
     */
    timer_input_clk = IfxScuCcu_getSpbFrequency();
    i = 0;
    while (i < 16) {
        timer_period = (uint32)(timer_input_clk * time / 1000000);
        if (timer_period < 0xffff) {
            break;
        }
        timer_input_clk >>= 1;
        i++;
    }
    if (16 <= i)
        IFX_ASSERT(IFX_VERBOSE_LEVEL_ERROR, FALSE);

    switch (ccu6n) {
        case CCU6_0: {
            if (PIT_CH0 == pit_ch) {
                timerConfig.interrupt1.typeOfService = CCU6_0_CH0_INT_SERVICE;
                timerConfig.interrupt1.priority = CCU6_0_CH0_ISR_PRIORITY;
            } else {
                timerConfig.interrupt2.typeOfService = CCU6_0_CH1_INT_SERVICE;
                timerConfig.interrupt2.priority = CCU6_0_CH1_ISR_PRIORITY;
            }
        } break;

        case CCU6_1: {
            if (PIT_CH0 == pit_ch) {
                timerConfig.interrupt1.typeOfService = CCU6_1_CH0_INT_SERVICE;
                timerConfig.interrupt1.priority = CCU6_1_CH0_ISR_PRIORITY;
            } else {
                timerConfig.interrupt2.typeOfService = CCU6_1_CH1_INT_SERVICE;
                timerConfig.interrupt2.priority = CCU6_1_CH1_ISR_PRIORITY;
            }
        } break;
    }

    if (PIT_CH0 == pit_ch) {
        timerConfig.timer = IfxCcu6_TimerId_t12;
        timerConfig.interrupt1.source = IfxCcu6_InterruptSource_t12PeriodMatch;
        timerConfig.interrupt1.serviceRequest = IfxCcu6_ServiceRequest_1;
        timerConfig.base.t12Period = timer_period;
        timerConfig.base.t12Frequency = (float)timer_input_clk;
        timerConfig.clock.t12countingInputMode = IfxCcu6_CountingInputMode_internal;
    } else {
        timerConfig.timer = IfxCcu6_TimerId_t13;
        timerConfig.interrupt2.source = IfxCcu6_InterruptSource_t13PeriodMatch;
        timerConfig.interrupt2.serviceRequest = IfxCcu6_ServiceRequest_2;
        timerConfig.base.t13Period = timer_period;
        timerConfig.base.t13Frequency = (float)timer_input_clk;
        timerConfig.clock.t13countingInputMode = IfxCcu6_CountingInputMode_internal;
    }

    timerConfig.timer12.counterValue = 0;
    timerConfig.timer13.counterValue = 0;
    timerConfig.trigger.t13InSyncWithT12 = FALSE; /* Configure timers synchronization            */

    IfxCcu6_Timer_initModule(&g_Ccu6Timer, &timerConfig); /* Initialize the CCU6 module                  */
    IfxCcu6_setSuspendMode(module, IfxCcu6_SuspendMode_hard);
}

static void Start(CCU6N_enum ccu6n, CCU6_CHN_enum pit_ch) {
    volatile Ifx_CCU6 *module;
    IfxCcu6_Timer g_Ccu6Timer;

    module = IfxCcu6_getAddress((IfxCcu6_Index)ccu6n);

    g_Ccu6Timer.ccu6 = module;
    g_Ccu6Timer.timer = (IfxCcu6_TimerId)(pit_ch);

    IfxCcu6_Timer_start(&g_Ccu6Timer);
}

static void Stop(CCU6N_enum ccu6n, CCU6_CHN_enum pit_ch) {
    volatile Ifx_CCU6 *module;
    IfxCcu6_Timer g_Ccu6Timer;

    module = IfxCcu6_getAddress((IfxCcu6_Index)ccu6n);

    g_Ccu6Timer.ccu6 = module;
    g_Ccu6Timer.timer = (IfxCcu6_TimerId)(pit_ch);

    IfxCcu6_Timer_stop(&g_Ccu6Timer);
}

static void ClearFlag(CCU6N_enum ccu6n, CCU6_CHN_enum pit_ch) {
    IfxCcu6_clearInterruptStatusFlag(IfxCcu6_getAddress((IfxCcu6_Index)ccu6n), (IfxCcu6_InterruptSource)(7 + (pit_ch * 2)));
}

CCU6_TypeDef CCU6 =
    {
        .Init = Init,
        .Start = Start,
        .Stop = Stop,
        .ClearFlag = ClearFlag};
