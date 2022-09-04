/*
 * gpt12.c
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include <gpt12.h>

#include "IfxGpt12_IncrEnc.h"
#include "IfxGpt12_PinMap.h"

static void Init(GPTN_enum gptn, IfxGpt12_TxIn_In *Gpt12_TxIn_Pin, IfxGpt12_TxEud_In *Gpt12_TxEud_Pin) {
    /* Initialize the GPT12 module */
    IfxGpt12_enableModule(&MODULE_GPT120);                                         /* Enable the GPT12 module      */
    IfxGpt12_setGpt1BlockPrescaler(&MODULE_GPT120, IfxGpt12_Gpt1BlockPrescaler_4); /* Set GPT1 block prescaler     */
    IfxGpt12_setGpt2BlockPrescaler(&MODULE_GPT120, IfxGpt12_Gpt2BlockPrescaler_4); /* Set GPT2 block prescaler     */

    IfxGpt12_initTxInPinWithPadLevel(Gpt12_TxIn_Pin, IfxPort_InputMode_pullUp, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGpt12_initTxEudInPinWithPadLevel(Gpt12_TxEud_Pin, IfxPort_InputMode_pullUp, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    switch (gptn) {
        case GPT12_T2: {
            IfxGpt12_T2_setCounterInputMode(&MODULE_GPT120, IfxGpt12_CounterInputMode_risingEdgeTxIN);
            IfxGpt12_T2_setDirectionSource(&MODULE_GPT120, IfxGpt12_TimerDirectionSource_external);
            IfxGpt12_T2_setMode(&MODULE_GPT120, IfxGpt12_Mode_counter);
            IfxGpt12_T2_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
        } break;

        case GPT12_T3: {
            IfxGpt12_T3_setCounterInputMode(&MODULE_GPT120, IfxGpt12_CounterInputMode_risingEdgeTxIN);
            IfxGpt12_T3_setDirectionSource(&MODULE_GPT120, IfxGpt12_TimerDirectionSource_external);
            IfxGpt12_T3_setMode(&MODULE_GPT120, IfxGpt12_Mode_counter);
            IfxGpt12_T3_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
        } break;

        case GPT12_T4: {
            IfxGpt12_T4_setCounterInputMode(&MODULE_GPT120, IfxGpt12_CounterInputMode_risingEdgeTxIN);
            IfxGpt12_T4_setDirectionSource(&MODULE_GPT120, IfxGpt12_TimerDirectionSource_external);
            IfxGpt12_T4_setMode(&MODULE_GPT120, IfxGpt12_Mode_counter);
            IfxGpt12_T4_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
        } break;

        case GPT12_T5: {
            IfxGpt12_T5_setCounterInputMode(&MODULE_GPT120, IfxGpt12_CounterInputMode_risingEdgeTxIN);
            IfxGpt12_T5_setDirectionSource(&MODULE_GPT120, IfxGpt12_TimerDirectionSource_external);
            IfxGpt12_T5_setMode(&MODULE_GPT120, IfxGpt12_Mode_counter);
            IfxGpt12_T5_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
        } break;

        case GPT12_T6: {
            IfxGpt12_T6_setCounterInputMode(&MODULE_GPT120, IfxGpt12_CounterInputMode_risingEdgeTxIN);
            IfxGpt12_T6_setDirectionSource(&MODULE_GPT120, IfxGpt12_TimerDirectionSource_external);
            IfxGpt12_T6_setMode(&MODULE_GPT120, IfxGpt12_Mode_counter);
            IfxGpt12_T6_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
        } break;
    }
}

static short Get(GPTN_enum gptn) {
    switch (gptn) {
        case GPT12_T2:
            return (short)IfxGpt12_T2_getTimerValue(&MODULE_GPT120);
        case GPT12_T3:
            return (short)IfxGpt12_T3_getTimerValue(&MODULE_GPT120);
        case GPT12_T4:
            return (short)IfxGpt12_T4_getTimerValue(&MODULE_GPT120);
        case GPT12_T5:
            return (short)IfxGpt12_T5_getTimerValue(&MODULE_GPT120);
        case GPT12_T6:
            return (short)IfxGpt12_T6_getTimerValue(&MODULE_GPT120);
        default:
            return 0;
    }
}

static void Clear(GPTN_enum gptn) {
    switch (gptn) {
        case GPT12_T2:
            IfxGpt12_T2_setTimerValue(&MODULE_GPT120, 0);
            break;
        case GPT12_T3:
            IfxGpt12_T3_setTimerValue(&MODULE_GPT120, 0);
            break;
        case GPT12_T4:
            IfxGpt12_T4_setTimerValue(&MODULE_GPT120, 0);
            break;
        case GPT12_T5:
            IfxGpt12_T5_setTimerValue(&MODULE_GPT120, 0);
            break;
        case GPT12_T6:
            IfxGpt12_T6_setTimerValue(&MODULE_GPT120, 0);
            break;
    }
}

GPT12_TypeDef GPT12 =
    {
        .Init = Init,
        .Get = Get,
        .Clear = Clear};
