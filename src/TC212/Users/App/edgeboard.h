/*
 * edgeboard.h
 *
 *  Created on: 2022-04-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#ifndef USERS_APP_EDGEBOARD_H_
#define USERS_APP_EDGEBOARD_H_

typedef struct
{
    void (*Rece)(void);
    void (*Tran)(void);
} Edge_TypeDef;

extern Edge_TypeDef Edge;

#endif /* USERS_APP_EDGEBOARD_H_ */
