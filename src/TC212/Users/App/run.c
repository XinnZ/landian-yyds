/*
 * run.c
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include "run.h"

static void Task_Auto(void) {
    /* Encoder */
    short encoder_value = -GPT12.Get(GPT12_T) * 10;
    Run_Params.CtEncoder_new = encoder_value - Run_Params.CtEncoder_old;
    Run_Params.CtEncoder_old = encoder_value;

    /* 1 pulse = (¦ÐD / Line) * (Master gear / slave gear) / time (m/s) */
    /* 1 pulse = (¦Ð*0.0625 / 5000) * (14 / 38) / 0.01 (m/s) */
    Run_Params.CtSpeed = Run_Params.CtEncoder_new * 0.001446786f;
    /* s = v * t = x(m/s) * 0.01(s) = m => cm * 100 */
    Run_Params.CtRoute += Run_Params.CtSpeed;

    /* Motor Control */
    Run_Params.CtMotorPwm += PID.Increase(&PID_Params.Speed, Run_Params.CtEncoder_new, Run_Params.TargetSpeed);
    Run_Params.CtMotorPwm = PID.Limiter(Run_Params.CtMotorPwm, -6000, 6000);
    Motor.Speed(Run_Params.CtMotorPwm);
}

static void Task_Datas(void) {
    static int freq_100 = 0;

    /* Edgeboard Heartbeat Pack 100 * 10ms = 1.0s */
    if (Run_Params.Edge_Tran) {
        if (Run_Params.Edge_HBP > 100) {
            Run_Params.CtRoute = 0;
            Run_Params.TargetSpeed = 0;
            Run_Params.TargetAngle = 0;
            Run_Params.Edge_Tran = false;
            Servo.Duty(Servo.median);
        } else {
            Run_Params.Edge_HBP++;

            /* Transfer to the Edgeboard */
            Edge.Tran();
        }
    }

    /* Buzzer */
    if (++freq_100 >= 10) {  // 100ms
        freq_100 = 0;
        if (Run_Params.Buzzer > 0) {
            Run_Params.Buzzer--;
            GPIO.High(IFXCFG_PORT_BUZZ);
        } else {
            GPIO.Low(IFXCFG_PORT_BUZZ);
        }
    }
}

static void Task_Manual(void) {
    char i = 0;
    short count = 0;
    short readCount = 64;   // Receive length
    char receiveDatas[64];  // Receive buffer

    static uint8 data[8] = {0};  // Frame data
    static uint8 index = 0;      // Frame index
    static uint16 timer = 0;     // Timer to prevent control failure

    sint16 speedPwm, anglePwm;

    Run_Params.CtEncoder_new = 0;
    Run_Params.CtEncoder_old = 0;

    UART_Console.Receive(receiveDatas, &readCount);
    count += readCount;

    for (i = 0; i < count; i++) {
        data[index++] = (uint8)receiveDatas[i];

        if (0xF1 != data[0]) {
            index = 0;
            return;
        }
        if (0xE1 != data[1] && index >= 2) {
            index = 0;
            return;
        }
        if ((8 - 1) < index) {
            if (0xF2 == data[(8 - 1)]) {
                speedPwm = (sint16)(((sint16)data[2] << 8) | data[3]);
                anglePwm = (sint16)(((sint16)data[4] << 8) | data[5]) + (sint16)Servo.median;

                Run_Params.CtMotorPwm = (sint32)speedPwm;
                Run_Params.CtServoPwm = (sint32)anglePwm;

                Motor.Speed(Run_Params.CtMotorPwm);
                Servo.Duty(Run_Params.CtServoPwm);

                if (0x01 == data[6]) {
                    memset(data, 0, sizeof(uint8) * 8);
                    Run_Params.Mode_Manual = false;
                    break;
                }
                timer = 0;  // Clear timer
            }
            index = 0;
        }
    }

    if (timer++ > 50) {  // 50 * 20ms = 1s
        Motor.Speed(0);
        Servo.Duty(Servo.median);
    }
}

static void Task_Upper(void) {
#if (DEBUG_VOFA_ == 1)

    short length = 12;
    uint8 sendData[12];
    float tempFloat[2];

    tempFloat[0] = (float)Run_Params.CtEncoder_new;
    tempFloat[1] = (float)Run_Params.TargetSpeed;

    memcpy(sendData, (uint8_t *)tempFloat, sizeof(tempFloat));
    sendData[8] = 0x00;
    sendData[9] = 0x00;
    sendData[10] = 0x80;
    sendData[11] = 0x7f;
    UART_Console.Transmit((const char *)sendData, &length);

#else

    short length = 3;
    uint8 sendData[16] = {0x2A, 0xF1, 0xE2, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF2};

    sendData[length++] = (uint8)(Run_Params.CtEncoder_new >> 8);
    sendData[length++] = (uint8)(Run_Params.CtEncoder_new >> 0);

    sendData[length++] = (uint8)((sint16)Run_Params.CtMotorPwm >> 8);
    sendData[length++] = (uint8)((sint16)Run_Params.CtMotorPwm >> 0);

    sendData[length++] = (uint8)((sint16)Run_Params.CtServoPwm >> 8);
    sendData[length++] = (uint8)((sint16)Run_Params.CtServoPwm >> 0);

    sendData[length++] = (uint8)(Run_Params.TargetSpeed >> 8);
    sendData[length++] = (uint8)(Run_Params.TargetSpeed >> 0);

    sendData[length++] = (uint8)(Run_Params.TargetAngle >> 8);
    sendData[length++] = (uint8)(Run_Params.TargetAngle >> 0);

    uint16 CRC16 = CRC_CHECK(&sendData[3], 10);
    sendData[length++] = (uint8)(CRC16 >> 8);
    sendData[length++] = (uint8)(CRC16 & 0xff);
    length++;

    UART_Console.Transmit((const char *)sendData, &length);

#endif
}

static void Init(void) {
    PID_Params.Init();
    PID_Params.Read();

    memset(&Run_Params, 0, sizeof(Run_Params_List));

    CCU6.Init(CCU6_0, PIT_CH0, 10 * 1000);
    CCU6.Start(CCU6_0, PIT_CH0);
    CCU6.Init(CCU6_0, PIT_CH1, 10 * 1000);
    CCU6.Start(CCU6_0, PIT_CH1);
    CCU6.Init(CCU6_1, PIT_CH0, 20 * 1000);
    CCU6.Stop(CCU6_1, PIT_CH0);
    CCU6.Init(CCU6_1, PIT_CH1, 20 * 1000);
    CCU6.Stop(CCU6_1, PIT_CH1);
}

Run_Params_List Run_Params =
    {
        0};

Run_TypeDef Run =
    {
        .Init = Init,
        .Task_Auto = Task_Auto,
        .Task_Datas = Task_Datas,
        .Task_Manual = Task_Manual,
        .Task_Upper = Task_Upper,
        .Task_Rgb = Task_Rgb};
