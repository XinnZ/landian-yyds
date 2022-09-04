/*
 * qspi.c
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include <qspi.h>

#include "IfxQspi.h"

static void Init(QSPIN_enum spi_n, QSPI_Moudle *spi_moudle, uint8 mode, float32 baudrate,
                 IfxQspi_Sclk_Out *CLK, IfxQspi_Mtsr_Out *MOSI, IfxQspi_Mrst_In *MISO, IfxQspi_Slso_Out *CS) {
    IfxQspi_SpiMaster_Config spiMasterConfig;               /* Define the Master Configuration          */
    IfxQspi_SpiMaster_ChannelConfig spiMasterChannelConfig; /* Define the Master Channel Configuration  */

    spi_moudle->spiMoudle = IfxQspi_getAddress((IfxQspi_Index)spi_n);

    /* Initialize it with default values */
    IfxQspi_SpiMaster_initModuleConfig(&spiMasterConfig, spi_moudle->spiMoudle);

    const IfxQspi_SpiMaster_Pins QSPIMasterpins = {
        CLK, IfxPort_OutputMode_pushPull,      /* SCLK Pin                          (CLK)  */
        MOSI, IfxPort_OutputMode_pushPull,     /* Master Transmit Slave Receive Pin (MOSI) */
        MISO, IfxPort_InputMode_pullDown,      /* Master Receive Slave Transmit Pin (MISO) */
        IfxPort_PadDriver_cmosAutomotiveSpeed3 /* Pad driver mode                          */
    };
    spiMasterConfig.pins = &QSPIMasterpins; /* Assign Master Pins                       */

    spiMasterConfig.base.mode = SpiIf_Mode_master;
    spiMasterConfig.base.maximumBaudrate = (float32)MAX_BAUDRATE;
    spiMasterConfig.base.isrProvider = IfxSrc_Tos_cpu0;

    /* Initialize the QSPI Master module using the user configuration */
    IfxQspi_SpiMaster_initModule(&spi_moudle->spiMaster, &spiMasterConfig);

    /* Initialize it with default values */
    IfxQspi_SpiMaster_initChannelConfig(&spiMasterChannelConfig, &spi_moudle->spiMaster);

    const IfxQspi_SpiMaster_Output QSPISlaveSelect = {
        CS, IfxPort_OutputMode_pushPull,       /* Slave Select Pin (CS)                    */
        IfxPort_PadDriver_cmosAutomotiveSpeed1 /* Pad driver mode                          */
    };
    spiMasterChannelConfig.sls.output = QSPISlaveSelect;
    spiMasterChannelConfig.base.baudrate = baudrate;

    spiMasterChannelConfig.base.mode.dataHeading = SpiIf_DataHeading_msbFirst;
    spiMasterChannelConfig.base.mode.dataWidth = 8;
    spiMasterChannelConfig.base.mode.csActiveLevel = Ifx_ActiveState_low;

    switch (mode) {
        case 0: {
            spiMasterChannelConfig.base.mode.clockPolarity = SpiIf_ClockPolarity_idleLow;                    // CPOL
            spiMasterChannelConfig.base.mode.shiftClock = SpiIf_ShiftClock_shiftTransmitDataOnTrailingEdge;  // CPHA
        } break;
        case 1: {
            spiMasterChannelConfig.base.mode.clockPolarity = SpiIf_ClockPolarity_idleLow;
            spiMasterChannelConfig.base.mode.shiftClock = SpiIf_ShiftClock_shiftTransmitDataOnLeadingEdge;
        } break;
        case 2: {
            spiMasterChannelConfig.base.mode.clockPolarity = SpiIf_ClockPolarity_idleHigh;
            spiMasterChannelConfig.base.mode.shiftClock = SpiIf_ShiftClock_shiftTransmitDataOnTrailingEdge;
        } break;
        case 3: {
            spiMasterChannelConfig.base.mode.clockPolarity = SpiIf_ClockPolarity_idleHigh;
            spiMasterChannelConfig.base.mode.shiftClock = SpiIf_ShiftClock_shiftTransmitDataOnLeadingEdge;
        } break;
    }

    /* Initialize the QSPI Master channel using the user configuration */
    IfxQspi_SpiMaster_initChannel(&spi_moudle->spiMasterChannel, &spiMasterChannelConfig);
}

static void MOSI(QSPI_Moudle *spi_moudle, uint8 *modata, uint8 *midata, uint32 len, uint8 continuous) {
    Ifx_QSPI_BACON bacon;

    bacon.U = spi_moudle->spiMoudle->BACON.U;

    bacon.B.DL = 7;
    bacon.B.IDLE = 1;
    bacon.B.IPRE = 1;
    bacon.B.LEAD = 1;
    bacon.B.LPRE = 1;
    bacon.B.MSB = 1;
    bacon.B.PARTYP = 0;
    bacon.B.BYTE = 0;
    bacon.B.TRAIL = 1;
    bacon.B.TPRE = 1;
    bacon.B.CS = (unsigned int)spi_moudle->spiMasterChannel.channelId;

    if (continuous)
        IfxQspi_writeBasicConfigurationBeginStream(spi_moudle->spiMoudle, bacon.U);  // 发送数据后CS继续保持为低
    else
        IfxQspi_writeBasicConfigurationEndStream(spi_moudle->spiMoudle, bacon.U);  // 每发送一个字节CS信号拉高一次

    if (len > 1) {
        uint32 i = 0;
        while (i < (len - 1)) {
            while (spi_moudle->spiMoudle->STATUS.B.TXFIFOLEVEL != 0)
                ;
            IfxQspi_write8(spi_moudle->spiMoudle, spi_moudle->spiMasterChannel.channelId, modata, 1);

            while (spi_moudle->spiMoudle->STATUS.B.RXFIFOLEVEL == 0)
                ;
            if (NULL != midata) {
                IfxQspi_read8(spi_moudle->spiMoudle, midata, 1);
                midata++;
            } else {
                (void)spi_moudle->spiMoudle->RXEXIT.U;
            }
            modata++;

            i++;
        }
    }

    // 发送最后一个数据
    if (continuous) {
        IfxQspi_writeBasicConfigurationEndStream(spi_moudle->spiMoudle, bacon.U);
    }

    IfxQspi_writeTransmitFifo(spi_moudle->spiMoudle, *modata);
    while (spi_moudle->spiMoudle->STATUS.B.TXFIFOLEVEL != 0)
        ;

    while (spi_moudle->spiMoudle->STATUS.B.RXFIFOLEVEL == 0)
        ;
    if (NULL != midata)
        IfxQspi_read8(spi_moudle->spiMoudle, midata, 1);
    else
        (void)spi_moudle->spiMoudle->RXEXIT.U;
}

QSPI_TypeDef QSPI =
    {
        .Init = Init,
        .MOSI = MOSI};
