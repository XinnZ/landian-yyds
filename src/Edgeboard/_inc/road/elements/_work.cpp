#include "_work.hpp"

#include <algorithm>

#include "../imgprocess.hpp"
#include "../utils/logger_helper.hpp"

#undef LOG_TAG
#define LOG_TAG "Work"

flag_work_e flag_work = WORK_NONE;

// 引用图像处理
extern ImageProcess imgprocess;
#define IMG imgprocess

// 变量定义
float work_route = 0;  // 施工区编码器积分
POINT work_near_cone;  // 较近的锥桶

/* ********************************************************************* */

void check_work() {
    if (flag_work == WORK_DETECTION) {
        IMG.element_identify = 1;
        work_route = IMG.encoder.route;
        flag_work = WORK_FOUND;
        MAT_LOG("WORK_FOUND");
    }
}

/* ********************************************************************* */

void run_work() {
    work_near_cone = POINT(0, 0);
    IMG.edge_det.clear();

    /* ***************************************************************** */

    if (flag_work == WORK_FOUND) {
        // 清弯道防止加大 P 项, 提高稳定性
        IMG.is_curve0 = IMG.is_curve1 = false;

        // 找锥桶
        IMG.edge_det = search_targets(IMG._det_result);

        // 进入方向
        if (IMG._config->direction_work == 0)
            work_near_cone = search_left_near(IMG.edge_left, IMG.mat_lab);
        else
            work_near_cone = search_right_near(IMG.edge_right, IMG.mat_lab);

        // 当车辆开始靠近锥桶: 准备进入
        if (work_near_cone.x > IMAGE_HEIGHT * 0.25f) {
            work_route = IMG.encoder.route;
            flag_work = WORK_READY;
            MAT_LOG("WORK_READY");
        }

        // 超过指定距离未进入绕行区, 元素行驶失败
        if (flag_work != WORK_READY && (IMG.encoder.route - work_route > 150)) {
            IMG.element_over = true;
            IMG.element_over_route = IMG.encoder.route;
            flag_work = WORK_NONE;
            MAT_LOG("WORK_ERROR");
        }
    }

    /* ***************************************************************** */
    /* *************************** 左侧施工区 **************************** */
    /* ***************************************************************** */

    if (IMG._config->direction_work == 0) {
        if (flag_work == WORK_READY) {
            // 清弯道防止加大 P 项, 提高稳定性
            IMG.is_curve0 = IMG.is_curve1 = false;

            // 左入口 巡右线
            IMG.flag_track = TRACK_RIGHT;

            // 找目标点
            IMG.edge_det = search_targets(IMG._det_result);
            work_near_cone = search_left_near(IMG.edge_left, IMG.mat_lab);
            POINT cone_RD = search_rightDown_cone(IMG.edge_det);

            // 补线
            if (IMG.encoder.route - work_route < IMG._config->work_rou_ready) {
                // 第一阶段: 当赛道边缘存在时
                if (IMG.rpts1s_num > 0.5 / SAMPLE_DIST) {
                    // 补线起点: 右
                    POINT startPoint = POINT((work_near_cone.x + IMAGE_HEIGHT) / 2,
                                             (work_near_cone.y + IMAGE_WIDTH) / 2);

                    double k = 0, b = 0;
                    k = (float)(work_near_cone.y - startPoint.y) /
                        (float)(work_near_cone.x - startPoint.x);
                    b = work_near_cone.y - k * work_near_cone.x;
                    if (b < 0)
                        b = 0;
                    else if (b >= IMAGE_WIDTH)
                        b = IMAGE_WIDTH - 1;

                    POINT endPoint = POINT(0, b);     // 补线终点: 左
                    POINT midPoint = work_near_cone;  // 补线中点
                    POINT SSPoint = POINT(BEGIN_Y, IMG.edge_right[0].y);

                    vector<POINT> input = {endPoint, midPoint, startPoint, SSPoint};
                    vector<POINT> repair = bezier(0.03, input);

                    IMG.edge_right = repair;
                }
                // 第二阶段: 检查右下锥桶坐标满足巡航条件
                else {
                    if (cone_RD.x > 0) {
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
                    }
                }
            } else {
                work_route = IMG.encoder.route;
                flag_work = WORK_RUN;
                MAT_LOG("WORK_RUN");
            }
        }

        // *****************************************************************

        if (flag_work == WORK_RUN) {
            // 清弯道防止加大 P 项, 提高稳定性
            IMG.is_curve0 = IMG.is_curve1 = false;

            // 左区域 巡左线
            IMG.flag_track = TRACK_LEFT;

            // 找目标点
            IMG.edge_det = search_targets(IMG._det_result);
            vector<POINT> conesLeft = search_left_cones(IMG.edge_det);  // 左方锥桶
            POINT cone_RU = search_rightUp_cone(IMG.edge_det);          // 右上锥桶

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

            // 积分延迟一定距离出施工区
            if ((cone_RU.x >= 55) || (IMG.encoder.route - work_route >= IMG._config->work_rou_run)) {
                work_route = IMG.encoder.route;
                flag_work = WORK_OUT;
                MAT_LOG("WORK_OUT");
            }
        }

        // *****************************************************************

        if (flag_work == WORK_OUT) {
            // 加大 P 项, 提升出区域稳定性
            IMG.is_curve0 = IMG.is_curve1 = true;

            // 右出口 巡左线
            IMG.flag_track = TRACK_LEFT;
            IMG.begin_x_l = IMAGE_WIDTH - 1;

            // 找目标点
            IMG.edge_det = search_targets(IMG._det_result);
            POINT cone_RU = search_rightUp_cone(IMG.edge_det);  // 右上锥桶

            if ((AT_IMAGE(IMG.mat_bin, (int)(IMAGE_WIDTH / 2 + IMG._config->work_rou_none), (int)(IMAGE_HEIGHT / 2)) > IMG._config->threshold) ||
                (IMG.encoder.route - work_route >= IMG._config->work_rou_out)) {
                if (IMG.rpts1s_num > 0.2 / SAMPLE_DIST) {
                    IMG.element_begin_id = IMG.rpts1s_num * 0.25f;
                    IMG.flag_track = TRACK_RIGHT;
                } else {
                    IMG.element_begin_id = IMG.rpts0s_num * 0.25f;
                    IMG.flag_track = TRACK_LEFT;
                }
                IMG.special_w_c = true;
                IMG.special_w = true;
                IMG.element_over = true;
                IMG.element_over_route = IMG.encoder.route + IMG._config->work_rou_over;
                flag_work = WORK_NONE;
                MAT_LOG("WORK_NONE");
            } else {
                if (cone_RU.x > 0) {
                    POINT p2 = POINT((cone_RU.x + IMAGE_HEIGHT) / 2, cone_RU.y / 2);
                    POINT p3 = cone_RU;
                    POINT p4 = POINT(cone_RU.x / 2, (cone_RU.y + IMAGE_WIDTH) / 2);

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
    }

    /* ***************************************************************** */
    /* *************************** 右侧施工区 **************************** */
    /* ***************************************************************** */
    /* ********************* 注意！！！ 右侧施工区有问题 ******************** */
    /* ************* 国赛前没调右侧施工区, 比赛的时候没有进入施工区 ************* */
    /* **************** 比赛完就没管了, 这个地方需要看图像调试 **************** */

    else {
        if (flag_work == WORK_READY) {
            // 清弯道防止加大 P 项, 提高稳定性
            IMG.is_curve0 = IMG.is_curve1 = false;

            // 右入口 巡左线
            IMG.flag_track = TRACK_LEFT;

            // 找目标点
            IMG.edge_det = search_targets(IMG._det_result);
            work_near_cone = search_right_near(IMG.edge_right, IMG.mat_lab);
            POINT cone_LD = search_leftDown_cone(IMG.edge_det);

            // 补线
            if (IMG.encoder.route - work_route < IMG._config->work_rou_ready) {
                // 第一阶段: 当赛道边缘存在时
                if (IMG.rpts0s_num > 0.5 / SAMPLE_DIST) {
                    // 补线起点: 左
                    POINT startPoint = POINT((work_near_cone.x + IMAGE_HEIGHT) / 2,
                                             work_near_cone.y / 2);

                    double k = 0, b = 0;
                    k = (float)(work_near_cone.y - startPoint.y) /
                        (float)(work_near_cone.x - startPoint.x);
                    b = work_near_cone.y - k * work_near_cone.x;
                    if (b < 0)
                        b = 0;
                    else if (b >= IMAGE_WIDTH)
                        b = IMAGE_WIDTH - 1;

                    POINT endPoint = POINT(0, b);     // 补线终点: 右
                    POINT midPoint = work_near_cone;  // 补线中点
                    POINT SSPoint = POINT(BEGIN_Y, IMG.edge_left[0].y);

                    vector<POINT> input = {endPoint, midPoint, startPoint, SSPoint};
                    vector<POINT> repair = bezier(0.03, input);

                    IMG.edge_left = repair;
                    IMG.last_edge_left = IMG.edge_left;
                }
                // 第二阶段: 检查左下锥桶坐标满足巡航条件
                else {
                    if (cone_LD.x > IMAGE_HEIGHT / 3) {
                        // 补线起点: 左
                        POINT startPoint = POINT((cone_LD.x + IMAGE_HEIGHT) / 2,
                                                 cone_LD.y / 2);

                        double k = 0, b = 0;
                        k = (float)(cone_LD.y - startPoint.y) /
                            (float)(cone_LD.x - startPoint.x);
                        b = cone_LD.y - k * cone_LD.x;
                        if (b < 0)
                            b = 0;
                        else if (b >= IMAGE_WIDTH)
                            b = IMAGE_WIDTH - 1;

                        POINT endPoint = POINT(0, b);  // 补线终点: 右
                        POINT midPoint = cone_LD;      // 补线中点

                        vector<POINT> input = {startPoint, midPoint, endPoint};
                        vector<POINT> repair = bezier(0.02, input);

                        IMG.edge_left = repair;
                        IMG.last_edge_left = IMG.edge_left;
                    } else {
                        IMG.edge_left = IMG.last_edge_left;
                    }
                }
            } else {
                work_route = IMG.encoder.route;
                flag_work = WORK_RUN;
                MAT_LOG("WORK_RUN");
            }
        }

        /* ***************************************************************** */

        if (flag_work == WORK_RUN) {
            // 清弯道防止加大 P 项, 提高稳定性
            IMG.is_curve0 = IMG.is_curve1 = false;

            // 右区域 巡右线
            IMG.flag_track = TRACK_RIGHT;

            // 找目标点
            IMG.edge_det = search_targets(IMG._det_result);
            vector<POINT> conesRight = search_right_cones(IMG.edge_det);  // 右方锥桶

            // 补线
            if (conesRight.size() >= 2) {
                uint8_t endd = conesRight.size() > 2 ? conesRight.size() - 2 : conesRight.size() - 1;
                double k = 0, b = 0;

                double m = conesRight[endd].y - conesRight[0].y;
                if (0 == m) {
                    k = 10000.0f;
                    b = conesRight[0].x - k * conesRight[0].y;
                } else {
                    k = (conesRight[endd].x - conesRight[0].x) /
                        (conesRight[endd].y - conesRight[0].y);
                    b = conesRight[0].x - k * conesRight[0].y;
                }
                int xxx = (int)(-b / k);
                xxx = clip(xxx, 0, IMAGE_WIDTH - 1);
                int yyy = int(k * (IMAGE_WIDTH - 1) + b);
                yyy = clip(yyy, 0, IMAGE_HEIGHT - 1);

                POINT startPoint = POINT(yyy, IMAGE_WIDTH - 1);  // 补线起点
                POINT endPoint = POINT(0, xxx);                  // 补线终点
                POINT midPoint = POINT((startPoint.x + endPoint.x) * 0.5,
                                       (startPoint.y + endPoint.y) * 0.5);  // 补线中点

                vector<POINT> input = {startPoint, midPoint, endPoint};
                vector<POINT> repair = bezier(0.02, input);

                // 右边缘切行, 提升左拐能力
                if (repair.size() > 10) {
                    int index = repair.size() * 0.33f;
                    IMG.edge_right.clear();
                    for (size_t i = index; i < repair.size(); i++) {
                        IMG.edge_right.push_back(repair[i]);
                    }
                } else
                    IMG.edge_right = repair;
                IMG.last_edge_right = IMG.edge_right;
            } else {
                IMG.edge_right = IMG.last_edge_right;
            }

            // 积分延迟一定距离出施工区
            if (IMG.encoder.route - work_route >= IMG._config->work_rou_run) {
                work_route = IMG.encoder.route;
                flag_work = WORK_OUT;
                MAT_LOG("WORK_OUT");
            }
        }

        /* ***************************************************************** */

        if (flag_work == WORK_OUT) {
            // 加大 P 项, 提升出区域稳定性
            IMG.is_curve0 = IMG.is_curve1 = true;

            // 左出口 巡右线
            IMG.flag_track = TRACK_RIGHT;
            IMG.begin_x_r = 0;

            // 找目标点
            IMG.edge_det = search_targets(IMG._det_result);
            POINT cone_LU = search_leftUp_cone(IMG.edge_det);  // 左上锥桶

            if ((AT_IMAGE(IMG.mat_bin, (int)(IMAGE_WIDTH / 2 - IMG._config->work_rou_none), (int)(IMAGE_HEIGHT / 2)) > IMG._config->threshold) ||
                (IMG.encoder.route - work_route >= IMG._config->work_rou_out)) {
                if (IMG.rpts0s_num > 0.2 / SAMPLE_DIST) {
                    IMG.element_begin_id = IMG.rpts0s_num * 0.25f;
                    IMG.flag_track = TRACK_LEFT;
                } else {
                    IMG.element_begin_id = IMG.rpts1s_num * 0.25f;
                    IMG.flag_track = TRACK_RIGHT;
                }
                IMG.special_w_c = true;
                IMG.special_w = true;
                IMG.element_over = true;
                IMG.element_over_route = IMG.encoder.route + IMG._config->work_rou_over;
                flag_work = WORK_NONE;
                MAT_LOG("WORK_NONE");
            } else {
                if (cone_LU.x > 0) {
                    POINT p2 = POINT((cone_LU.x + IMAGE_HEIGHT) / 2, (cone_LU.y + IMAGE_WIDTH) / 2);
                    POINT p3 = cone_LU;
                    POINT p4 = POINT(cone_LU.x / 2, cone_LU.y / 2);

                    vector<POINT> input = {p2, p3, p4};
                    vector<POINT> repair = bezier(0.02, input);

                    // 左边缘切行, 提升右拐能力
                    if (repair.size() > 10) {
                        int index = repair.size() * 0.2f;
                        IMG.edge_right.clear();
                        for (size_t i = index; i < repair.size(); i++) {
                            IMG.edge_right.push_back(repair[i]);
                        }
                    } else
                        IMG.edge_right = repair;

                    IMG.last_edge_right = IMG.edge_right;
                } else {
                    IMG.edge_right = IMG.last_edge_right;
                }
            }
        }
    }

    /* ***************************************************************** */
    /* ***************************************************************** */

    if (flag_work > WORK_FOUND) {
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