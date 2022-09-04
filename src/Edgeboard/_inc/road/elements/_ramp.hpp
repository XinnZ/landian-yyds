#ifndef _RAMP_HPP_
#define _RAMP_HPP_

enum flag_ramp_e {
    RAMP_NONE = 0,   // 非坡道模式
    RAMP_DETECTION,  // 检测到坡道
    RAMP_UP,         // 上坡阶段
    RAMP_DOWN,       // 下坡阶段
};
extern flag_ramp_e flag_ramp;

void check_ramp();
void run_ramp();

#endif
