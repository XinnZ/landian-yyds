#ifndef _EXTERN_HPP_
#define _EXTERN_HPP_

#include <cstdint>

#include "_define.hpp"

// 目标检测线程
extern std::shared_ptr<Detection> detection;

// 加油站
extern uint8_t gas_which_last;  // 加油站出站口号 上一次
extern uint8_t gas_which;       // 加油站出站口
extern bool gas_which_is;       // 加油站出站口检测标志

// 泛行区
extern float yroad_rptss[IMAGE_HEIGHT][2];
extern int yroad_rptss_num, yroad_Lpt_id;
extern bool yroad_Lpt_found;

// 十字
extern float far_rpts0s[IMAGE_HEIGHT][2];
extern float far_rpts1s[IMAGE_HEIGHT][2];
extern int far_rpts0s_num, far_rpts1s_num;
extern int far_Lpt0_rpts0s_id, far_Lpt1_rpts1s_id;
extern bool far_Lpt0_found, far_Lpt1_found;

#endif
