#ifndef _YROAD_HPP_
#define _YROAD_HPP_

enum flag_yroad_e {
    YROAD_NONE = 0,
    YROAD_DETECTION,  // 检测到泛行区
    YROAD_FOUND,      // 找到泛行区
    YROAD_IN,         // 泛行区内部
    YROAD_OUT,        // 驶离泛行区
};
extern flag_yroad_e flag_yroad;

void check_yroad();
void run_yroad();

#endif
