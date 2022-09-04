#include "_garage.hpp"

#include "../imgprocess.hpp"
#include "../utils/logger_helper.hpp"

#undef LOG_TAG
#define LOG_TAG "Garage"

flag_garage_e flag_garage = GARAGE_NONE;

// 引用图像处理
extern ImageProcess imgprocess;
#define IMG imgprocess

// 变量定义
float garage_route = 0;    // 车库编码器积分
bool garage_stop = false;  // 车库停车标志位
int garage_num = 0;        // 记录当前第几次车库
int none_left = 0, none_right = 0;
int have_left = 0, have_right = 0;

/* ********************************************************************* */

void check_garage() {
    // 检测斑马线
    ///> 1、设置一个栈, 用来存储黑色元素
    ///> 2、从此行的第0个开始往左扫
    ///> 3、如果遇到黑色元素, 入栈
    ///> 4、如果遇到白色元素, 出栈, 同时统计栈中元素个数, 如果栈中元素为4—8个
    ///>    (也就是黑胶的宽度), black_blocks++, 否则不加. 然后将栈中元素清掉
    ///> 5、如果此行的black_blocks在6个左右, times++
    ///> 6、遍历18——23行, 如果times在4个左右, 则确定为起跑线。
    if (flag_garage == GARAGE_NONE) {
        int16_t black_blocks = 0;
        int16_t cursor = 0;
        int16_t times = 0;

        for (int16_t y = 40; y < 100; y++) {
            black_blocks = 0;
            cursor = 0;  // 指向栈顶的游标
            for (int16_t x = IMAGE_WIDTH / 2 - 60; x < IMAGE_WIDTH / 2 + 60; x++) {
                if (AT_IMAGE(IMG.mat_bin, x, y) < IMG._config->threshold) {
                    if (cursor < 20)
                        cursor++;
                } else {
                    if (cursor >= 4 && cursor <= 8)
                        black_blocks++;
                    cursor = 0;
                }
            }
            if (black_blocks >= 5 && black_blocks <= 9)
                times++;
        }
        if (times >= 3) {
            flag_garage = GARAGE_DETECTION;
            MAT_LOG("GARAGE_DETECTION");
        }
    }

    /* ***************************************************************** */

    // 非出库, 找到斑马线
    if (flag_garage != GARAGE_OUT_LEFT && flag_garage != GARAGE_OUT_RIGHT &&
        flag_garage == GARAGE_DETECTION) {
        IMG.element_identify = 1;
        garage_route = IMG.encoder.route;

        if (IMG._config->direction_garage == 0) {  // 左库
            if (++garage_num >= 2) {
                flag_garage = GARAGE_IN_LEFT;  // 第二次入库
                MAT_LOG("GARAGE_IN_LEFT");
            } else {
                flag_garage = GARAGE_PASS_LEFT;  // 第一次不入库
                MAT_LOG("GARAGE_PASS_LEFT");
            }
        } else if (IMG._config->direction_garage == 1) {  // 右库
            if (++garage_num >= 2) {
                flag_garage = GARAGE_IN_RIGHT;  // 第二次入库
                MAT_LOG("GARAGE_IN_RIGHT");
            } else {
                flag_garage = GARAGE_PASS_RIGHT;  // 第一次不入库
                MAT_LOG("GARAGE_PASS_RIGHT");
            }
        } else if (IMG._config->direction_garage == 2) {  // 左库 循环
            flag_garage = GARAGE_PASS_LEFT;
            MAT_LOG("GARAGE_PASS_LEFT");
        } else if (IMG._config->direction_garage == 3) {  // 右库 循环
            flag_garage = GARAGE_PASS_RIGHT;
            MAT_LOG("GARAGE_PASS_RIGHT");
        }
    }
}

/* ********************************************************************* */

