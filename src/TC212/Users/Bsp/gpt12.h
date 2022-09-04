/*
 * gpt12.h
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#ifndef USERS_BSP_GPT12_H_
#define USERS_BSP_GPT12_H_

#include "IfxGpt12_PinMap.h"

typedef enum {
    GPT12_T2,
    GPT12_T3,
    GPT12_T4,
    GPT12_T5,
    GPT12_T6,
} GPTN_enum;

typedef struct
{
    void (*Init)(GPTN_enum gptn, IfxGpt12_TxIn_In *Gpt12_TxIn_Pin, IfxGpt12_TxEud_In *Gpt12_TxEud_Pin);
    short (*Get)(GPTN_enum gptn);
    void (*Clear)(GPTN_enum gptn);
} GPT12_TypeDef;

extern GPT12_TypeDef GPT12;

#endif /* USERS_BSP_GPT12_H_ */
