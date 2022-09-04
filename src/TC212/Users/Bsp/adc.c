/*
 * adc.c
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include <adc.h>

static void Init(void) {
}

static void Read(uint16 *Data) {
}

ADC_TypeDef ADC =
    {
        .Init = Init,
        .Read = Read};
