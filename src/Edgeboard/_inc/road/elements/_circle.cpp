#include "_circle.hpp"

#include "../imgprocess.hpp"
#include "../utils/logger_helper.hpp"

#undef LOG_TAG
#define LOG_TAG "Circle"

flag_circle_e flag_circle = CIRCLE_NONE;

// 引用图像处理
extern ImageProcess imgprocess;
#define IMG imgprocess

// 变量定义
float circle_route = 0;  // 环岛编码器积分
int none_left_line = 0, none_right_line = 0;
int have_left_line = 0, have_right_line = 0;

/* ********************************************************************* */

void check_circle() {
    // 非环岛, 单边 L 角点, 单边长直道
    if (flag_circle == CIRCLE_NONE &&
        (IMG.Lpt0_found && !IMG.Lpt1_found && IMG.is_straight1)) {
        none_left_line = 0;
        have_left_line = 0;
        IMG.element_identify = 1;
        circle_route = IMG.encoder.route;
        flag_circle = CIRCLE_LEFT_BEGIN;
        MAT_LOG("CIRCLE_LEFT_BEGIN");
    }

    if (flag_circle == CIRCLE_NONE &&
        (!IMG.Lpt0_found && IMG.Lpt1_found && IMG.is_straight0)) {
        none_right_line = 0;
        have_right_line = 0;
        IMG.element_identify = 1;
        circle_route = IMG.encoder.route;
        flag_circle = CIRCLE_RIGHT_BEGIN;
        MAT_LOG("CIRCLE_RIGHT_BEGIN");
    }
}

/* ********************************************************************* */

