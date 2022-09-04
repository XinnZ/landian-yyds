/*
 * qspi.h
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#ifndef USERS_BSP_QSPI_H_
#define USERS_BSP_QSPI_H_

#include "IfxQspi_PinMap.h"
#include "IfxQspi_SpiMaster.h"
#include "Ifx_Types.h"

#define MAX_BAUDRATE 50000000 /* Master channel baud rate                         */

typedef enum {
    SPI_0 = 0,
    SPI_1,
    SPI_2
} QSPIN_enum;

typedef struct
{
    Ifx_QSPI *spiMoudle;                        /* QSPI Object                  */
    IfxQspi_SpiMaster spiMaster;                /* QSPI Master handle           */
    IfxQspi_SpiMaster_Channel spiMasterChannel; /* QSPI Master Channel handle   */
} QSPI_Moudle;

typedef struct
{
    void (*Init)(QSPIN_enum spi_n, QSPI_Moudle *spi_moudle, uint8 mode, float32 baudrate,
                 IfxQspi_Sclk_Out *CLK, IfxQspi_Mtsr_Out *MOSI, IfxQspi_Mrst_In *MISO, IfxQspi_Slso_Out *CS);
    void (*MOSI)(QSPI_Moudle *spi_moudle, uint8 *modata, uint8 *midata, uint32 len, uint8 continuous);
} QSPI_TypeDef;

extern QSPI_TypeDef QSPI;

#endif /* USERS_BSP_QSPI_H_ */
