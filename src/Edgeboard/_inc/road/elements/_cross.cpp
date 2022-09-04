#include "_cross.hpp"

#include "../imgprocess.hpp"
#include "../utils/logger_helper.hpp"

#undef LOG_TAG
#define LOG_TAG "Cross"

flag_cross_e flag_cross = CROSS_NONE;

// 引用图像处理
extern ImageProcess imgprocess;
#define IMG imgprocess

// 变量定义
float cross_route = 0;  // 十字编码器积分
int not_have_line = 0;  // 十字丢线计次

// 十字远线 L 角点
bool far_Lpt0_found, far_Lpt1_found;
int far_Lpt0_rpts0s_id, far_Lpt1_rpts1s_id;

// 十字远线
int far_ipts0[IMAGE_HEIGHT][2];
int far_ipts1[IMAGE_HEIGHT][2];
int far_ipts0_num, far_ipts1_num;

float far_rpts0[IMAGE_HEIGHT][2];
float far_rpts1[IMAGE_HEIGHT][2];

float far_rpts0b[IMAGE_HEIGHT][2];
float far_rpts1b[IMAGE_HEIGHT][2];

float far_rpts0s[IMAGE_HEIGHT][2];
float far_rpts1s[IMAGE_HEIGHT][2];
int far_rpts0s_num, far_rpts1s_num;

float far_rpts0a[IMAGE_HEIGHT];
float far_rpts1a[IMAGE_HEIGHT];

float far_rpts0an[IMAGE_HEIGHT];
float far_rpts1an[IMAGE_HEIGHT];

/* ********************************************************************* */

void check_cross() {
    if (flag_cross == CROSS_NONE &&
        ((IMG.Lpt0_found && IMG.Lpt1_found) ||
         (IMG.Lpt0_found && IMG.rpts1s_num < 0.25 / SAMPLE_DIST) ||
         (IMG.Lpt1_found && IMG.rpts0s_num < 0.25 / SAMPLE_DIST))) {
        IMG.element_identify = 1;
        flag_cross = CROSS_BEGIN;
        MAT_LOG("CROSS_BEGIN");
    }
}

/* ********************************************************************* */

void run_cross() {
    // 检测到十字, 先按照近线走
    if (flag_cross == CROSS_BEGIN) {
        if (IMG.Lpt0_found) {
            IMG.ipts0_num = IMG.rpts0s_num = IMG.rptsc0_num = IMG.Lpt0_rpts0s_id;
        }
        if (IMG.Lpt1_found) {
            IMG.ipts1_num = IMG.rpts1s_num = IMG.rptsc1_num = IMG.Lpt1_rpts1s_id;
        }

        // 近角点过少, 进入远线控制
        if ((!IMG.Lpt0_found && !IMG.Lpt0_found) ||
            (IMG.Lpt0_found && IMG.Lpt0_rpts0s_id < 0.10 / SAMPLE_DIST) ||
            (IMG.Lpt1_found && IMG.Lpt1_rpts1s_id < 0.10 / SAMPLE_DIST) ||
            (IMG.rpts0s_num < 0.10 / SAMPLE_DIST && IMG.rpts1s_num < 0.10 / SAMPLE_DIST)) {
            cross_route = IMG.encoder.route;
            flag_cross = CROSS_IN;
            MAT_LOG("CROSS_IN");
        }
    }

    /* ***************************************************************** */

    // 远线控制进十字
    if (flag_cross == CROSS_IN) {
        // 寻远线, 算法与近线相同
        cross_farline();

        if (far_Lpt0_found && far_Lpt1_found && far_rpts0s[far_rpts0s_num - 1][0] > IMAGE_WIDTH / 2)
            IMG.flag_track = TRACK_LEFT;
        else if (far_Lpt0_found && far_Lpt1_found && far_rpts1s[far_rpts1s_num - 1][0] < IMAGE_WIDTH / 2)
            IMG.flag_track = TRACK_RIGHT;
        else if (far_rpts0s_num > far_rpts1s_num)
            IMG.flag_track = TRACK_LEFT;
        else if (far_rpts0s_num < far_rpts1s_num)
            IMG.flag_track = TRACK_RIGHT;
        else
            IMG.flag_track = TRACK_LEFT;

        // 正常巡线先丢线后有线
        if (IMG.rpts0s_num < 0.1 / SAMPLE_DIST &&
            IMG.rpts1s_num < 0.1 / SAMPLE_DIST) {
            not_have_line++;
        }
        if ((IMG.rpts0s_num > 1.0 / SAMPLE_DIST &&
             IMG.rpts1s_num > 1.0 / SAMPLE_DIST &&
             !IMG.Lpt0_found && !IMG.Lpt1_found &&
             not_have_line > 1) ||
            (IMG.encoder.route - cross_route >= 55)) {
            not_have_line = 0;
            IMG.element_over = true;
            IMG.element_over_route = IMG.encoder.route;
            flag_cross = CROSS_NONE;
            MAT_LOG("CROSS_NONE");
        }
    }
}

