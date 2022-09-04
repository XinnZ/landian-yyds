/*
 * run.h
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#ifndef USERS_APP_RUN_H_
#define USERS_APP_RUN_H_

#include "../HeadFiles.h"

#define DEBUG_VOFA_ 	0


typedef struct
{
    volatile boolean Mode_Manual;  	// Enable Manual Control
    volatile boolean Mode_Upload;  	// Enable Upload to Upper

    volatile boolean Edge_Tran;  	// Transfer to the Edgeboard
    volatile uint16  Edge_HBP;    	// Edgeboard Heartbeat Packet

    volatile uint8 Buzzer;  		// Buzzer ringing time (n * 100ms)

    volatile float32 CtRoute;       // Current Real Route
    volatile float32 CtSpeed;       // Current Real Speed
    volatile sint16 CtEncoder_old;  // Current Encoder Value (Last)
    volatile sint16 CtEncoder_new;  // Current Encoder Value (Now)

    volatile sint32 CtMotorPwm;  	// Current Motor PWM Value
    volatile sint32 CtServoPwm;  	// Current Servo PWM Value

    volatile sint16 TargetSpeed;  	// Target Encoder Value
    volatile sint16 TargetAngle;  	// Target Servo Angle

    volatile uint8 Element;  		// Identity Elements
} Run_Params_List;

extern Run_Params_List Run_Params;


typedef struct
{
    void (*Init)(void);
    void (*Task_Auto)(void);
    void (*Task_Datas)(void);
    void (*Task_Manual)(void);
    void (*Task_Upper)(void);
    void (*Task_Rgb)(void);
} Run_TypeDef;

extern Run_TypeDef Run;


#endif /* USERS_APP_RUN_H_ */
