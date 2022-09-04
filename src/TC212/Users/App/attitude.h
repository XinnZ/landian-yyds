/*
 * attitude.h
 *
 *  Created on: 2022-05-04
 *      Author: Pomin & XinnZ, Landian, HBUT
 *  Pomin's Blog: https://www.pomin.top/
 *  XinnZ's Blog: https://blog.xinnz.cn/
 */

#ifndef _ATTITURE_H
#define _ATTITURE_H

#include "icm20602.h"

/* 是否限制角度 */
#define IS_LIMIT_ANGLE 0

/* 四元数 */
typedef struct {
    float q0;
    float q1;
    float q2;
    float q3;
} quater_t;

/* 欧拉角 */
typedef struct {
    float pitch;
    float roll;
    float yaw;
} angles_t;

/* IMU 类 */
typedef struct {
    void (*Init)(void);
    void (*get_data)(void);
    void (*get_quater)(void);
    void (*get_angles)(void);
    quater_t quater;
    angles_t angles;
} IMU_TypeDef;

extern IMU_TypeDef IMU;

#endif  // _ATTITURE_H
