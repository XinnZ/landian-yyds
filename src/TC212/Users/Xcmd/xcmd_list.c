/*
 * xcmd_list.c
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include <xcmd_list.h>
#include <HeadFiles.h>

static int cmd_motor(int argc, char *argv[]) {
    if (xcmd_param_check(2, argc)) {
        if (strcmp(argv[1], "-d") == 0) {
            Motor.Speed((sint32)(atof(argv[2]) * 100));
            xcmd_print("motor duty: %.2f%%\r\n", atof(argv[2]));
        } else {
            goto motor_tip;
        }
    } else {
    motor_tip:

        xcmd_print(TX_YELLOW "motor -d duty(%%)\r\n");
    }

    return 0;
}

static int cmd_servo(int argc, char *argv[]) {
    if (xcmd_param_check(3, argc)) {
        if (strcmp(argv[1], "-m") == 0 && strcmp(argv[2], "-w") == 0) {
            Servo.MedianWrite((uint32)atoi(argv[3]));
            xcmd_print("servo median: %d\r\n", Servo.median);
        } else {
            goto servo_tip;
        }
    } else if (xcmd_param_check(2, argc)) {
        if (strcmp(argv[1], "-m") == 0) {
            if (strcmp(argv[2], "-r") == 0) {
                Servo.MedianRead();
                xcmd_print("servo median: %d\r\n", Servo.median);
            } else {
                goto servo_tip;
            }
        } else if (strcmp(argv[1], "-d") == 0) {
            Servo.Duty((uint32)(atof(argv[2]) * 100));
            xcmd_print("servo duty: %.2f%%\r\n", atof(argv[2]));
        } else if (strcmp(argv[1], "-a") == 0) {
            uint8 dutyDir = 0;
            uint16 dutyValue = SERVO_MIN;

            Servo.Duty(Servo.median);
            xcmd_print("servo auto-rotation.\r\n");

            UART_Console.FlagReceived = 0;
            while (!UART_Console.FlagReceived) {
                if (dutyValue == SERVO_MIN) {
                    dutyDir = 0;
                }
                if (dutyValue == SERVO_MAX) {
                    dutyDir = 1;
                }

                if (dutyDir == 0) {
                    dutyValue++;
                }
                if (dutyDir == 1) {
                    dutyValue--;
                }

                Servo.Duty((uint32)(dutyValue));
                delay_ms(atoi(argv[2]));
            }
            xcmd_print("exit.\r\n");
        } else {
            goto servo_tip;
        }
    } else {
    servo_tip:

        xcmd_print(TX_YELLOW "servo -m -r|-w [data]\r\n" TX_DEF);
        xcmd_print(TX_YELLOW "servo -d duty(%%)\r\n" TX_DEF);
        xcmd_print(TX_YELLOW "servo -a time(ms)\r\n" TX_DEF);
    }

    return 0;
}

static int cmd_run(int argc, char *argv[]) {
    if (xcmd_param_check(2, argc)) {
        if (strcmp(argv[1], "-s") == 0) {
            Run_Params.TargetSpeed = (sint16)atoi(argv[2]);
            xcmd_print("target speed: %d\r\n", Run_Params.TargetSpeed);
        } else if (strcmp(argv[1], "-r") == 0) {
            Run_Params.TargetAngle = (sint16)atoi(argv[2]);
            xcmd_print("target angle: %d\r\n", Run_Params.TargetAngle);
        } else {
            goto run_tip;
        }
    } else if (xcmd_param_check(1, argc)) {
        if (strcmp(argv[1], "--auto") == 0) {
            xcmd_print("Auto Run module is enabled.\r\n");

            PID.Clear(&PID_Params.Angle);
            PID.Clear(&PID_Params.Omega);
            PID.Clear(&PID_Params.Speed);
            Run_Params.CtEncoder_new = 0;
            Run_Params.TargetSpeed = 0;
            Run_Params.TargetAngle = 0;

            CCU6.Start(CCU6_0, PIT_CH0);  // Open Auto
            CCU6.Start(CCU6_0, PIT_CH1);  // Open Data

            Run_Params.Mode_Manual = false;
            CCU6.Stop(CCU6_1, PIT_CH0);  // Stop Manual
        } else if (strcmp(argv[1], "--manual") == 0) {
            xcmd_print("Manual Run module is enabled.\r\n");

            Run_Params.Mode_Manual = true;
            CCU6.Start(CCU6_1, PIT_CH0);  // Open Manual
            CCU6.Start(CCU6_0, PIT_CH1);  // Open Data
            CCU6.Stop(CCU6_0, PIT_CH0);   // Stop Auto
            Motor.Speed(0);

            while (Run_Params.Mode_Manual)
                ;

            Run_Params.Mode_Manual = false;
            CCU6.Stop(CCU6_1, PIT_CH0);  // Stop Manual
            CCU6.Stop(CCU6_0, PIT_CH1);  // Stop Data
            Motor.Speed(0);

            xcmd_print("exit.\r\n");
        } else if (strcmp(argv[1], "--upload") == 0) {
            xcmd_print("Upload module is enabled.\r\n");
            Run_Params.Mode_Upload = true;

            CCU6.Start(CCU6_1, PIT_CH1);
        } else if (strcmp(argv[1], "--stop") == 0) {
            xcmd_print("Run module is disabled.\r\n");
            Run_Params.Mode_Manual = false;
            Run_Params.Mode_Upload = false;
            Run_Params.Edge_Tran = false;

            CCU6.Stop(CCU6_0, PIT_CH0);  // Stop Auto
            CCU6.Stop(CCU6_0, PIT_CH1);  // Stop Data
            CCU6.Stop(CCU6_1, PIT_CH0);  // Stop Manual
            CCU6.Stop(CCU6_1, PIT_CH1);  // Stop Upload

            Motor.Speed(0);
            Servo.Duty(Servo.median);
        } else {
            goto run_tip;
        }
    } else {
    run_tip:

        xcmd_print(TX_YELLOW "run --auto|--stop|--manual|--upload\r\n");
        xcmd_print(TX_YELLOW "run -s|-r value\r\n");
    }

    return 0;
}

static int cmd_imu(int argc, char *argv[]) {
    UART_Console.FlagReceived = 0;
    while (!UART_Console.FlagReceived) {
        xcmd_print("imu: %5.2f, %5.2f, %5.2f\r\n", IMU.angles.yaw, IMU.angles.pitch, IMU.angles.roll);
        delay_ms(10);
    }
    xcmd_print("exit.\r\n");
    return 0;
}

static int cmd_icm(int argc, char *argv[]) {
    if (xcmd_param_check(1, argc)) {
        if (strcmp(argv[1], "-a") == 0) {
            UART_Console.FlagReceived = 0;
            while (!UART_Console.FlagReceived) {
                xcmd_print("icm: %5.2f, %5.2f, %5.2f\r\n", ICM.Accel.x, ICM.Accel.y, ICM.Accel.z);
                delay_ms(10);
            }
            xcmd_print("exit.\r\n");
        } else if (strcmp(argv[1], "-g") == 0) {
            UART_Console.FlagReceived = 0;
            while (!UART_Console.FlagReceived) {
                xcmd_print("icm: %5.2f, %5.2f, %5.2f\r\n", ICM.Gyro.x, ICM.Gyro.y, ICM.Gyro.z);
                delay_ms(10);
            }
            xcmd_print("exit.\r\n");
        } else if (strcmp(argv[1], "-p") == 0) {
            UART_Console.FlagReceived = 0;
            while (!UART_Console.FlagReceived) {
                xcmd_print("icm: %5.2f, %5.2f, %5.2f\r\n", ICM.Gyro_PU.x, ICM.Gyro_PU.y, ICM.Gyro_PU.z);
                delay_ms(10);
            }
            xcmd_print("exit.\r\n");
        } else {
            goto icm_tip;
        }
    } else {
    icm_tip:

        xcmd_print(TX_YELLOW "icm -a|-g|-p\r\n");
    }

    return 0;
}

static int cmd_encoder(int argc, char *argv[]) {
    short encoder_value_new = 0;
    short encoder_value_old = 0;

    UART_Console.FlagReceived = 0;
    while (!UART_Console.FlagReceived) {
        encoder_value_new = -GPT12.Get(GPT12_T) * 10;

        xcmd_print("encoder: %d\r\n", encoder_value_new - encoder_value_old);

        encoder_value_old = encoder_value_new;

        delay_ms(10);
    }
    xcmd_print("exit.\r\n");

    return 0;
}

static int cmd_pid(int argc, char *argv[]) {
    if (argc == 2) {
        if (strcmp(argv[1], "--read") == 0) {
            PID_Params.Read();
            xcmd_print("pid parameters has been updated.\r\n");
        } else if (strcmp(argv[1], "--save") == 0) {
            PID_Params.Save();
            xcmd_print("pid parameters has been rewritten.\r\n");
        } else {
            goto pid_tip;
        }
    } else if (argc == 4) {
        PID_Param_TypeDef *PidParam;

        if (strcmp(argv[1], "-s") == 0) {
            PidParam = &PID_Params.Speed;
        } else if (strcmp(argv[1], "-a") == 0) {
            PidParam = &PID_Params.Angle;
        } else if (strcmp(argv[1], "-o") == 0) {
            PidParam = &PID_Params.Omega;
        } else {
            goto pid_tip;
        }

        if (strcmp(argv[2], "-p") == 0) {
            PidParam->kP = atof(argv[3]);
        } else if (strcmp(argv[2], "-i") == 0) {
            PidParam->kI = atof(argv[3]);
        } else if (strcmp(argv[2], "-d") == 0) {
            PidParam->kD = atof(argv[3]);
        } else {
            goto pid_tip;
        }

        PID.Clear(PidParam);
        xcmd_print("pid %s -p %.2f -i %.2f -d %.2f\r\n", argv[1], PidParam->kP, PidParam->kI, PidParam->kD);
    } else if (argc == 6) {
        PID_Param_TypeDef *PidParam;

        if (strcmp(argv[2], "-f") != 0) {
            goto pid_tip;
        }

        PID_Params.Read();

        if (strcmp(argv[1], "-s") == 0) {
            PidParam = &PID_Params.Speed;
        } else if (strcmp(argv[1], "-a") == 0) {
            PidParam = &PID_Params.Angle;
        } else if (strcmp(argv[1], "-o") == 0) {
            PidParam = &PID_Params.Omega;
        } else {
            goto pid_tip;
        }

        PidParam->kP = atof(argv[3]);
        PidParam->kI = atof(argv[4]);
        PidParam->kD = atof(argv[5]);

        PID_Params.Save();
        PID.Clear(PidParam);
        xcmd_print("pid %s -p %.2f -i %.2f -d %.2f\r\n", argv[1], PidParam->kP, PidParam->kI, PidParam->kD);
    } else {
    pid_tip:

        xcmd_print(TX_YELLOW "pid --read|--save\r\n");
        xcmd_print(TX_YELLOW "pid -a|-o|-s -p|-i|-d float\r\n" TX_DEF);
        xcmd_print(TX_YELLOW "pid -a|-o|-s -f pF iF dF\r\n\r\n" TX_DEF);

        xcmd_print("Angle -p %.2f, -i %.2f, -d %.2f.\r\n", PID_Params.Angle.kP, PID_Params.Angle.kI, PID_Params.Angle.kD);
        xcmd_print("Omega -p %.2f, -i %.2f, -d %.2f.\r\n", PID_Params.Omega.kP, PID_Params.Omega.kI, PID_Params.Omega.kD);
        xcmd_print("Speed -p %.2f, -i %.2f, -d %.2f.\r\n", PID_Params.Speed.kP, PID_Params.Speed.kI, PID_Params.Speed.kD);
    }

    return 0;
}

static int cmd_ccu6(int argc, char *argv[]) {
    if (xcmd_param_check(3, argc)) {
        if (strcmp(argv[1], "--start") == 0) {
            CCU6.Start(atoi(argv[2]), atoi(argv[3]));
            xcmd_print("ccu6_%d_ch%d: start\r\n", atoi(argv[2]), atoi(argv[3]));
        } else if (strcmp(argv[1], "--stop") == 0) {
            CCU6.Stop(atoi(argv[2]), atoi(argv[3]));
            xcmd_print("ccu6_%d_ch%d: stop\r\n", atoi(argv[2]), atoi(argv[3]));
        } else {
            goto ccu6_tip;
        }
    } else {
    ccu6_tip:

        xcmd_print(TX_YELLOW "ccu6 --start|--stop ccu6N chN\r\n");
    }

    return 0;
}

static int cmd_flash(int argc, char *argv[]) {
    if (xcmd_param_check(4, argc)) {
        if (strcmp(argv[1], "-w") == 0) {
            uint32 data;
            if (strcmp(argv[2], "-f") == 0) {
                float dataf = atof(argv[4]);
                data = (*(uint32 *)&dataf);
                xcmd_print("flash %d write: %f\r\n", atoi(argv[3]), dataf);
            } else {
                data = atoi(argv[4]);
                xcmd_print("flash %d write: %u\r\n", atoi(argv[3]), data);
            }

            Flash.Erase(flash_test);
            Flash.Write(flash_test, atoi(argv[3]), &data);
        } else {
            goto flash_tip;
        }
    } else if (xcmd_param_check(3, argc)) {
        if (strcmp(argv[1], "-r") == 0) {
            if (strcmp(argv[2], "-f") == 0) {
                xcmd_print("flash %d read: %f\r\n", atoi(argv[3]), (*(float *)(Flash.Read(flash_test, atoi(argv[3])))));
            } else {
                xcmd_print("flash %d read: %d\r\n", atoi(argv[3]), (*(int *)(Flash.Read(flash_test, atoi(argv[3])))));
            }
        } else {
            goto flash_tip;
        }
    } else {
    flash_tip:

        xcmd_print(TX_YELLOW "flash -r|-w -f|-d page [data]\r\n");
    }

    return 0;
}

static int cmd_led(int argc, char *argv[]) {
    if (xcmd_param_check(2, argc)) {
        if (strcmp(argv[1], "-h") == 0) {
            switch (atoi(argv[2])) {
                case 0:
                    GPIO.High(IFXCFG_PORT_LED2);
                    break;
                case 1:
                    GPIO.High(IFXCFG_PORT_LED3);
                    break;
                case 2:
                    GPIO.High(IFXCFG_PORT_LED4);
                    break;
                case 3:
                    GPIO.High(IFXCFG_PORT_LED5);
                    break;
            }
        } else if (strcmp(argv[1], "-l") == 0) {
            switch (atoi(argv[2])) {
                case 0:
                    GPIO.Low(IFXCFG_PORT_LED2);
                    break;
                case 1:
                    GPIO.Low(IFXCFG_PORT_LED3);
                    break;
                case 2:
                    GPIO.Low(IFXCFG_PORT_LED4);
                    break;
                case 3:
                    GPIO.Low(IFXCFG_PORT_LED5);
                    break;
            }
        } else if (strcmp(argv[1], "-t") == 0) {
            switch (atoi(argv[2])) {
                case 0:
                    GPIO.Toggle(IFXCFG_PORT_LED2);
                    break;
                case 1:
                    GPIO.Toggle(IFXCFG_PORT_LED3);
                    break;
                case 2:
                    GPIO.Toggle(IFXCFG_PORT_LED4);
                    break;
                case 3:
                    GPIO.Toggle(IFXCFG_PORT_LED5);
                    break;
            }
        } else {
            goto led_tip;
        }
    } else {
    led_tip:

        xcmd_print(TX_YELLOW "led -h|-l|-t pin\r\n");
    }

    return 0;
}

#define CHECK_RANGE(x) (((x) > 255 || (x) < 0) ? 0 : x)
static int cmd_rgb(int argc, char *argv[]) {
    int index;
    uint8 r, g, b;
    if (xcmd_param_check(4, argc)) {
        index = atoi(argv[1]);
        r = CHECK_RANGE(atoi(argv[2]));
        g = CHECK_RANGE(atoi(argv[3]));
        b = CHECK_RANGE(atoi(argv[4]));
        if (index < 4) {
            rgb_led[index].Points(rgb_led[index].io, r, g, b, rgb_led[index].num);
        } else {
            for (size_t i = 0; i < 4; i++) {
                rgb_led[i].Points(rgb_led[i].io, r, g, b, rgb_led[i].num);
            }
        }
        xcmd_print("rgb_led %d %d %d %d.\r\n", index, r, g, b);
    } else {
        xcmd_print(TX_YELLOW "rgb_led index red green blue\r\n");
    }
    return 0;
}

static xcmd_t cmds[] =
    {
        {"ccu6", 	cmd_ccu6, 		"ccu6 timer control.", 			NULL},
        {"flash", 	cmd_flash, 		"flash read or write.", 		NULL},
        {"imu", 	cmd_imu, 		"imu data display.", 			NULL},
        {"icm", 	cmd_icm, 		"icm data display.", 			NULL},
        {"run", 	cmd_run, 		"run switch control.", 			NULL},
        {"led", 	cmd_led, 		"setting led gpio status.", 	NULL},
        {"pid", 	cmd_pid, 		"setting pid parameters.", 		NULL},
        {"motor", 	cmd_motor, 		"setting motor parameters.", 	NULL},
        {"servo", 	cmd_servo, 		"setting servo parameters.",	NULL},
        {"encoder", cmd_encoder, 	"check encoder values.", 		NULL},
        {"rgb", 	cmd_rgb, 		"set rgb color.", 				NULL},
};

void extern_cmds_init(void) {
    xcmd_cmd_register(cmds, sizeof(cmds) / sizeof(xcmd_t));
}