void run_garage() {
    switch (flag_garage) {
        case GARAGE_OUT_LEFT:
            IMG.flag_track = TRACK_LEFT;
            // 先丢线后有线
            if (IMG.rpts1s_num < 0.2 / SAMPLE_DIST)
                none_right++;
            if (IMG.rpts1s_num > 1.0 / SAMPLE_DIST && none_right > 1) {
                // 未识别到双 L 角点, 识别到双线, 出库完成
                if (!IMG.Lpt0_found && !IMG.Lpt1_found &&
                    IMG.rpts0s_num > 0.2 / SAMPLE_DIST) {
                    none_right = 0;
                    IMG.element_over = true;
                    IMG.element_over_route = IMG.encoder.route;
                    flag_garage = GARAGE_NONE;
                    MAT_LOG("GARAGE_NONE");
                }
            }
            break;
        case GARAGE_OUT_RIGHT:
            IMG.flag_track = TRACK_RIGHT;
            // 先丢线后有线
            if (IMG.rpts0s_num < 0.2 / SAMPLE_DIST)
                none_left++;
            if (IMG.rpts0s_num > 1.0 / SAMPLE_DIST && none_left > 1) {
                // 未识别到双 L 角点, 识别到双线, 出库完成
                if (!IMG.Lpt0_found && !IMG.Lpt1_found &&
                    IMG.rpts1s_num > 0.2 / SAMPLE_DIST) {
                    none_left = 0;
                    IMG.element_over = true;
                    IMG.element_over_route = IMG.encoder.route;
                    flag_garage = GARAGE_NONE;
                    MAT_LOG("GARAGE_NONE");
                }
            }
            break;

            /* ********************************************************* */

        case GARAGE_IN_LEFT:
            IMG.flag_track = TRACK_LEFT;
            // 先丢线后有线
            if (IMG.rpts1s_num < 0.2 / SAMPLE_DIST)
                none_right++;
            if ((IMG.rpts1s_num > 0.8 / SAMPLE_DIST && none_right > 1)) {
                if (IMG.Lpt0_found)
                    IMG.rptsc0_num = IMG.rpts0s_num = IMG.Lpt0_rpts0s_id;
                if (IMG.Lpt1_found)
                    IMG.rptsc1_num = IMG.rpts1s_num = IMG.Lpt1_rpts1s_id;
                if (have_right++ > 1) {
                    if ((IMG.Lpt0_found && IMG.Lpt0_rpts0s_id < 55) ||
                        (IMG.Lpt1_found && IMG.Lpt1_rpts1s_id < 55)) {
                        none_right = 0;
                        have_right = 0;
                        garage_stop = true;
                    }
                }
            }
            if (garage_stop || IMG.encoder.route - garage_route >= 125) {
                flag_garage = GARAGE_STOP;
                MAT_LOG("GARAGE_STOP");
            }
            break;
        case GARAGE_IN_RIGHT:
            IMG.flag_track = TRACK_RIGHT;
            // 先丢线后有线
            if (IMG.rpts0s_num < 0.2 / SAMPLE_DIST)
                none_left++;
            if ((IMG.rpts0s_num > 0.8 / SAMPLE_DIST && none_left > 1)) {
                if (IMG.Lpt0_found)
                    IMG.rptsc0_num = IMG.rpts0s_num = IMG.Lpt0_rpts0s_id;
                if (IMG.Lpt1_found)
                    IMG.rptsc1_num = IMG.rpts1s_num = IMG.Lpt1_rpts1s_id;
                if (have_left++ > 1) {
                    if ((IMG.Lpt0_found && IMG.Lpt0_rpts0s_id < 55) ||
                        (IMG.Lpt1_found && IMG.Lpt1_rpts1s_id < 55)) {
                        none_left = 0;
                        have_left = 0;
                        garage_stop = true;
                    }
                }
            }
            if (garage_stop || IMG.encoder.route - garage_route >= 125) {
                flag_garage = GARAGE_STOP;
                MAT_LOG("GARAGE_STOP");
            }
            break;

            /* ********************************************************* */

        case GARAGE_PASS_LEFT:
            IMG.flag_track = TRACK_RIGHT;
            if (IMG.encoder.route - garage_route > 110) {
                IMG.element_over = true;
                IMG.element_over_route = IMG.encoder.route;
                flag_garage = GARAGE_NONE;
                MAT_LOG("GARAGE_NONE");
            }
            break;
        case GARAGE_PASS_RIGHT:
            IMG.flag_track = TRACK_LEFT;
            if (IMG.encoder.route - garage_route > 110) {
                IMG.element_over = true;
                IMG.element_over_route = IMG.encoder.route;
                flag_garage = GARAGE_NONE;
                MAT_LOG("GARAGE_NONE");
            }
            break;

        default:
            break;
    }
}

/* ********************************************************************* */