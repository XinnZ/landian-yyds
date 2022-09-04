/*
 * Isr.c
 *
 *  Created on: 2022-1-1
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include <HeadFiles.h>
#include <Isr.h>

/*********************************************************************************************************************/
/*----------------------------------------------------- CCU6 --------------------------------------------------------*/
/*********************************************************************************************************************/
IFX_INTERRUPT(CCU60_CH0_Isr, 0, CCU6_0_CH0_ISR_PRIORITY) {
    static uint8 timer = 0;

    IfxCpu_enableInterrupts();
    CCU6.ClearFlag(CCU6_0, PIT_CH0);

    if (timer++ > 50) {
        timer = 0;
        GPIO.Toggle(IFXCFG_PORT_LED2);
    }

    /* Running Auto control tasks */
    Run.Task_Auto();

    /* Running WS2812 control tasks */
    Run.Task_Rgb();
}

IFX_INTERRUPT(CCU60_CH1_Isr, 0, CCU6_0_CH1_ISR_PRIORITY) {
    static uint8 timer = 0;

    IfxCpu_enableInterrupts();
    CCU6.ClearFlag(CCU6_0, PIT_CH1);

    if (timer++ > 50) {
        timer = 0;
        GPIO.Toggle(IFXCFG_PORT_LED3);
    }

    /* Running data processing tasks */
    Run.Task_Datas();
}

IFX_INTERRUPT(CCU61_CH0_Isr, 0, CCU6_1_CH0_ISR_PRIORITY) {
    static uint8 timer = 0;

    IfxCpu_enableInterrupts();
    CCU6.ClearFlag(CCU6_1, PIT_CH0);

    if (timer++ > 50) {
        timer = 0;
        GPIO.Toggle(IFXCFG_PORT_LED4);
    }

    /* Running Manual control tasks */
    Run.Task_Manual();
}

IFX_INTERRUPT(CCU61_CH1_Isr, 0, CCU6_1_CH1_ISR_PRIORITY) {
    static uint8 timer = 0;

    IfxCpu_enableInterrupts();
    CCU6.ClearFlag(CCU6_1, PIT_CH1);

    if (timer++ > 50) {
        timer = 0;
        GPIO.Toggle(IFXCFG_PORT_LED5);
    }

    /* Running the task of sending data to the Upper */
    Run.Task_Upper();
}

/*********************************************************************************************************************/
/*------------------------------------------------- UART Console ----------------------------------------------------*/
/*********************************************************************************************************************/
extern IfxStdIf_DPipe UART_Console_StdIf;

IFX_INTERRUPT(UART_Console_TxIsr, 0, UART_Console_ISR_PRIORITY_TX) {
    IfxCpu_enableInterrupts();
    IfxStdIf_DPipe_onTransmit(&UART_Console_StdIf);

    UART_Console.FlagTransmitted = 1;
}

IFX_INTERRUPT(UART_Console_RxIsr, 0, UART_Console_ISR_PRIORITY_RX) {
    IfxCpu_enableInterrupts();
    IfxStdIf_DPipe_onReceive(&UART_Console_StdIf);

    UART_Console.FlagReceived = 1;
}

IFX_INTERRUPT(UART_Console_ErrIsr, 0, UART_Console_ISR_PRIORITY_ER) {
    IfxCpu_enableInterrupts();
    IfxStdIf_DPipe_onError(&UART_Console_StdIf);
}

/*********************************************************************************************************************/
/*------------------------------------------------ UART EdgeBoard ---------------------------------------------------*/
/*********************************************************************************************************************/
extern IfxStdIf_DPipe UART_EdgeBoard_StdIf;

IFX_INTERRUPT(UART_EdgeBoard_TxIsr, 0, UART_EdgeBoard_ISR_PRIORITY_TX) {
    IfxCpu_enableInterrupts();
    IfxStdIf_DPipe_onTransmit(&UART_EdgeBoard_StdIf);

    UART_EdgeBoard.FlagTransmitted = 1;
}

IFX_INTERRUPT(UART_EdgeBoard_RxIsr, 0, UART_EdgeBoard_ISR_PRIORITY_RX) {
    IfxCpu_enableInterrupts();
    IfxStdIf_DPipe_onReceive(&UART_EdgeBoard_StdIf);

    UART_EdgeBoard.FlagReceived = 1;
}

IFX_INTERRUPT(UART_EdgeBoard_ErrIsr, 0, UART_EdgeBoard_ISR_PRIORITY_ER) {
    IfxCpu_enableInterrupts();
    IfxStdIf_DPipe_onError(&UART_EdgeBoard_StdIf);
}
