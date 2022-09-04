/*
 * rgb.c
 *
 *  Created on: 2022-08-19
 *      Author: Pomin & XinnZ, Landian, HBUT
 *  Pomin's Blog: https://www.pomin.top/
 *  XinnZ's Blog: https://blog.xinnz.cn/
 */

#include "rgb.h"

void delay375ns(void) {
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
}

void delay825ns(void) {
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
}

void rgbBit1(IfxPort_Pin io) {
    io.port->OMR.U = IfxPort_State_high << io.pinIndex;
    delay825ns();
    io.port->OMR.U = IfxPort_State_low << io.pinIndex;
    delay375ns();
}

void rgbBit0(IfxPort_Pin io) {
    io.port->OMR.U = IfxPort_State_high << io.pinIndex;
    delay375ns();
    io.port->OMR.U = IfxPort_State_low << io.pinIndex;
    delay825ns();
}

/**
 * @brief 点亮一个 rgb 灯珠
 *
 * @param red   红色值
 * @param green 绿色值
 * @param blue  蓝色值
**/
void rgbWriteOne(IfxPort_Pin io, uint8 red, uint8 green, uint8 blue) {
    uint8 i = 0;
    uint32 rgb = (uint32)green;
    rgb <<= 8;
    rgb |= (uint32)red;
    rgb <<= 8;
    rgb |= (uint32)blue;
    rgb <<= 8;
    for (i = 0; i < 24; i++) {
        if (rgb & 0x80000000) {
            rgbBit1(io);
        } else {
            rgbBit0(io);
        }
        rgb <<= 1;
    }
}

/**
 * @brief 连续点亮 rgb 灯
 *
 * @param red   红色值
 * @param green 绿色值
 * @param blue  蓝色值
**/
void rgbWriteSome(IfxPort_Pin io, uint8 red, uint8 green, uint8 blue, uint8 num) {
    uint8 i;
    for (i = 0; i < num; i++) {
        rgbWriteOne(io, red, green, blue);
    }
}

Rgb_TypeDef rgb_led[4] = {
    {
        .num = 3,
        .io = {&MODULE_P20,11},
        .Point = rgbWriteOne,
        .Points = rgbWriteSome
    },
    {
        .num = 4,
        .io = {&MODULE_P20,12},
        .Point = rgbWriteOne,
        .Points = rgbWriteSome
    },
    {
        .num = 4,
        .io = {&MODULE_P20,13},
        .Point = rgbWriteOne,
        .Points = rgbWriteSome
    },
    {
        .num = 3,
        .io = {&MODULE_P20,14},
        .Point = rgbWriteOne,
        .Points = rgbWriteSome
    },
};

//////////////////////////////////////////////////////////////////////////

/**
 * @brief hsv 转 rgb
 *
 * @param rgb
 * @param hsv
**/
void HSVtoRGB(crgb* rgb, chsv* hsv) {
    // R,G,B from 0-255, H from 0-360, S,V from 0-100

    uint16_t rgb_max = (uint16_t)(hsv->v * 2.55f);
    uint16_t rgb_min = (uint16_t)(rgb_max * (100 - hsv->s) / 100.0f);

    int i = hsv->h / 60;
    int difs = hsv->h % 60; // factorial part of h

    // rgb adjustment amount by hue
    uint16_t rgb_Adj = (rgb_max - rgb_min) * difs / 60.0f;

    switch (i) {
        case 0:
            rgb->r = (uint8)rgb_max;
            rgb->g = (uint8)(rgb_min + rgb_Adj);
            rgb->b = (uint8)rgb_min;
            break;
        case 1:
            rgb->r = (uint8)(rgb_max - rgb_Adj);
            rgb->g = (uint8)rgb_max;
            rgb->b = (uint8)rgb_min;
            break;
        case 2:
            rgb->r = (uint8)rgb_min;
            rgb->g = (uint8)rgb_max;
            rgb->b = (uint8)(rgb_min + rgb_Adj);
            break;
        case 3:
            rgb->r = (uint8)rgb_min;
            rgb->g = (uint8)(rgb_max - rgb_Adj);
            rgb->b = (uint8)rgb_max;
            break;
        case 4:
            rgb->r = (uint8)(rgb_min + rgb_Adj);
            rgb->g = (uint8)rgb_min;
            rgb->b = (uint8)rgb_max;
            break;
        default:  // case 5:
            rgb->r = (uint8)rgb_max;
            rgb->g = (uint8)rgb_min;
            rgb->b = (uint8)(rgb_max - rgb_Adj);
            break;
    }
}

/**
 * @brief 流光溢彩特效
 *
**/
crgb* rgb_brilliant(void) {
    static crgb rgb = { 0 };
    static int step = 1;
    static chsv hsv = {
        .h = 0,     // 0 ~ 360
        .s = 100,   // 0 ~ 100
        .v = 100    // 0 ~ 100
    };
    hsv.h += step;
    if (hsv.h == 358) {
        step = -1;
    } else if (hsv.h == 0) {
        step = 1;
    }
    HSVtoRGB(&rgb, &hsv);
    return &rgb;
}

/**
 * @brief 10ms tick 提供
 *
**/
void Task_Rgb(void) {
    crgb * rgb = rgb_brilliant();

    /* 运行状态 */
    if(Run_Params.Edge_Tran) {
        uint8_t rgb_b_speed = 255 - (uint8_t)((fabs(Run_Params.CtSpeed) / 2.5f > 1.0f ? 1.0f : fabs(Run_Params.CtSpeed) / 2.5f) * 255);

        RGB_FRONT.Points(RGB_FRONT.io, 255, 255, 255, RGB_FRONT.num);       // 车头
        RGB_BACK.Points(RGB_BACK.io, 255, 0, rgb_b_speed, RGB_BACK.num);    // 车尾

        RGB_LEFT.Points(RGB_LEFT.io, rgb->r, rgb->g, rgb->b, RGB_LEFT.num);     // 左
        RGB_RIGHT.Points(RGB_RIGHT.io, rgb->r, rgb->g, rgb->b, RGB_RIGHT.num);  // 右
    /* 停车状态 */
    } else {
        for (size_t i = 0; i < 4; i++)
        {
            rgb_led[i].Points(rgb_led[i].io, rgb->r, rgb->g, rgb->b, rgb_led[i].num);
        }
    }
}