void run_circle() {
    // 左环开始, 寻外直道右线
    if (flag_circle == CIRCLE_LEFT_BEGIN) {
        IMG.flag_track = TRACK_RIGHT;

        // 先丢左线后有线
        if (IMG.rpts0s_num < 0.2 / SAMPLE_DIST)
            none_left_line++;
        if (IMG.rpts0s_num > 0.8 / SAMPLE_DIST && none_left_line > 1) {
            have_left_line++;
            if (have_left_line > 1) {
                none_left_line = 0;
                have_left_line = 0;
                circle_route = IMG.encoder.route;
                flag_circle = CIRCLE_LEFT_IN;
                MAT_LOG("CIRCLE_LEFT_IN");
            }
        }
        // 入环失败
        if (IMG.encoder.route - circle_route > 100 &&
            flag_circle != CIRCLE_LEFT_IN) {
            none_left_line = 0;
            have_left_line = 0;
            IMG.element_over = true;
            IMG.element_over_route = IMG.encoder.route;
            flag_circle = CIRCLE_NONE;
            MAT_LOG("CIRCLE_ERROR");
        }
    }
    // 入左环, 寻内圆左线
    if (flag_circle == CIRCLE_LEFT_IN) {
        IMG.flag_track = TRACK_LEFT;

        // 先丢右线后有线
        if (IMG.rpts1s_num < 0.2 / SAMPLE_DIST)
            none_right_line++;
        if (IMG.rpts1s_num > 0.8 / SAMPLE_DIST && none_right_line > 0) {
            if (IMG.encoder.route - circle_route > 60)
                have_right_line++;
            if (have_right_line > 0) {
                none_right_line = 0;
                have_right_line = 0;
                flag_circle = CIRCLE_LEFT_RUNNING;
                MAT_LOG("CIRCLE_LEFT_RUNNING");
            }
        }
    }
    // 正常巡线, 寻外圆右线
    if (flag_circle == CIRCLE_LEFT_RUNNING) {
        IMG.flag_track = TRACK_RIGHT;

        // 外环存在拐点, 可再加拐点距离判据 (左L点)
        if (IMG.Lpt1_found)
            IMG.ipts1_num = IMG.rpts1s_num = IMG.rptsc1_num = IMG.Lpt1_rpts1s_id;
        if (IMG.Lpt1_found && IMG.Lpt1_rpts1s_id < 0.6 / SAMPLE_DIST) {
            flag_circle = CIRCLE_LEFT_OUT;
            MAT_LOG("CIRCLE_LEFT_OUT");
        }
    }
    // 出环, 寻内圆
    if (flag_circle == CIRCLE_LEFT_OUT) {
        IMG.flag_track = TRACK_LEFT;

        if (IMG.Lpt1_found)
            IMG.ipts1_num = IMG.rpts1s_num = IMG.rptsc1_num = IMG.Lpt1_rpts1s_id;

        // 先丢右线后有线
        if (IMG.rpts1s_num < 0.2 / SAMPLE_DIST)
            none_right_line++;
        if (IMG.rpts1s_num > 1.0 / SAMPLE_DIST && none_right_line > 0) {
            if (have_right_line > 0) {
                none_right_line = 0;
                have_right_line = 0;
                IMG.element_over = true;
                IMG.element_over_route = IMG.encoder.route;
                flag_circle = CIRCLE_NONE;
                MAT_LOG("CIRCLE_NONE");
            } else {
                have_right_line++;
            }
        }
    }

    /* ***************************************************************** */

    // 右环开始, 寻外直道左线
    if (flag_circle == CIRCLE_RIGHT_BEGIN) {
        IMG.flag_track = TRACK_LEFT;

        // 先丢右线后有线
        if (IMG.rpts1s_num < 0.2 / SAMPLE_DIST)
            none_right_line++;
        if (IMG.rpts1s_num > 0.8 / SAMPLE_DIST && none_right_line > 1) {
            have_right_line++;
            if (have_right_line > 1) {
                none_right_line = 0;
                have_right_line = 0;
                circle_route = IMG.encoder.route;
                flag_circle = CIRCLE_RIGHT_IN;
                MAT_LOG("CIRCLE_RIGHT_IN");
            }
        }
        // 入环失败
        if (IMG.encoder.route - circle_route > 100 &&
            flag_circle != CIRCLE_RIGHT_IN) {
            none_right_line = 0;
            have_right_line = 0;
            IMG.element_over = true;
            IMG.element_over_route = IMG.encoder.route;
            flag_circle = CIRCLE_NONE;
            MAT_LOG("CIRCLE_ERROR");
        }
    }
    // 入右环, 寻内圆右线
    if (flag_circle == CIRCLE_RIGHT_IN) {
        IMG.flag_track = TRACK_RIGHT;

        // 先丢左线后有线
        if (IMG.rpts0s_num < 0.2 / SAMPLE_DIST)
            none_left_line++;
        if (IMG.rpts0s_num > 0.8 / SAMPLE_DIST && none_left_line > 0) {
            if (IMG.encoder.route - circle_route > 60)
                have_left_line++;
            if (have_left_line > 0) {
                none_left_line = 0;
                have_left_line = 0;
                flag_circle = CIRCLE_RIGHT_RUNNING;
                MAT_LOG("CIRCLE_RIGHT_RUNNING");
            }
        }
    }
    // 正常巡线, 寻外圆左线
    if (flag_circle == CIRCLE_RIGHT_RUNNING) {
        IMG.flag_track = TRACK_LEFT;

        // 外环存在拐点, 可再加拐点距离判据 (左L点)
        if (IMG.Lpt0_found)
            IMG.ipts0_num = IMG.rpts0s_num = IMG.rptsc0_num = IMG.Lpt0_rpts0s_id;
        if (IMG.Lpt0_found && IMG.Lpt0_rpts0s_id < 0.6 / SAMPLE_DIST) {
            flag_circle = CIRCLE_RIGHT_OUT;
            MAT_LOG("CIRCLE_RIGHT_OUT");
        }
    }
    // 出环, 寻内圆
    if (flag_circle == CIRCLE_RIGHT_OUT) {
        IMG.flag_track = TRACK_RIGHT;

        if (IMG.Lpt0_found)
            IMG.ipts0_num = IMG.rpts0s_num = IMG.rptsc0_num = IMG.Lpt0_rpts0s_id;

        // 先丢左线后有线
        if (IMG.rpts0s_num < 0.2 / SAMPLE_DIST)
            none_left_line++;
        if (IMG.rpts0s_num > 1.0 / SAMPLE_DIST && none_left_line > 0) {
            if (have_left_line > 0) {
                none_left_line = 0;
                have_left_line = 0;
                IMG.element_over = true;
                IMG.element_over_route = IMG.encoder.route;
                flag_circle = CIRCLE_NONE;
                MAT_LOG("CIRCLE_NONE");
            } else {
                have_left_line++;
            }
        }
    }
}

/* ********************************************************************* */