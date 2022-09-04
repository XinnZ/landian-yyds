/*
 * uart.c
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include <Isr.h>
#include <uart.h>

#include "IfxAsclin_Asc.h"
#include "IfxStdIf_DPipe.h"

#define ASC_TX_BUFFER_SIZE 256 /* Define the TX buffer size in byte    */
#define ASC_RX_BUFFER_SIZE 256 /* Define the RX buffer size in byte    */

uint16 CRC_CHECK(uint8 *Buf, uint8 CRC_CNT) {
    uint16 CRC_Temp = 0xffff;
    uint8 i, j;

    for (i = 0; i < CRC_CNT; i++) {
        CRC_Temp ^= Buf[i];
        for (j = 0; j < 8; j++) {
            if (CRC_Temp & 0x01)
                CRC_Temp = (CRC_Temp >> 1) ^ 0xa001;
            else
                CRC_Temp = CRC_Temp >> 1;
        }
    }
    return CRC_Temp;
}

/*********************************************************************************************************************/
/*------------------------------------------------- UART Console ----------------------------------------------------*/
/*********************************************************************************************************************/
IfxStdIf_DPipe UART_Console_StdIf; /* Standard interface object            */
IfxAsclin_Asc UART_Console_Asclin; /* ASCLIN module object                 */

/* Declaration of the FIFOs parameters */
uint8 UART_Console_TxBuffer[ASC_TX_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];
uint8 UART_Console_RxBuffer[ASC_RX_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];

static void UART_Console_Init(void) {
    IfxAsclin_Asc_Config ascConf;

    /* Set default configurations */
    IfxAsclin_Asc_initModuleConfig(&ascConf, &UART_Console_MODULE); /* Initialize the structure with default values */

    /* Set the desired baud rate */
    ascConf.baudrate.baudrate = UART_Console_BAUDRATE;               /* Set the baud rate in bit/s               */
    ascConf.baudrate.oversampling = IfxAsclin_OversamplingFactor_16; /* Set the oversampling factor              */

    /* Configure the sampling mode */
    ascConf.bitTiming.medianFilter = IfxAsclin_SamplesPerBit_three;          /* Set the number of samples per bit        */
    ascConf.bitTiming.samplePointPosition = IfxAsclin_SamplePointPosition_8; /* Set the first sample position    */

    /* ISR priorities and interrupt target */
    ascConf.interrupt.txPriority = UART_Console_ISR_PRIORITY_TX; /* Set the interrupt priority for TX events    */
    ascConf.interrupt.rxPriority = UART_Console_ISR_PRIORITY_RX; /* Set the interrupt priority for RX events    */
    ascConf.interrupt.erPriority = UART_Console_ISR_PRIORITY_ER; /* Set the interrupt priority for Error events */
    ascConf.interrupt.typeOfService = UART_Console_INT_SERVICE;

    /* Pin configuration */
    const IfxAsclin_Asc_Pins pins = {
        .cts = NULL_PTR, /* CTS pin not used                                     */
        .ctsMode = IfxPort_InputMode_pullUp,
        .rx = &UART_Console_RX_Pin,         /* Select the pin for RX connected to the USB port      */
        .rxMode = IfxPort_InputMode_pullUp, /* RX pin                                               */
        .rts = NULL_PTR,                    /* RTS pin not used                                     */
        .rtsMode = IfxPort_OutputMode_pushPull,
        .tx = &UART_Console_TX_Pin,            /* Select the pin for TX connected to the USB port      */
        .txMode = IfxPort_OutputMode_pushPull, /* TX pin                                               */
        .pinDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1};
    ascConf.pins = &pins;

    /* FIFO buffers configuration */
    ascConf.txBuffer = UART_Console_TxBuffer;  /* Set the transmission buffer                          */
    ascConf.txBufferSize = ASC_TX_BUFFER_SIZE; /* Set the transmission buffer size                     */
    ascConf.rxBuffer = UART_Console_RxBuffer;  /* Set the receiving buffer                             */
    ascConf.rxBufferSize = ASC_RX_BUFFER_SIZE; /* Set the receiving buffer size                        */

    /* Init ASCLIN module */
    IfxAsclin_Asc_initModule(&UART_Console_Asclin, &ascConf); /* Initialize the module with the given configuration */

    /* Initialize the Standard Interface */
    IfxAsclin_Asc_stdIfDPipeInit(&UART_Console_StdIf, &UART_Console_Asclin);
}

static void UART_Console_Transmit(const char *message, short *count) {
    if (!UART_Console_StdIf.txDisabled) {
        IfxStdIf_DPipe_write(&UART_Console_StdIf, (void *)message, count, TIME_INFINITE);
    }
}

static void UART_Console_Receive(const char *message, short *count) {
    IfxStdIf_DPipe_read(&UART_Console_StdIf, (void *)message, count, TIME_NULL);
}

