#ifndef _WORK_HPP_
#define _WORK_HPP_

enum flag_work_e {
    WORK_NONE = 0,
    WORK_DETECTION,  // 检测到标志
    WORK_FOUND,      // 准备进入绕行区域
    WORK_READY,      // 正在进入绕行区域
    WORK_RUN,        // 施工区绕行中
    WORK_OUT,        // 驶离
};
extern flag_work_e flag_work;

void check_work();
void run_work();

#endif
