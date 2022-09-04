/*
 * icm20602.c
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include "icm20602.h"

#include "gpio.h"
#include "qspi.h"

QSPI_Moudle QSPI_ICM;
static uint16 ICM_InitOK;

static void icm_spi_w_reg_byte(uint8 cmd, uint8 val) {
    uint8 dat[2];

    dat[0] = cmd | ICM20602_SPI_W;
    dat[1] = val;

    QSPI.MOSI(&QSPI_ICM, dat, dat, 2, 1);
}

static void icm_spi_r_reg_byte(uint8 cmd, uint8 *val) {
    uint8 dat[2];

    dat[0] = cmd | ICM20602_SPI_R;
    dat[1] = *val;

    QSPI.MOSI(&QSPI_ICM, dat, dat, 2, 1);

    *val = dat[1];
}

static void icm_spi_r_reg_bytes(uint8 *val, uint8 num) {
    QSPI.MOSI(&QSPI_ICM, val, val, num, 1);
}

static void icm20602_self_check(void) {
    uint8 dat = 0;
    uint8 time = 0;

    delay_ms(10);
    icm_spi_r_reg_byte(ICM20602_WHO_AM_I, &dat);

    while (0x12 != dat)  // ICM20602 ID
    {
        GPIO.Toggle(IFXCFG_PORT_LED2);
        GPIO.Toggle(IFXCFG_PORT_BUZZ);

        if (time++ > 60) {
            GPIO.Low(IFXCFG_PORT_LED2);
            GPIO.Low(IFXCFG_PORT_BUZZ);
            break;
        }

        delay_ms(50);
        icm_spi_r_reg_byte(ICM20602_WHO_AM_I, &dat);
        // 卡在这里原因有以下几点
        // 1 ICM20602坏了，如果是新的这样的概率极低
        // 2 接线错误或者没有接好
        // 3 可能你需要外接上拉电阻，上拉到3.3V
    }
    if (time < 60) {
        ICM_InitOK = true;
    }
}

static void icm20602_init(void) {
    uint8 val = 0x0;
    uint8 time = 0;

    delay_ms(10);

    QSPI.Init(SPI_NUM, &QSPI_ICM, 3, 10 * 1000 * 1000,
              &SPI_SCK_PIN, &SPI_MOSI_PIN, &SPI_MISO_PIN, &SPI_CS_PIN);

    icm20602_self_check();                          // 检测
    icm_spi_w_reg_byte(ICM20602_PWR_MGMT_1, 0x80);  // 复位
    delay_ms(2);

    do  // 等待复位成功
    {
        icm_spi_r_reg_byte(ICM20602_PWR_MGMT_1, &val);
        delay_ms(10);
        if (time++ > 100)
            break;
    } while (0x41 != val);

    icm_spi_w_reg_byte(ICM20602_PWR_MGMT_1, 0x01);      // 时钟设置
    icm_spi_w_reg_byte(ICM20602_PWR_MGMT_2, 0x00);      // 开启陀螺仪和加速度计
    icm_spi_w_reg_byte(ICM20602_CONFIG, 0x02);          // 92HZ 1KHZ
    icm_spi_w_reg_byte(ICM20602_SMPLRT_DIV, 0x07);      // 采样速率 SAMPLE_RATE = INTERNAL_SAMPLE_RATE / (1 + SMPLRT_DIV)
    icm_spi_w_reg_byte(ICM20602_GYRO_CONFIG, 0x18);     // ±2000 dps
    icm_spi_w_reg_byte(ICM20602_ACCEL_CONFIG, 0x10);    // ±8g
    icm_spi_w_reg_byte(ICM20602_ACCEL_CONFIG_2, 0x03);  // Average 4 samples   44.8HZ   //0x23 Average 16 samples

    // ICM20602_GYRO_CONFIG 寄存器
    // 设置为:0x00 陀螺仪量程为:±250 dps     获取到的陀螺仪数据除以131           可以转化为带物理单位的数据，单位为：°/s
    // 设置为:0x08 陀螺仪量程为:±500 dps     获取到的陀螺仪数据除以65.5          可以转化为带物理单位的数据，单位为：°/s
    // 设置为:0x10 陀螺仪量程为:±1000dps     获取到的陀螺仪数据除以32.8          可以转化为带物理单位的数据，单位为：°/s
    // 设置为:0x18 陀螺仪量程为:±2000dps     获取到的陀螺仪数据除以16.4          可以转化为带物理单位的数据，单位为：°/s

    // ICM20602_ACCEL_CONFIG 寄存器
    // 设置为:0x00 加速度计量程为:±2g          获取到的加速度计数据 除以16384      可以转化为带物理单位的数据，单位：g(m/s^2)
    // 设置为:0x08 加速度计量程为:±4g          获取到的加速度计数据 除以8192       可以转化为带物理单位的数据，单位：g(m/s^2)
    // 设置为:0x10 加速度计量程为:±8g          获取到的加速度计数据 除以4096       可以转化为带物理单位的数据，单位：g(m/s^2)
    // 设置为:0x18 加速度计量程为:±16g         获取到的加速度计数据 除以2048       可以转化为带物理单位的数据，单位：g(m/s^2)
}

static Axis3d get_icm20602_accdata(void) {
    Axis3d datas;

    struct
    {
        uint8 reg;
        uint8 dat[6];
    } buf;

    if (!ICM_InitOK)
        return (Axis3d){
            0,
            0,
            0,
        };

    buf.reg = ICM20602_ACCEL_XOUT_H | ICM20602_SPI_R;

    icm_spi_r_reg_bytes(&buf.reg, 7);

    datas.x = (sint16)((((uint16)buf.dat[0] << 8) | buf.dat[1]));
    datas.y = (sint16)((((uint16)buf.dat[2] << 8) | buf.dat[3]));
    datas.z = (sint16)((((uint16)buf.dat[4] << 8) | buf.dat[5]));

    return datas;
}

static Axis3d get_icm20602_gyro(void) {
    Axis3d datas;

    struct
    {
        uint8 reg;
        uint8 dat[6];
    } buf;

    if (!ICM_InitOK)
        return (Axis3d){
            0,
            0,
            0,
        };

    buf.reg = ICM20602_GYRO_XOUT_H | ICM20602_SPI_R;

    icm_spi_r_reg_bytes(&buf.reg, 7);

    datas.x = (sint16)((((uint16)buf.dat[0] << 8) | buf.dat[1]));
    datas.y = (sint16)((((uint16)buf.dat[2] << 8) | buf.dat[3]));
    datas.z = (sint16)((((uint16)buf.dat[4] << 8) | buf.dat[5]));

    return datas;
}

static float fffff(float com) {
    static float iLastData = 0.0f;
    float iData;

    iData = (com * 0.35f) + ((1 - 0.35f) * iLastData);
    iLastData = iData;

    return iData;
}

static void get_physical_unit(void) {
    /*
    ICM.Accel = ICM.GetAccel();
    ICM.Accel_PU.x = (float)ICM.Accel.x / 4096.0f;
    ICM.Accel_PU.y = (float)ICM.Accel.y / 4096.0f;
    ICM.Accel_PU.z = (float)ICM.Accel.z / 4096.0f;
     */

    ICM.Gyro = ICM.GetGyro();
    // ICM.Gyro_PU.x = (float)ICM.Gyro.x * M_PI / 180 / 16.4f;
    // ICM.Gyro_PU.y = (float)ICM.Gyro.y * M_PI / 180 / 16.4f;
    ICM.Gyro_PU.z = fffff(((float)ICM.Gyro.z * M_PI / 180.0f / 16.4f));
}

ICM_TypeDef ICM =
    {
        .Init = icm20602_init,

        .GetAccel = get_icm20602_accdata,
        .GetGyro = get_icm20602_gyro,

        .GetPhysicalUnit = get_physical_unit,

        .Accel = {0},
        .Gyro = {0},

        .Accel_PU = {0},
        .Gyro_PU = {0}};
