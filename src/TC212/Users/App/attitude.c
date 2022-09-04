/*
 * attitude.c
 *
 *  Created on: 2022-05-04
 *      Author: Pomin & XinnZ, Landian, HBUT
 *  Pomin's Blog: https://www.pomin.top/
 *  XinnZ's Blog: https://blog.xinnz.cn/
 */

#include "attitude.h"

/* 忽略没有用到的变量和函数  */
#pragma warning 537
#pragma warning 536

/* 参数 */
#define delta_T 0.010f  // 10ms计算一次

float param_Kp = 50.0;  // 加速度计(磁力计)的收敛速率比例增益50
float param_Ki = 0.20;  // 陀螺仪收敛速率的积分增益 0.2

/* 重定向宏 */
#define icm_acc_x ICM.Accel.x
#define icm_acc_y ICM.Accel.y
#define icm_acc_z ICM.Accel.z

#define icm_gyro_x ICM.Gyro.x
#define icm_gyro_y ICM.Gyro.y
#define icm_gyro_z ICM.Gyro.z
#define g_q4 IMU.quater
#define g_angle IMU.angles

static float values[10];

IMU_TypeDef IMU;

/**
 * @brief  快速开方
 * @param  x:
 * @return float:
 */
static float invSqrt(float x) {
    float halfx = 0.5f * x;
    float y = x;
    long i = *(long *)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float *)&i;
    y = y * (1.5f - (halfx * y * y));
    return y;
}

/**
 * @brief  初始化参数
 */
static void imu_init(void) {
    // g_q4.q0 = 1;
    // g_q4.q1 = g_q4.q2 = g_q4.q3 = 0;
    // param_Kp = 50.0;
    // param_Ki = 0.20;
}

/**
 * @brief  数据转换 对accel一阶低通滤波(参考匿名)，对gyro转成弧度每秒(2000dps)
 */
#define new_weight 0.35f
#define old_weight 0.65f
static void IMU_getValues(void) {
    static double lastaccel[3] = {0, 0, 0};
    int i;
    values[0] = ((float)icm_acc_x) * new_weight + lastaccel[0] * old_weight;
    values[1] = ((float)icm_acc_y) * new_weight + lastaccel[1] * old_weight;
    values[2] = ((float)icm_acc_z) * new_weight + lastaccel[2] * old_weight;
    for (i = 0; i < 3; i++) {
        lastaccel[i] = values[i];
    }
    values[3] = ((float)icm_gyro_x) * M_PI / 180 / 16.4f;
    values[4] = ((float)icm_gyro_y) * M_PI / 180 / 16.4f;
    values[5] = ((float)icm_gyro_z) * M_PI / 180 / 16.4f;
}

/**
 * @brief  姿态解算融合，是Crazepony和核心算法，使用的是互补滤波算法，没有使用Kalman滤波算法
 */
