#ifndef _CROSS_HPP_
#define _CROSS_HPP_

enum flag_cross_e {
    CROSS_NONE = 0,  // 非十字模式
    CROSS_BEGIN,     // 找到左右两个 L 角点
    CROSS_IN,        // 两个 L 角点很近, 即进入十字内部(此时切换远线控制)
};
extern flag_cross_e flag_cross;

void check_cross();
void run_cross();
void cross_farline();

#endif
