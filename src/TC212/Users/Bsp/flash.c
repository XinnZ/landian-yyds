/*
 * flash.c
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include <flash.h>

#include "IfxFlash.h"
#include "IfxFlash_cfg.h"
#include "IfxScuWdt.h"
#include "Ifx_Types.h"

static void Write(FLASH_Sectors sector_num, uint32 page_num, uint32 *data) {
    uint16 endInitSafetyPassword = IfxScuWdt_getSafetyWatchdogPassword();
    uint32 sector_addr = IfxFlash_dFlashTableEepLog[sector_num].start;

    /* --------------- WRITE PROCESS --------------- */
    uint32 page_addr = sector_addr + page_num * IFXFLASH_DFLASH_PAGE_LENGTH; /* Get the address of the page      */

    /* Enter in page mode */
    IfxFlash_enterPageMode(page_addr);

    /* Wait until page mode is entered */
    IfxFlash_waitUnbusy(0, IfxFlash_FlashType_D0);

    /* Load data to be written in the page */
    IfxFlash_loadPage(page_addr, data[0], 0);

    /* Write the loaded page */
    IfxScuWdt_clearSafetyEndinit(endInitSafetyPassword); /* Disable EndInit protection                       */
    IfxFlash_writePage(page_addr);                       /* Write the page                                   */
    IfxScuWdt_setSafetyEndinit(endInitSafetyPassword);   /* Enable EndInit protection                        */

    /* Wait until the data is written in the Data Flash memory */
    IfxFlash_waitUnbusy(0, IfxFlash_FlashType_D0);
}

static uint32 Read(FLASH_Sectors sector_num, uint32 page_num) {
    /* float转为u32:  (*(uint32 *)&float_data) */
    /* 转为指定格式:     (*(type *)) */

    return (uint32)((IFXFLASH_DFLASH_START + (sector_num) * (IFXFLASH_DFLASH_SIZE / IFXFLASH_DFLASH_NUM_LOG_SECTORS)) + (page_num * 8));
}

static void Erase(FLASH_Sectors sector_num) {
    uint16 end_init_sfty_pw = IfxScuWdt_getSafetyWatchdogPassword();
    uint32 sector_addr = IfxFlash_dFlashTableEepLog[sector_num].start;

    IfxScuWdt_clearSafetyEndinit(end_init_sfty_pw);
    IfxFlash_eraseSector(sector_addr);
    IfxScuWdt_setSafetyEndinit(end_init_sfty_pw);

    IfxFlash_waitUnbusy(0, IfxFlash_FlashType_D0);
}

FLASH_TypeDef Flash =
    {
        .Write = Write,
        .Read = Read,
        .Erase = Erase};
