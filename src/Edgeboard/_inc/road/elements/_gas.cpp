#include "_gas.hpp"

#include <algorithm>

#include "../imgprocess.hpp"
#include "../utils/logger_helper.hpp"

#undef LOG_TAG
#define LOG_TAG "Gas"

flag_gas_e flag_gas = GAS_NONE;

// 引用图像处理
extern ImageProcess imgprocess;
#define IMG imgprocess

// 变量定义
float gas_route = 0;         // 加油站编码器积分
POINT gas_near_cone;         // 较近的锥桶
bool gas_which_is = false;   // 加油站出站口号检测标志
uint8_t gas_which = 1;       // 加油站出站口号
uint8_t gas_which_last = 0;  // 加油站出站口号 上一次

/* ********************************************************************* */

void check_gas() {
    if (flag_gas == GAS_DETECTION) {
        IMG.element_identify = 1;
        gas_route = IMG.encoder.route;
        flag_gas = GAS_FOUND;
        MAT_LOG("GAS_FOUND");
    }
}

/* ********************************************************************* */

void run_gas() {
    gas_near_cone = POINT(0, 0);
    IMG.edge_det.clear();

    /* ***************************************************************** */

    if (flag_gas == GAS_FOUND) {
        // 清弯道防止加大 P 项, 提高稳定性
        IMG.is_curve0 = IMG.is_curve1 = false;

        // 找目标点
        IMG.edge_det = search_targets(IMG._det_result);
        gas_near_cone = search_left_near(IMG.edge_left, IMG.mat_lab);

        // 当车辆开始靠近右边锥桶: 准备进入
        if (gas_near_cone.x > IMAGE_HEIGHT * 0.25f) {
            gas_route = IMG.encoder.route;
            flag_gas = GAS_READY;
            MAT_LOG("GAS_READY");
        }

        // 超过指定距离未进入绕行区, 元素行驶失败
        if (flag_gas != GAS_READY && (IMG.encoder.route - gas_route > 150)) {
            IMG.element_over = true;
            IMG.element_over_route = IMG.encoder.route;
            flag_gas = GAS_NONE;
            MAT_LOG("GAS_ERROR");
        }
    }

    /* ***************************************************************** */

    if (flag_gas == GAS_READY) {
        // 清弯道防止加大 P 项, 提高稳定性
        IMG.is_curve0 = IMG.is_curve1 = false;

        // 左入口 巡右线
        IMG.flag_track = TRACK_RIGHT;

        // 找目标点
        IMG.edge_det = search_targets(IMG._det_result);
        gas_near_cone = search_left_near(IMG.edge_left, IMG.mat_lab);
        POINT cone_RD = search_rightDown_cone(IMG.edge_det);

        // 补线
        if (IMG.encoder.route - gas_route < IMG._config->gas_rou_ready) {
            // 第一阶段: 当赛道边缘存在时
            if (IMG.rpts1s_num > 0.5 / SAMPLE_DIST) {
                // 补线起点: 右
                POINT startPoint = POINT((gas_near_cone.x + IMAGE_HEIGHT) / 2,
                                         (gas_near_cone.y + IMAGE_WIDTH) / 2);

                double k = 0, b = 0;
                k = (float)(gas_near_cone.y - startPoint.y) /
                    (float)(gas_near_cone.x - startPoint.x);
                b = gas_near_cone.y - k * gas_near_cone.x;
                if (b < 0)
                    b = 0;
                else if (b >= IMAGE_WIDTH)
                    b = IMAGE_WIDTH - 1;

                POINT endPoint = POINT(0, b);    // 补线终点: 左
                POINT midPoint = gas_near_cone;  // 补线中点
                POINT SSPoint = POINT(BEGIN_Y, IMG.edge_right[0].y);

                vector<POINT> input = {endPoint, midPoint, startPoint, SSPoint};
                vector<POINT> repair = bezier(0.03, input);

                IMG.edge_right = repair;
                IMG.last_edge_right = IMG.edge_right;
            }
            // 第二阶段: 检查右下锥桶坐标满足巡航条件
            else {
                if (cone_RD.x > IMAGE_HEIGHT / 3) {
                    // 补线起点: 右
                    POINT startPoint = POINT((cone_RD.x + IMAGE_HEIGHT) / 2,
                                             (cone_RD.y + IMAGE_WIDTH) / 2);

                    double k = 0, b = 0;
                    k = (float)(cone_RD.y - startPoint.y) /
                        (float)(cone_RD.x - startPoint.x);
                    b = cone_RD.y - k * cone_RD.x;
                    if (b < 0)
                        b = 0;
                    else if (b >= IMAGE_WIDTH)
                        b = IMAGE_WIDTH - 1;

                    POINT endPoint = POINT(0, b);  // 补线终点: 左
                    POINT midPoint = cone_RD;      // 补线中点

                    vector<POINT> input = {startPoint, midPoint, endPoint};
                    vector<POINT> repair = bezier(0.02, input);

                    IMG.edge_right = repair;
                    IMG.last_edge_right = IMG.edge_right;
                } else {
                    IMG.edge_right = IMG.last_edge_right;
                }
            }
        } else {
            gas_route = IMG.encoder.route;
            flag_gas = GAS_RUN;
            MAT_LOG("GAS_RUN");
        }
    }

    /* ***************************************************************** */

    if (flag_gas == GAS_RUN) {
        // 清弯道防止加大 P 项, 提高稳定性
        IMG.is_curve0 = IMG.is_curve1 = false;

        // 左区域 巡左线
        IMG.flag_track = TRACK_LEFT;

        // 找目标点
        IMG.edge_det = search_targets(IMG._det_result);
        vector<POINT> conesLeft = search_left_cones(IMG.edge_det);  // 左方锥桶

        // 补线
        if (conesLeft.size() >= 2) {
            uint8_t endd = conesLeft.size() > 2 ? conesLeft.size() - 2 : conesLeft.size() - 1;
            double k = 0, b = 0;
            k = (float)(conesLeft[0].y - conesLeft[endd].y) /
                (float)(conesLeft[0].x - conesLeft[endd].x);
            b = conesLeft[0].y - k * conesLeft[0].x;

            IMG.edge_left = IMG.last_edge_left;
            if (k != 0 && b != 0) {
                POINT startPoint = POINT(-b / k, 0);  // 补线起点: 左
                POINT endPoint = POINT(0, b);         // 补线终点: 右
                POINT midPoint = POINT((startPoint.x + endPoint.x) * 0.5,
                                       (startPoint.y + endPoint.y) * 0.5);  // 补线中点

                vector<POINT> input = {startPoint, midPoint, endPoint};
                vector<POINT> repair = bezier(0.02, input);

                // 左边缘切行, 提升右拐能力
                if (repair.size() > 10) {
                    int index = repair.size() * 0.33f;
                    IMG.edge_left.clear();
                    for (size_t i = index; i < repair.size(); i++) {
                        IMG.edge_left.push_back(repair[i]);
                    }
                } else
                    IMG.edge_left = repair;
                IMG.last_edge_left = IMG.edge_left;
            }
        } else {
            IMG.edge_left = IMG.last_edge_left;
        }

        // 出加油站
        if (gas_which == 1) {
            if (IMG.encoder.route - gas_route >= IMG._config->gas_rou_run1) {
                gas_route = IMG.encoder.route;
                flag_gas = GAS_OUT;
                MAT_LOG("GAS_OUT_#1");
            }
        } else {
            if (IMG.encoder.route - gas_route >= IMG._config->gas_rou_run2) {
                gas_route = IMG.encoder.route;
                flag_gas = GAS_OUT;
                MAT_LOG("GAS_OUT_#2");
            }
        }
    }

    /* ***************************************************************** */

    if (flag_gas == GAS_OUT) {
        // 加大 P 项, 提升出区域稳定性
        IMG.is_curve0 = IMG.is_curve1 = true;

        // 右出口 巡左线
        IMG.flag_track = TRACK_LEFT;
        IMG.begin_x_l = IMAGE_WIDTH - 1;

        // 找目标点
        IMG.edge_det = search_targets(IMG._det_result);
        POINT coneOut = search_rightUp_cone(IMG.edge_det);    // 右上锥桶: 2号出口
        POINT cone_RD = search_rightDown_cone(IMG.edge_det);  // 右下锥桶: 1号出口

        uint8_t route_out = IMG._config->gas_rou_out1;
        if (gas_which == 1) {
            route_out = IMG._config->gas_rou_out2;
            if (IMG.encoder.route - gas_route >= 30)
                coneOut = cone_RD;
            else
                coneOut = POINT(IMAGE_HEIGHT / 2, IMAGE_WIDTH - 36);
        }

        if ((AT_IMAGE(IMG.mat_bin, (int)(IMAGE_WIDTH / 2 + IMG._config->gas_rou_none), (int)(IMAGE_HEIGHT / 2)) > IMG._config->threshold) ||
            (IMG.encoder.route - gas_route >= route_out)) {
            // 切换出口
            // gas_which_last = gas_which;
            // if (gas_which_is)
            // gas_which = gas_which == 1 ? 2 : 1;

            if (IMG.rpts1s_num > 0.2 / SAMPLE_DIST) {
                IMG.element_begin_id = IMG.rpts1s_num * 0.25f;
                IMG.flag_track = TRACK_RIGHT;
            } else {
                IMG.element_begin_id = IMG.rpts0s_num * 0.25f;
                IMG.flag_track = TRACK_LEFT;
            }
            IMG.special_g = true;
            IMG.element_over = true;
            IMG.element_over_route = IMG.encoder.route + IMG._config->gas_rou_over;
            flag_gas = GAS_NONE;
            MAT_LOG("GAS_NONE");
        } else {
            if (coneOut.x > 0) {
                POINT p2 = POINT((coneOut.x + IMAGE_HEIGHT) / 2, coneOut.y / 2);
                POINT p3 = coneOut;
                POINT p4 = POINT(coneOut.x / 2, (coneOut.y + IMAGE_WIDTH) / 2);

                vector<POINT> input = {p2, p3, p4};
                vector<POINT> repair = bezier(0.02, input);

                // 左边缘切行, 提升右拐能力
                if (repair.size() > 10) {
                    int index = repair.size() * 0.2f;
                    IMG.edge_left.clear();
                    for (size_t i = index; i < repair.size(); i++) {
                        IMG.edge_left.push_back(repair[i]);
                    }
                } else
                    IMG.edge_left = repair;

                IMG.last_edge_left = IMG.edge_left;
            } else {
                IMG.edge_left = IMG.last_edge_left;
            }
        }
    }

    /* ***************************************************************** */

    if (flag_gas > GAS_FOUND) {
        IMG.det_ipts_num = 0;

        if (IMG.flag_track == TRACK_LEFT) {
            // 进行降序排序
            std::sort(IMG.edge_left.begin(), IMG.edge_left.end(), sort_x_down);
            for (size_t i = 0; i < IMG.edge_left.size(); i++) {
                if (i >= IMAGE_HEIGHT)
                    break;
                IMG.det_ipts[i][0] = IMG.edge_left[i].y;
                IMG.det_ipts[i][1] = IMG.edge_left[i].x;
                IMG.det_ipts_num++;
            }
        } else {
            // 进行降序排序
            std::sort(IMG.edge_right.begin(), IMG.edge_right.end(), sort_x_down);
            for (size_t i = 0; i < IMG.edge_right.size(); i++) {
                if (i >= IMAGE_HEIGHT)
                    break;
                IMG.det_ipts[i][0] = IMG.edge_right[i].y;
                IMG.det_ipts[i][1] = IMG.edge_right[i].x;
                IMG.det_ipts_num++;
            }
        }

        // 原图边线 -> 透视边线
        for (int i = 0; i < IMG.det_ipts_num; i++)
            map_perspective(IMG.det_ipts[i][0], IMG.det_ipts[i][1], IMG.det_rpts[i], 0);

        // 边线滤波
        blur_points(IMG.det_rpts, IMG.det_ipts_num, IMG.det_rptsb,
                    (int)round(LINE_BLUR_KERNEL));

        // 边线等距采样
        IMG.det_rptss_num = sizeof(IMG.det_rptss) / sizeof(IMG.det_rptss[0]);
        resample_points(IMG.det_rptsb, IMG.det_ipts_num, IMG.det_rptss,
                        &IMG.det_rptss_num, SAMPLE_DIST * PIXEL_PER_METER);
    }
}

/* ********************************************************************* */