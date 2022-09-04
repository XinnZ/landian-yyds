/*
 * Cpu0_Main.c
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include <HeadFiles.h>

IfxCpu_syncEvent g_cpuSyncEvent = 0;

int core0_main(void) {
    /* Disable interruption */
    IfxCpu_disableInterrupts();

    /* !!WATCHDOG0 AND SAFETY WATCHDOG ARE DISABLED HERE!!
     * Enable the watchdogs and service them periodically if it is required
     */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());

    /* Wait for CPU sync event */
    IfxCpu_emitEvent(&g_cpuSyncEvent);
    IfxCpu_waitEvent(&g_cpuSyncEvent, 1);

    /* Initialize all functions */
    UART_EdgeBoard.Init();
    UART_Console.Init();

    GPIO.Init();
    Motor.Init();
    Servo.Init();
    // ICM.Init();
    Run.Init();
    Xcmd.Init();

    /* Enable interruption */
    IfxCpu_enableInterrupts();

    Run_Params.Buzzer = 5;

    while (1) {
        /* Receiving data from EdgeBoard */
        Edge.Rece();

        /* Receiving data from Upper Computer */
        Xcmd.Rece();

        /* Low battery alarm */
        if (!GPIO.Get(IFXCFG_PORT_POWER)) {
            GPIO.High(IFXCFG_PORT_BUZZ);
        }
    }

    return 0;
}
