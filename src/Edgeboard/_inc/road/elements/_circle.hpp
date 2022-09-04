#ifndef _CIRCLE_HPP_
#define _CIRCLE_HPP_

enum flag_circle_e {
    CIRCLE_NONE = 0,  // 非环岛模式
    CIRCLE_LEFT_BEGIN,
    CIRCLE_RIGHT_BEGIN,  // 环岛开始, 识别到单侧 L 角点另一侧长直道。
    CIRCLE_LEFT_IN,
    CIRCLE_RIGHT_IN,  // 环岛进入, 即走到一侧直道, 一侧环岛的位置。
    CIRCLE_LEFT_OUT,
    CIRCLE_RIGHT_OUT,  // 准备出环岛, 即识别到出环处的 L 角点。
    CIRCLE_LEFT_RUNNING,
    CIRCLE_RIGHT_RUNNING,  // 环岛内部。
};
extern flag_circle_e flag_circle;

void check_circle();
void run_circle();

#endif