/* ********************************************************************* */

void cross_farline() {
    int far_x1 = 50;
    int far_x2 = IMAGE_WIDTH - 50;

    int far_y1 = 0;
    int far_y2 = 0;

    int yy1 = BEGIN_Y;
    int yy2 = BEGIN_Y;

    if (IMG.Lpt0_found && IMG.encoder.route - cross_route < 25) {
        float trans[2];
        map_perspective(IMG.rpts0s[IMG.Lpt0_rpts0s_id][0],
                        IMG.rpts0s[IMG.Lpt0_rpts0s_id][1], trans, 1);
        yy1 = trans[1] - 5;
        far_x1 = trans[0];
    }
    if (IMG.Lpt1_found && IMG.encoder.route - cross_route < 25) {
        float trans[2];
        map_perspective(IMG.rpts1s[IMG.Lpt1_rpts1s_id][0],
                        IMG.rpts1s[IMG.Lpt1_rpts1s_id][1], trans, 1);
        yy2 = trans[1] - 5;
        far_x2 = trans[0];
    }

    /* ***************************************************************** */

    bool white_found = false;
    bool black_found = false;
    far_ipts0_num = sizeof(far_ipts0) / sizeof(far_ipts0[0]);
    for (; yy1 >= 60; yy1--) {
        if (AT_IMAGE(IMG.mat_bin, far_x1, yy1) >= IMG._config->threshold)
            white_found = true;

        if (AT_IMAGE(IMG.mat_bin, far_x1, yy1) < IMG._config->threshold &&
            white_found) {
            int black = 0;
            for (int i = 1; i <= 30; i++)
                if (yy1 - i >= 0)
                    black += AT_IMAGE(IMG.mat_bin, far_x1, yy1 - i);
            if (black > 200) {
                white_found = false;
                continue;
            }
            black_found = true;
            far_y1 = yy1;
            break;
        }
    }
    if (AT_IMAGE(IMG.mat_bin, far_x1, far_y1 + 1) >= IMG._config->threshold &&
        black_found)
        findline_lefthand_adaptive(IMG.mat_bin, BLOCK_SIZE, CLIP_VALUE, far_x1,
                                   far_y1 + 1, far_ipts0, &far_ipts0_num);
    else
        far_ipts0_num = 0;

    /* ***************************************************************** */

    white_found = false;
    black_found = false;
    far_ipts1_num = sizeof(far_ipts1) / sizeof(far_ipts1[0]);
    for (; yy2 >= 60; yy2--) {
        if (AT_IMAGE(IMG.mat_bin, far_x2, yy2) >= IMG._config->threshold)
            white_found = true;

        if (AT_IMAGE(IMG.mat_bin, far_x2, yy2) < IMG._config->threshold &&
            white_found) {
            int black = 0;
            for (int i = 1; i <= 30; i++)
                if (yy2 - i >= 0)
                    black += AT_IMAGE(IMG.mat_bin, far_x2, yy2 - i);
            if (black > 200) {
                white_found = false;
                continue;
            }
            black_found = true;
            far_y2 = yy2;
            break;
        }
    }
    if (AT_IMAGE(IMG.mat_bin, far_x2, far_y2 + 1) >= IMG._config->threshold &&
        black_found)
        findline_righthand_adaptive(IMG.mat_bin, BLOCK_SIZE, CLIP_VALUE, far_x2,
                                    far_y2 + 1, far_ipts1, &far_ipts1_num);
    else
        far_ipts1_num = 0;

    /* ***************************************************************** */

    // 原图边线 -> 透视边线
    for (int i = 0; i < far_ipts0_num; i++)
        map_perspective(far_ipts0[i][0], far_ipts0[i][1], far_rpts0[i], 0);
    for (int i = 0; i < far_ipts1_num; i++)
        map_perspective(far_ipts1[i][0], far_ipts1[i][1], far_rpts1[i], 0);

    // 边线滤波
    blur_points(far_rpts0, far_ipts0_num, far_rpts0b,
                (int)round(LINE_BLUR_KERNEL));
    blur_points(far_rpts1, far_ipts1_num, far_rpts1b,
                (int)round(LINE_BLUR_KERNEL));

    // 边线等距采样
    far_rpts0s_num = sizeof(far_rpts0s) / sizeof(far_rpts0s[0]);
    resample_points(far_rpts0b, far_ipts0_num, far_rpts0s, &far_rpts0s_num,
                    SAMPLE_DIST * PIXEL_PER_METER);
    far_rpts1s_num = sizeof(far_rpts1s) / sizeof(far_rpts1s[0]);
    resample_points(far_rpts1b, far_ipts1_num, far_rpts1s, &far_rpts1s_num,
                    SAMPLE_DIST * PIXEL_PER_METER);

    // 边线局部角度变化率
    local_angle_points(far_rpts0s, far_rpts0s_num, far_rpts0a,
                       (int)round(ANGLE_DIST / SAMPLE_DIST));
    local_angle_points(far_rpts1s, far_rpts1s_num, far_rpts1a,
                       (int)round(ANGLE_DIST / SAMPLE_DIST));

    // 角度变化率非极大抑制
    nms_angle(far_rpts0a, far_rpts0s_num, far_rpts0an,
              (int)round(ANGLE_DIST / SAMPLE_DIST) * 2 + 1);
    nms_angle(far_rpts1a, far_rpts1s_num, far_rpts1an,
              (int)round(ANGLE_DIST / SAMPLE_DIST) * 2 + 1);

    // 找远线上的 L 角点
    far_Lpt0_found = far_Lpt1_found = false;
    for (int i = 0; i < AT_MIN(far_rpts0s_num, 80); i++) {
        if (far_rpts0an[i] == 0)
            continue;
        int im1 =
            clip(i - (int)round(ANGLE_DIST / SAMPLE_DIST), 0, far_rpts0s_num - 1);
        int ip1 =
            clip(i + (int)round(ANGLE_DIST / SAMPLE_DIST), 0, far_rpts0s_num - 1);
        float conf = fabs(far_rpts0a[i]) -
                     (fabs(far_rpts0a[im1]) + fabs(far_rpts0a[ip1])) / 2;

        if (30. / 180. * PI < conf && conf < 150. / 180. * PI && i < 100) {
            far_Lpt0_rpts0s_id = i;
            far_Lpt0_found = true;
            break;
        }
    }
    for (int i = 0; i < AT_MIN(far_rpts1s_num, 80); i++) {
        if (far_rpts1an[i] == 0)
            continue;
        int im1 =
            clip(i - (int)round(ANGLE_DIST / SAMPLE_DIST), 0, far_rpts1s_num - 1);
        int ip1 =
            clip(i + (int)round(ANGLE_DIST / SAMPLE_DIST), 0, far_rpts1s_num - 1);
        float conf = fabs(far_rpts1a[i]) -
                     (fabs(far_rpts1a[im1]) + fabs(far_rpts1a[ip1])) / 2;

        if (30. / 180. * PI < conf && conf < 150. / 180. * PI && i < 100) {
            far_Lpt1_rpts1s_id = i;
            far_Lpt1_found = true;
            break;
        }
    }
}

/* ********************************************************************* */