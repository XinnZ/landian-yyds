/*
 * rgb.h
 *
 *  Created on: 2022-08-19
 *      Author: Pomin & XinnZ, Landian, HBUT
 *  Pomin's Blog: https://www.pomin.top/
 *  XinnZ's Blog: https://blog.xinnz.cn/
 */

#ifndef _FS_RGB_
#define _FS_RGB_

#include "../HeadFiles.h"


#define RGB_FRONT rgb_led[1]
#define RGB_BACK  rgb_led[2]
#define RGB_LEFT  rgb_led[0]
#define RGB_RIGHT rgb_led[3]

#define RGB_SET_FRONT(r, g, b) RGB_FRONT.Points(RGB_FRONT.io, r, g, b, RGB_FRONT.num)
#define RGB_SET_BACK(r, g, b)  RGB_BACK.Points(RGB_BACK.io , r, g, b, RGB_BACK.num)
#define RGB_SET_LEFT(r, g, b)  RGB_LEFT.Points(RGB_LEFT.io , r, g, b, RGB_LEFT.num)
#define RGB_SET_RIGHT(r, g, b) RGB_RIGHT.Points(RGB_RIGHT.io, r, g, b, RGB_RIGHT.num)


/* hsv */
typedef struct chsv {
    uint16 h;
    uint16 s;
    uint16 v;
} chsv;

/* rgb */
typedef struct crgb {
    uint8 r;
    uint8 g;
    uint8 b;
} crgb;


typedef struct
{
        IfxPort_Pin io;
        uint8 num;
        void (* Point)(IfxPort_Pin io, uint8 red, uint8 green, uint8 blue);
        void (* Points)(IfxPort_Pin io, uint8 red, uint8 green, uint8 blue, uint8 num);
} Rgb_TypeDef;

extern Rgb_TypeDef rgb_led[4];


void Task_Rgb(void);


#endif