static void imu_get_quater(void) {
    IMU_getValues();

    float gx = values[3];
    float gy = values[4];
    float gz = values[5];
    float ax = values[0];
    float ay = values[1];
    float az = values[2];
    static float halfT = 0.5 * delta_T;
    static float I_ex = 0.0f, I_ey = 0.0f, I_ez = 0.0f;  // 误差积分

    // 当前的机体坐标系上的重力单位向量
    float vx, vy, vz;
    // 四元数计算值与加速度计测量值的误差
    float ex, ey, ez;
    float q0 = g_q4.q0;
    float q1 = g_q4.q1;
    float q2 = g_q4.q2;
    float q3 = g_q4.q3;
    float q0q0 = q0 * q0;
    float q0q1 = q0 * q1;
    float q0q2 = q0 * q2;

    float q0q3 = q0 * q3;
    float q1q1 = q1 * q1;
    float q1q2 = q1 * q2;
    float q1q3 = q1 * q3;
    float q2q2 = q2 * q2;
    float q2q3 = q2 * q3;
    float q3q3 = q3 * q3;
    // float delta_2 = 0;

    // 对加速度数据进行归一化 得到单位加速度
    float norm = invSqrt(ax * ax + ay * ay + az * az);
    ax = ax * norm;
    ay = ay * norm;
    az = az * norm;
    vx = 2 * (q1q3 - q0q2);
    vy = 2 * (q0q1 + q2q3);
    vz = q0q0 - q1q1 - q2q2 + q3q3;

    ex = ay * vz - az * vy;
    ey = az * vx - ax * vz;
    ez = ax * vy - ay * vx;

    // 用叉乘误差来做PI修正陀螺零偏，
    // 通过调节 param_Kp，param_Ki 两个参数，
    // 可以控制加速度计修正陀螺仪积分姿态的速度。
    I_ex += delta_T * ex;  // integral error scaled by Ki
    I_ey += delta_T * ey;
    I_ez += delta_T * ez;

    gx = gx + param_Kp * ex + param_Ki * I_ex;
    gy = gy + param_Kp * ey + param_Ki * I_ey;
    gz = gz + param_Kp * ez + param_Ki * I_ez;

    /* 数据修正完成，下面是四元数微分方程 */

    // 四元数微分方程，其中halfT为测量周期的1/2，gx gy
    // gz为陀螺仪角速度，以下都是已知量，这里使用了一阶龙哥库塔求解四元数微分方程
    q0 = q0 + (-q1 * gx - q2 * gy - q3 * gz) * halfT;
    q1 = q1 + (q0 * gx + q2 * gz - q3 * gy) * halfT;
    q2 = q2 + (q0 * gy - q1 * gz + q3 * gx) * halfT;
    q3 = q3 + (q0 * gz + q1 * gy - q2 * gx) * halfT;
    // delta_2=(2*halfT*gx)*(2*halfT*gx)+(2*halfT*gy)*(2*halfT*gy)+(2*halfT*gz)*(2*halfT*gz);
    // 整合四元数率 四元数微分方程  四元数更新算法，二阶毕卡法
    // q0 = (1-delta_2/8)*q0 + (-q1*gx - q2*gy - q3*gz)*halfT;
    // q1 = (1-delta_2/8)*q1 + (q0*gx + q2*gz - q3*gy)*halfT;
    // q2 = (1-delta_2/8)*q2 + (q0*gy - q1*gz + q3*gx)*halfT;
    // q3 = (1-delta_2/8)*q3 + (q0*gz + q1*gy - q2*gx)*halfT;
    // normalise quaternion
    norm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    g_q4.q0 = q0 * norm;
    g_q4.q1 = q1 * norm;
    g_q4.q2 = q2 * norm;
    g_q4.q3 = q3 * norm;
}

/**
 * @brief  把四元数转换成欧拉角
 */
static void imu_get_angles(void) {
    imu_get_quater();

    float q0 = g_q4.q0;
    float q1 = g_q4.q1;
    float q2 = g_q4.q2;
    float q3 = g_q4.q3;
    g_angle.pitch = asin(-2 * q1 * q3 + 2 * q0 * q2) * 180 / M_PI;  // pitch
    g_angle.roll =
        atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * 180 /
        M_PI;  // roll
    g_angle.yaw =
        atan2(2 * q1 * q2 + 2 * q0 * q3, -2 * q2 * q2 - 2 * q3 * q3 + 1) * 180 /
        M_PI;  // yaw

    /* 可以不用作姿态限度的限制 */
#if (1 == IS_LIMIT_ANGLE)
    if (g_angle.roll > 90 || g_angle.roll < -90) {
        if (g_angle.pitch > 0) {
            g_angle.pitch = 180 - g_angle.pitch;
        }
        if (g_angle.pitch < 0) {
            g_angle.pitch = -(180 + g_angle.pitch);
        }
    }
    if (g_angle.yaw > 180) {
        g_angle.yaw -= 360;
    } else if (g_angle.yaw < -180) {
        g_angle.yaw += 360;
    }
#endif
}

/**
 * @brief  一阶滤波
 * @param  com: 采样的原始数值
 * @return float: 经过一阶滤波后的采样值
 */
static float lowV(float com) {
    static unsigned int iLastData;                      // 上一次值
    float iData;                                        // 本次计算值
    float dPower = 0.1;                                 // 滤波系数
    iData = (com * dPower) + (1 - dPower) * iLastData;  // 计算
    iLastData = iData;                                  // 存贮本次数据
    return iData;                                       // 返回数据
}

/**
 * @brief  刷新原始数据
 */
static void imu_get_data_ori(void) {
    ICM.Accel = ICM.GetAccel();
    ICM.Gyro = ICM.GetGyro();

    /* 可不滤波 */
    ICM.Gyro.x = lowV(ICM.Gyro.x);
    ICM.Gyro.y = lowV(ICM.Gyro.y);
    ICM.Gyro.z = lowV(ICM.Gyro.z);
}

/* 角度和四元数 只选择其中一个使用！！！ */
IMU_TypeDef IMU = {
    .Init = imu_init,
    .get_data = imu_get_data_ori,
    .get_quater = imu_get_quater,
    .get_angles = imu_get_angles,

    .quater = {1.0f, 0.0f, 0.0f, 0.0f},
    .angles = {0}};

#pragma warning default
