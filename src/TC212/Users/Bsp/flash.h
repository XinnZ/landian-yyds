/*
 * flash.h
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#ifndef USERS_BSP_FLASH_H_
#define USERS_BSP_FLASH_H_

#include "Ifx_Types.h"

typedef enum {
    flash_test = 0,
    flash_work,
    flash_pid
} FLASH_Sectors;

typedef struct
{
    void (*Write)(FLASH_Sectors sector_num, uint32 page_num, uint32 *data);
    uint32 (*Read)(FLASH_Sectors sector_num, uint32 page_num);
    void (*Erase)(FLASH_Sectors sector_num);
} FLASH_TypeDef;

extern FLASH_TypeDef Flash;

#endif /* USERS_BSP_FLASH_H_ */
