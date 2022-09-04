#ifndef _GARAGE_HPP_
#define _GARAGE_HPP_

enum flag_garage_e {
    GARAGE_NONE = 0,
    GARAGE_DETECTION,   // 检测到斑马线
    GARAGE_OUT_LEFT,    // 出库 左
    GARAGE_OUT_RIGHT,   // 出库 右
    GARAGE_IN_LEFT,     // 入库 左
    GARAGE_IN_RIGHT,    // 入库 右
    GARAGE_PASS_LEFT,   // 不进库 左
    GARAGE_PASS_RIGHT,  // 不进库 右
    GARAGE_STOP,        // 进库完毕, 停车
};
extern flag_garage_e flag_garage;

void check_garage();
void run_garage();

#endif
