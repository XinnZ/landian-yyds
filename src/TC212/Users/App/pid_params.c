/*
 * pid_params.c
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#include "flash.h"
#include "pid.h"

static void Init(void) {
    PID.Init(&PID_Params.Angle);
    PID.Init(&PID_Params.Omega);
    PID.Init(&PID_Params.Speed);
}

static void Save(void) {
    uint8 count = 0;
    uint32 write_buf = 0;

    Flash.Erase(flash_pid);

    write_buf = (*(uint32 *)&PID_Params.Angle.kP);
    Flash.Write(flash_pid, count++, &write_buf);
    write_buf = (*(uint32 *)&PID_Params.Angle.kI);
    Flash.Write(flash_pid, count++, &write_buf);
    write_buf = (*(uint32 *)&PID_Params.Angle.kD);
    Flash.Write(flash_pid, count++, &write_buf);

    write_buf = (*(uint32 *)&PID_Params.Speed.kP);
    Flash.Write(flash_pid, count++, &write_buf);
    write_buf = (*(uint32 *)&PID_Params.Speed.kI);
    Flash.Write(flash_pid, count++, &write_buf);
    write_buf = (*(uint32 *)&PID_Params.Speed.kD);
    Flash.Write(flash_pid, count++, &write_buf);

    write_buf = (*(uint32 *)&PID_Params.Omega.kP);
    Flash.Write(flash_pid, count++, &write_buf);
    write_buf = (*(uint32 *)&PID_Params.Omega.kI);
    Flash.Write(flash_pid, count++, &write_buf);
    write_buf = (*(uint32 *)&PID_Params.Omega.kD);
    Flash.Write(flash_pid, count++, &write_buf);
}

static void Read(void) {
    uint8 count = 0;

    PID_Params.Angle.kP = (*(float32 *)(Flash.Read(flash_pid, count++)));
    PID_Params.Angle.kI = (*(float32 *)(Flash.Read(flash_pid, count++)));
    PID_Params.Angle.kD = (*(float32 *)(Flash.Read(flash_pid, count++)));

    PID_Params.Speed.kP = (*(float32 *)(Flash.Read(flash_pid, count++)));
    PID_Params.Speed.kI = (*(float32 *)(Flash.Read(flash_pid, count++)));
    PID_Params.Speed.kD = (*(float32 *)(Flash.Read(flash_pid, count++)));

    PID_Params.Omega.kP = (*(float32 *)(Flash.Read(flash_pid, count++)));
    PID_Params.Omega.kI = (*(float32 *)(Flash.Read(flash_pid, count++)));
    PID_Params.Omega.kD = (*(float32 *)(Flash.Read(flash_pid, count++)));
}

PID_Params_List PID_Params =
    {
        .Init = Init,
        .Save = Save,
        .Read = Read,

        .Angle = {0},
        .Omega = {0},
        .Speed = {0}};