UART_TypeDef UART_Console =
    {
        .Init = UART_Console_Init,
        .Transmit = UART_Console_Transmit,
        .Receive = UART_Console_Receive,

        .FlagTransmitted = 0,
		.FlagReceived = 0};

/*********************************************************************************************************************/
/*------------------------------------------------ UART EdgeBoard ---------------------------------------------------*/
/*********************************************************************************************************************/
IfxStdIf_DPipe UART_EdgeBoard_StdIf; /* Standard interface object            */
IfxAsclin_Asc UART_EdgeBoard_Asclin; /* ASCLIN module object                 */

/* Declaration of the FIFOs parameters */
uint8 UART_EdgeBoard_TxBuffer[ASC_TX_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];
uint8 UART_EdgeBoard_RxBuffer[ASC_RX_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];

static void UART_EdgeBoard_Init(void) {
    IfxAsclin_Asc_Config ascConf;

    /* Set default configurations */
    IfxAsclin_Asc_initModuleConfig(&ascConf, &UART_EdgeBoard_MODULE); /* Initialize the structure with default values */

    /* Set the desired baud rate */
    ascConf.baudrate.baudrate = UART_EdgeBoard_BAUDRATE;             /* Set the baud rate in bit/s               */
    ascConf.baudrate.oversampling = IfxAsclin_OversamplingFactor_16; /* Set the oversampling factor              */

    /* Configure the sampling mode */
    ascConf.bitTiming.medianFilter = IfxAsclin_SamplesPerBit_three;          /* Set the number of samples per bit        */
    ascConf.bitTiming.samplePointPosition = IfxAsclin_SamplePointPosition_8; /* Set the first sample position    */

    /* ISR priorities and interrupt target */
    ascConf.interrupt.txPriority = UART_EdgeBoard_ISR_PRIORITY_TX; /* Set the interrupt priority for TX events    */
    ascConf.interrupt.rxPriority = UART_EdgeBoard_ISR_PRIORITY_RX; /* Set the interrupt priority for RX events    */
    ascConf.interrupt.erPriority = UART_EdgeBoard_ISR_PRIORITY_ER; /* Set the interrupt priority for Error events */
    ascConf.interrupt.typeOfService = UART_EdgeBoard_INT_SERVICE;

    /* Pin configuration */
    const IfxAsclin_Asc_Pins pins = {
        .cts = NULL_PTR, /* CTS pin not used                                     */
        .ctsMode = IfxPort_InputMode_pullUp,
        .rx = &UART_EdgeBoard_RX_Pin,       /* Select the pin for RX connected to the USB port      */
        .rxMode = IfxPort_InputMode_pullUp, /* RX pin                                               */
        .rts = NULL_PTR,                    /* RTS pin not used                                     */
        .rtsMode = IfxPort_OutputMode_pushPull,
        .tx = &UART_EdgeBoard_TX_Pin,          /* Select the pin for TX connected to the USB port      */
        .txMode = IfxPort_OutputMode_pushPull, /* TX pin                                               */
        .pinDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1};
    ascConf.pins = &pins;

    /* FIFO buffers configuration */
    ascConf.txBuffer = UART_EdgeBoard_TxBuffer; /* Set the transmission buffer                          */
    ascConf.txBufferSize = ASC_TX_BUFFER_SIZE;  /* Set the transmission buffer size                     */
    ascConf.rxBuffer = UART_EdgeBoard_RxBuffer; /* Set the receiving buffer                             */
    ascConf.rxBufferSize = ASC_RX_BUFFER_SIZE;  /* Set the receiving buffer size                        */

    /* Init ASCLIN module */
    IfxAsclin_Asc_initModule(&UART_EdgeBoard_Asclin, &ascConf); /* Initialize the module with the given configuration */

    /* Initialize the Standard Interface */
    IfxAsclin_Asc_stdIfDPipeInit(&UART_EdgeBoard_StdIf, &UART_EdgeBoard_Asclin);
}

static void UART_EdgeBoard_Transmit(const char *message, short *count) {
    if (!UART_EdgeBoard_StdIf.txDisabled) {
        IfxStdIf_DPipe_write(&UART_EdgeBoard_StdIf, (void *)message, count, TIME_INFINITE);
    }
}

static void UART_EdgeBoard_Receive(const char *message, short *count) {
    IfxStdIf_DPipe_read(&UART_EdgeBoard_StdIf, (void *)message, count, TIME_NULL);
}

UART_TypeDef UART_EdgeBoard =
    {
        .Init = UART_EdgeBoard_Init,
        .Transmit = UART_EdgeBoard_Transmit,
        .Receive = UART_EdgeBoard_Receive,

        .FlagTransmitted = 0,
		.FlagReceived = 0};
