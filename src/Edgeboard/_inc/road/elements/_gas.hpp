#ifndef _GAS_HPP_
#define _GAS_HPP_

enum flag_gas_e {
    GAS_NONE = 0,
    GAS_DETECTION,
    GAS_FOUND,  // 准备进入加油站区域
    GAS_READY,  // 正在进入加油站区域
    GAS_RUN,    // 加油站绕行中
    GAS_OUT,    // 驶离
};
extern flag_gas_e flag_gas;

void check_gas();
void run_gas();

#endif
