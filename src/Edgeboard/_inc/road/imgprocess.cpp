#include "imgprocess.hpp"

#undef LOG_TAG
#define LOG_TAG "Run"

/* 初始化 */
int ImageProcess::init(std::string config_path, bool is_result) {
    _config = make_shared<ControlConfig>(config_path);
    _is_result = is_result;

    // 初始化出库方向
    if (_config->direction_garage == 0)
        flag_garage = GARAGE_OUT_LEFT;
    else if (_config->direction_garage == 1)
        flag_garage = GARAGE_OUT_RIGHT;
    else
        flag_garage = GARAGE_NONE;

    // 初始化参数
    aim_distance_f = _config->aim_distance_far;
    aim_distance_n = _config->aim_distance_near;
    element_over = false;
    speed_diff = (_config->speed_up - _config->speed_base) / 10;

    return 0;
}

/* 图像处理 */
void ImageProcess::process(cv::Mat &mat_origin, cv::Mat &mat_result,
                           Payload_t &payload) {
    /* ***************************************************************** */
    /* ***************************** 初始化 ***************************** */
    /* ***************************************************************** */

    // 图像预处理: 缩放 -> 灰度 -> 二值化
    cv::resize(mat_origin, mat_result, cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), 0, 0, cv::INTER_AREA);
    cv::cvtColor(mat_result, mat_bin, cv::COLOR_BGR2GRAY);
    cv::threshold(mat_bin, mat_bin, 0, 255, cv::THRESH_OTSU);

    // 获取 LAB 图片 A 通道 (红色) -> 提取锥桶
    cv::cvtColor(mat_result, mat_lab, cv::COLOR_BGR2Lab);
    std::vector<cv::Mat> channels_lab;
    cv::split(mat_lab, channels_lab);
    cv::threshold(channels_lab[1], mat_lab, 150, 255, cv::THRESH_BINARY);

    // 绘制处理结果图, 灰度格式转BGR格式
    /*
    if (_is_result) {
        std::vector<cv::Mat> channels;
        // for (int i = 0; i < 3; i++) channels.push_back(mat_lab);
        for (int i = 0; i < 3; i++) channels.push_back(mat_bin);
        cv::merge(channels, mat_result);
    }
    */

    /* ***************************************************************** */
    /* **************************** 边线处理 **************************** */
    /* ***************************************************************** */

    // 预瞄点纵坐标黑点检测 ------------------------------------------------
    begin_y_t = BEGIN_Y;
    if ((flag_gas <= GAS_FOUND && flag_work <= WORK_FOUND &&
         (flag_garage != GARAGE_IN_LEFT) && (flag_garage != GARAGE_IN_RIGHT) &&
         (!(is_curve0 && flag_track == TRACK_LEFT) && !(is_curve1 && flag_track == TRACK_RIGHT))) ||
        (flag_garage == GARAGE_PASS_LEFT || flag_garage == GARAGE_PASS_RIGHT)) {
        int black_sum = 0;
        for (int i = 100; i < IMAGE_WIDTH - 100; i++) {
            if (AT_IMAGE(mat_bin, i, begin_y_t) < _config->threshold)
                black_sum++;
        }
        if (black_sum > 10) {
            begin_y_t = BEGIN_Y - 50;
            begin_x_l = begin_x_l + 50 > IMAGE_WIDTH - 1 ? IMAGE_WIDTH - 1 : begin_x_l + 50;
            begin_x_r = begin_x_r - 50 < 0 ? 0 : begin_x_r - 50;
        }
    }

    // 原图找左边线 -------------------------------------------------------
    {
        int x1 = begin_x_l, y1 = begin_y_t;
        // 向左寻找白点
        if (AT_IMAGE(mat_bin, x1, y1) < _config->threshold)
            for (x1--; x1 > 0; x1--)
                if (AT_IMAGE(mat_bin, x1, y1) >= _config->threshold)
                    break;
        // 向左寻找黑点
        for (; x1 > 0; x1--)
            if (AT_IMAGE(mat_bin, x1 - 1, y1) < _config->threshold)
                break;
        // 向上寻找黑点
        if (x1 < BLOCK_SIZE / 2) {
            x1 = BLOCK_SIZE / 2;
            for (; y1 > IMAGE_HEIGHT * 2 / 3; y1--)
                if (AT_IMAGE(mat_bin, x1, y1 - 1) < _config->threshold)
                    break;
        }
        if (AT_IMAGE(mat_bin, x1, y1) >= _config->threshold) {
            ipts0_num = y1 + (IMAGE_HEIGHT - BEGIN_Y);
            findline_lefthand_adaptive(mat_bin, BLOCK_SIZE, CLIP_VALUE,
                                       x1, y1, ipts0, &ipts0_num);
            begin_x_l = x1 + 50 > IMAGE_WIDTH - 1 ? IMAGE_WIDTH - 1 : x1 + 50;
        } else {
            ipts0_num = 0;
            begin_x_l = BEGIN_X;
        }
    }
    // 原图找右边线 -------------------------------------------------------
    {
        int x2 = begin_x_r, y2 = begin_y_t;
        // 向右寻找白点
        if (AT_IMAGE(mat_bin, x2, y2) < _config->threshold)
            for (x2++; x2 < IMAGE_WIDTH - 1; x2++)
                if (AT_IMAGE(mat_bin, x2, y2) >= _config->threshold)
                    break;
        // 向右寻找黑点
        for (; x2 < IMAGE_WIDTH - 1; x2++)
            if (AT_IMAGE(mat_bin, x2 + 1, y2) < _config->threshold)
                break;
        // 向上寻找黑点
        if (x2 > IMAGE_WIDTH - BLOCK_SIZE / 2 - 1) {
            x2 = IMAGE_WIDTH - BLOCK_SIZE / 2 - 1;
            for (; y2 > IMAGE_HEIGHT * 2 / 3; y2--)
                if (AT_IMAGE(mat_bin, x2, y2 - 1) < _config->threshold)
                    break;
        }
        if (AT_IMAGE(mat_bin, x2, y2) >= _config->threshold) {
            ipts1_num = y2 + (IMAGE_HEIGHT - BEGIN_Y);
            findline_righthand_adaptive(mat_bin, BLOCK_SIZE, CLIP_VALUE,
                                        x2, y2, ipts1, &ipts1_num);
            begin_x_r = x2 - 50 < 0 ? 0 : x2 - 50;
        } else {
            ipts1_num = 0;
            begin_x_r = IMAGE_WIDTH - BEGIN_X;
        }
    }

    // 变换后左右中线
    float rpts0[IMAGE_HEIGHT][2] = {0};
    float rpts1[IMAGE_HEIGHT][2] = {0};
    float rptsc[IMAGE_HEIGHT][2] = {0};
    // 原图边线转换数据定义
    int iptsc0[IMAGE_HEIGHT] = {0};
    int iptsc1[IMAGE_HEIGHT] = {0};

    // 原图边线 -> 透视边线 左
    rptsc_num = 0;
    edge_left.clear();
    for (int i = 0; i < ipts0_num; i++, rptsc_num++) {
        if (ipts0[i][1] < 10)
            break;

        map_perspective(ipts0[i][0], ipts0[i][1], rpts0[i], 0);
        iptsc0[ipts0[i][1]] = ipts0[i][0];

        // 注意: x 与 y 位置相反 !!!
        POINT pointTmp(ipts0[i][1], ipts0[i][0]);
        edge_left.push_back(pointTmp);
    }
    ipts0_num = rptsc_num;
    // 原图边线 -> 透视边线 右
    rptsc_num = 0;
    edge_right.clear();
    for (int i = 0; i < ipts1_num; i++, rptsc_num++) {
        if (ipts1[i][1] < 10)
            break;

        map_perspective(ipts1[i][0], ipts1[i][1], rpts1[i], 0);
        iptsc1[ipts1[i][1]] = ipts1[i][0];

        // 注意: x 与 y 位置相反 !!!
        POINT pointTmp(ipts1[i][1], ipts1[i][0]);
        edge_right.push_back(pointTmp);
    };
    ipts1_num = rptsc_num;

    // 中线获取 图像顶部10个像素丢弃
    rptsc_num = 0;
    for (int ccy = IMAGE_HEIGHT - 1; ccy >= 10; ccy--) {
        iptsc[ccy] = iptsc0[ccy] + iptsc1[ccy];

        if (iptsc[ccy] != 0) {
            if (iptsc1[ccy] == 0)
                iptsc[ccy] = (int)((IMAGE_WIDTH - iptsc[ccy]) / 2 + iptsc[ccy]);
            else
                iptsc[ccy] = (int)(iptsc[ccy] / 2);

            // 原图中线 -> 透视中线
            map_perspective(iptsc[ccy], ccy, rptsc[rptsc_num++], 0);
        }
    }

    // 滤波
    blur_points(rpts0, ipts0_num, rpts0b, (int)round(LINE_BLUR_KERNEL));
    blur_points(rpts1, ipts1_num, rpts1b, (int)round(LINE_BLUR_KERNEL));
    blur_points(rptsc, rptsc_num, rptscb, (int)round(LINE_BLUR_KERNEL));

    // 边线等距采样
    rpts0s_num = sizeof(rpts0s) / sizeof(rpts0s[0]);
    resample_points(rpts0b, ipts0_num, rpts0s, &rpts0s_num, SAMPLE_DIST * PIXEL_PER_METER);
    rpts1s_num = sizeof(rpts1s) / sizeof(rpts1s[0]);
    resample_points(rpts1b, ipts1_num, rpts1s, &rpts1s_num, SAMPLE_DIST * PIXEL_PER_METER);
    rptscs_num = sizeof(rptscs) / sizeof(rptscs[0]);
    resample_points(rptscb, rptsc_num, rptscs, &rptscs_num, SAMPLE_DIST * PIXEL_PER_METER);

    // 边线局部角度变化率
    local_angle_points(rpts0s, rpts0s_num, rpts0a, (int)round(ANGLE_DIST / SAMPLE_DIST));
    local_angle_points(rpts1s, rpts1s_num, rpts1a, (int)round(ANGLE_DIST / SAMPLE_DIST));

    // 角度变化率非极大抑制
    nms_angle(rpts0a, rpts0s_num, rpts0an, (int)round(ANGLE_DIST / SAMPLE_DIST) * 2 + 1);
    nms_angle(rpts1a, rpts1s_num, rpts1an, (int)round(ANGLE_DIST / SAMPLE_DIST) * 2 + 1);

    // 左右中线跟踪
    track_leftline(rpts0s, rpts0s_num, rptsc0,
                   (int)round(ANGLE_DIST / SAMPLE_DIST),
                   PIXEL_PER_METER * ROAD_WIDTH / 2);
    rptsc0_num = rpts0s_num;
    track_rightline(rpts1s, rpts1s_num, rptsc1,
                    (int)round(ANGLE_DIST / SAMPLE_DIST),
                    PIXEL_PER_METER * ROAD_WIDTH / 2);
    rptsc1_num = rpts1s_num;

    // 透视中线 -> 原图中线
    for (int i = 0; i < rptsc0_num; i++)
        map_perspective(rptsc0[i][0], rptsc0[i][1], rptsc0[i], 1);
    for (int i = 0; i < rptsc1_num; i++)
        map_perspective(rptsc1[i][0], rptsc1[i][1], rptsc1[i], 1);
    for (int i = 0; i < rptscs_num; i++)
        map_perspective(rptscs[i][0], rptscs[i][1], rptscs[i], 1);

    /* ***************************************************************** */
    /* *************************** 弯直道检测 *************************** */
    /* ***************************************************************** */

    // 标志位重置
    is_curve0 = is_curve1 = is_straight0 = is_straight1 = false;
    mea_0 = mea_1 = 10.0f;
    // 左线直线拟合
    if (rpts0s_num > 10) {
        mea_0 = fit_line(rpts0s, rpts0s_num, 60);
        if (mea_0 < 2.5f && rpts0s_num > 60)
            is_straight0 = true;
        else
            is_curve0 = true;
    }
    // 右线直线拟合
    if (rpts1s_num > 10) {
        mea_1 = fit_line(rpts1s, rpts1s_num, 60);
        if (mea_1 < 2.5f && rpts1s_num > 60)
            is_straight1 = true;
        else
            is_curve1 = true;
    }

    /* ***************************************************************** */
    /* **************************** 角点检测 **************************** */
    /* ***************************************************************** */

    // 角点重置
    Lpt0_found = Lpt1_found = false;
    // 左线角点
    for (int i = 0; i < rpts0s_num; i++) {
        if (rpts0an[i] == 0)
            continue;
        int im1 = clip(i - (int)round(ANGLE_DIST / SAMPLE_DIST), 0, rpts0s_num - 1);
        int ip1 = clip(i + (int)round(ANGLE_DIST / SAMPLE_DIST), 0, rpts0s_num - 1);
        float conf = fabs(rpts0a[i]) - (fabs(rpts0a[im1]) + fabs(rpts0a[ip1])) / 2;
        // L 角点阈值 0.6981317 < x < 2.44346
        if (Lpt0_found == false && 40. / 180. * PI < conf &&
            conf < 140. / 180. * PI && i < 0.8 / SAMPLE_DIST) {
            Lpt0_found_conf = conf;
            Lpt0_rpts0s_id = i;
            Lpt0_found = true;
        }
        if (Lpt0_found)
            break;
    }
    // 右线角点
    for (int i = 0; i < rpts1s_num; i++) {
        if (rpts1an[i] == 0)
            continue;
        int im1 = clip(i - (int)round(ANGLE_DIST / SAMPLE_DIST), 0, rpts1s_num - 1);
        int ip1 = clip(i + (int)round(ANGLE_DIST / SAMPLE_DIST), 0, rpts1s_num - 1);
        float conf = fabs(rpts1a[i]) - (fabs(rpts1a[im1]) + fabs(rpts1a[ip1])) / 2;
        // L 角点阈值
        if (Lpt1_found == false && 40. / 180. * PI < conf &&
            conf < 140. / 180. * PI && i < 0.8 / SAMPLE_DIST) {
            Lpt1_found_conf = conf;
            Lpt1_rpts1s_id = i;
            Lpt1_found = true;
        }
        if (Lpt1_found)
            break;
    }

    // L 点二次检查, 车库模式不检查, 依据两角点距离及角点后张开特性
    if (flag_garage == GARAGE_NONE) {
        if (Lpt0_found && Lpt1_found) {
            float dx = rpts0s[Lpt0_rpts0s_id][0] - rpts1s[Lpt1_rpts1s_id][0];
            float dy = rpts0s[Lpt0_rpts0s_id][1] - rpts1s[Lpt1_rpts1s_id][1];
            float dn = sqrtf(dx * dx + dy * dy);
            if (fabs(dn - 0.45 * PIXEL_PER_METER) < 0.15 * PIXEL_PER_METER) {
                float dwx = rpts0s[clip(Lpt0_rpts0s_id + 50, 0, rpts0s_num - 1)][0] -
                            rpts1s[clip(Lpt1_rpts1s_id + 50, 0, rpts1s_num - 1)][0];
                float dwy = rpts0s[clip(Lpt0_rpts0s_id + 50, 0, rpts0s_num - 1)][1] -
                            rpts1s[clip(Lpt1_rpts1s_id + 50, 0, rpts1s_num - 1)][1];
                float dwn = sqrtf(dwx * dwx + dwy * dwy);
                if (!(dwn > 0.7 * PIXEL_PER_METER &&
                      rpts0s[clip(Lpt0_rpts0s_id + 50, 0, rpts0s_num - 1)][0] <
                          rpts0s[Lpt0_rpts0s_id][0] &&
                      rpts1s[clip(Lpt1_rpts1s_id + 50, 0, rpts1s_num - 1)][0] >
                          rpts1s[Lpt1_rpts1s_id][0])) {
                    Lpt0_found = Lpt1_found = false;
                }
            } else
                Lpt0_found = Lpt1_found = false;
        }
    }

    // 角点二次确认 左
    if (Lpt0_found) {
        if (!Lpt0_found_last) {
            Lpt0_found_last = true;
            Lpt0_found = false;
        } else if (_is_result) {
            map_perspective(rpts0s[Lpt0_rpts0s_id][0], rpts0s[Lpt0_rpts0s_id][1],
                            trans, 1);
            MAT_INFO(mat_result, cv::Point(trans[0] + 5, trans[1]), 0.3, "L_%.3f",
                     Lpt0_found_conf);
        }
    } else
        Lpt0_found_last = false;
    // 角点二次确认 右
    if (Lpt1_found) {
        if (!Lpt1_found_last) {
            Lpt1_found_last = true;
            Lpt1_found = false;
        } else if (_is_result) {
            map_perspective(rpts1s[Lpt1_rpts1s_id][0], rpts1s[Lpt1_rpts1s_id][1],
                            trans, 1);
            MAT_INFO(mat_result, cv::Point(trans[0] + 5, trans[1]), 0.3, "L_%.3f",
                     Lpt1_found_conf);
        }
    } else
        Lpt1_found_last = false;

    /* ***************************************************************** */
    /* **************************** 选定中线 **************************** */
    /* *********************** 单侧线少, 切换巡线方向 ********************** */

    if (is_straight0 && is_straight1)
        flag_track = TRACK_MIDDLE;
    else if (is_straight0)
        flag_track = TRACK_LEFT;
    else if (is_straight1)
        flag_track = TRACK_RIGHT;
    else if (is_curve0 && is_curve1 && rptscs[rptscs_num - 1][0] > IMAGE_WIDTH / 2)
        flag_track = TRACK_LEFT;
    else if (is_curve0 && is_curve1 && rptscs[rptscs_num - 1][0] < IMAGE_WIDTH / 2)
        flag_track = TRACK_RIGHT;
    else if (rpts0s_num == 0 && rpts1s_num != 0)
        flag_track = TRACK_RIGHT;
    else if (rpts0s_num != 0 && rpts1s_num == 0)
        flag_track = TRACK_LEFT;
    else if (rpts0s_num < rpts1s_num / 2)
        flag_track = TRACK_RIGHT;
    else if (rpts0s_num / 2 > rpts1s_num)
        flag_track = TRACK_LEFT;
    else if (rpts0s_num < 10 && rpts0s_num < rpts1s_num)
        flag_track = TRACK_RIGHT;
    else if (rpts1s_num < 10 && rpts0s_num > rpts1s_num)
        flag_track = TRACK_LEFT;
    else
        flag_track = TRACK_MIDDLE;

    // 特殊处理
    if (special_w) {  // 施工区
        if (_config->direction_work == 0)
            flag_track = TRACK_LEFT;
        else
            flag_track = TRACK_RIGHT;

        is_curve0 = is_curve1 = true;
        is_straight0 = is_straight1 = false;
    }
    if (special_g) {  // 加油站
        if (_config->direction_gas == 0)
            flag_track = TRACK_LEFT;
        else
            flag_track = TRACK_RIGHT;

        is_curve0 = is_curve1 = true;
        is_straight0 = is_straight1 = false;
    }

    /* ***************************************************************** */
    /* *************************** 目标检测结果 ************************** */
    /* ***************************************************************** */

    // 检测完成判断 (加油站 & 施工区 跳过 防止重复获取检测结果)
    if ((flag_gas <= GAS_DETECTION && flag_work <= WORK_DETECTION) &&
        detection->_is_predicted) {
        _det_result = detection->getResult()->result;

        // 遍历检测结果
        for (size_t i = 0; i < _det_result.size(); i++) {
            DetectionPredictResult r = _det_result[i];

            // 检测结果筛选
            if (r.score < _config->threshold_detection)
                continue;
            if (DetectionLabel(r.type) == label_red)
                continue;

            // 转换到处理图像坐标
            int deX = r.x / 2, deY = r.y / 2;
            int deH = r.height / 2, deW = r.width / 2;

            // 尺寸位置筛选
            if ((deX < 25 || deX + deW > 275) || deY < 40 || (deH < 25 || deW < 25))
                continue;

            // 禁止在元素内判断其他元素
            if (flag_work == WORK_NONE && flag_gas == GAS_NONE &&
                flag_yroad == YROAD_NONE && flag_ramp == RAMP_NONE) {
                switch (DetectionLabel(r.type)) {
                    case label_work:  // 施工区
                        if (flag_work == WORK_NONE && _config->check_work_ == 1) {
                            flag_work = WORK_DETECTION;
                            sprintf(string_log, "WORK_DETECTION");
                            log_i("WORK_DETECTION");
                        }
                        break;

                    case label_gas:  // 加油站
                        if (flag_gas == GAS_NONE && _config->check_gas_ == 1) {
                            flag_gas = GAS_DETECTION;
                            sprintf(string_log, "GAS_DETECTION");
                            log_i("GAS_DETECTION");
                        }
                        break;

                    case label_three:  // 泛行区
                        if (flag_yroad == YROAD_NONE && _config->check_yroad_ == 1) {
                            flag_yroad = YROAD_DETECTION;
                            sprintf(string_log, "YROAD_DETECTION");
                            log_i("YROAD_DETECTION");
                        }
                        break;

                    case label_ramp:  // 坡道
                        if (flag_ramp == RAMP_NONE && _config->check_ramp_ == 1) {
                            flag_ramp = RAMP_DETECTION;
                            sprintf(string_log, "RAMP_DETECTION");
                            log_i("RAMP_DETECTION");
                        }
                        break;

                    default:
                        break;
                }
            }

            if (r.type > 0 && r.type < 8) {
                // 计算偏移
                if (deY < BEGIN_Y + 30 && deY + deH > BEGIN_Y - 30 &&
                    deX < IMAGE_WIDTH - BEGIN_X + 20 && deX + deW > BEGIN_X - 20) {
                    if (deX < begin_x_l)
                        begin_x_l = deX;
                    if (deX + deW > begin_x_r)
                        begin_x_r = deX + deW;
                }

                // 绘制检测结果
                if (_is_result) {
                    MAT_INFO(mat_result, cv::Point(deX, deY), 0.4, "%s - %.2f",
                             detection_labels[r.type].c_str(), r.score);
                    cv::Rect rect(deX, deY, deW, deH);
                    cv::rectangle(mat_result, rect, Scalar(0, 0, 224), 2);
                }
            }
        }
    }

    /* ***************************************************************** */
    /* **************************** 元素检测 **************************** */
    /* ***************************************************************** */

    // 重置识别元素开始标志
    element_identify = 0;

    // 完成一个元素后, 过一段路程再开始检测(识别)新的元素
    if (element_over && encoder.route - element_over_route > 20.0f) {
        element_over = false;
        element_over_route = 0.0f;
        element_begin_id = 0;
        special_w = false;
        special_g = false;
    }

    // 识别元素
    if (!element_over) {
        check_garage();  // 斑马线 - 识别特殊特征

        if (flag_garage == GARAGE_NONE)
            check_work();  // 施工区 - 识别标志

        if (flag_garage == GARAGE_NONE && flag_work == WORK_NONE)
            check_gas();  // 加油站 - 识别标志

        if (flag_garage == GARAGE_NONE && flag_work == WORK_NONE &&
            flag_gas == GAS_NONE)
            check_yroad();  // 泛行区 - 识别标志

        if (flag_garage == GARAGE_NONE && flag_work == WORK_NONE &&
            flag_gas == GAS_NONE && flag_yroad == YROAD_NONE)
            check_ramp();  // 坡道 - 识别标志

        if (_config->check_circle_ == 1 &&
            flag_garage == GARAGE_NONE && flag_work == WORK_NONE &&
            flag_gas == GAS_NONE && flag_yroad == YROAD_NONE &&
            flag_ramp == RAMP_NONE)
            check_circle();  // 环岛路 - 识别角点特征

        if (_config->check_cross_ == 1 &&
            flag_garage == GARAGE_NONE && flag_work == WORK_NONE &&
            flag_gas == GAS_NONE && flag_yroad == YROAD_NONE &&
            flag_ramp == RAMP_NONE && flag_circle <= 2)
            check_cross();  // 十字路 - 识别角点特征
    }

    // 根据检查结果执行模式
    if (flag_garage != GARAGE_NONE) {
        run_garage();
        /* *** *** *** *** *** */
        flag_work = WORK_NONE;
        flag_gas = GAS_NONE;
        flag_yroad = YROAD_NONE;
        flag_ramp = RAMP_NONE;
        flag_circle = CIRCLE_NONE;
        flag_cross = CROSS_NONE;
    } else if (flag_work != WORK_NONE) {
        run_work();
        /* *** *** *** *** *** */
        flag_gas = GAS_NONE;
        flag_yroad = YROAD_NONE;
        flag_ramp = RAMP_NONE;
        flag_circle = CIRCLE_NONE;
        flag_cross = CROSS_NONE;
    } else if (flag_gas != GAS_NONE) {
        run_gas();
        /* *** *** *** *** *** */
        flag_yroad = YROAD_NONE;
        flag_ramp = RAMP_NONE;
        flag_circle = CIRCLE_NONE;
        flag_cross = CROSS_NONE;
    } else if (flag_yroad != YROAD_NONE) {
        run_yroad();
        /* *** *** *** *** *** */
        flag_ramp = RAMP_NONE;
        flag_circle = CIRCLE_NONE;
        flag_cross = CROSS_NONE;
    } else if (flag_ramp != RAMP_NONE) {
        run_ramp();
        /* *** *** *** *** *** */
        flag_circle = CIRCLE_NONE;
        flag_cross = CROSS_NONE;
    } else if (flag_cross != CROSS_NONE) {
        special_w_c = false;
        run_cross();
        /* *** *** *** *** *** */
        flag_circle = CIRCLE_NONE;
    } else if (flag_circle != CIRCLE_NONE) {
        run_circle();
        /* *** *** *** *** *** */
        flag_cross = CROSS_NONE;
    }

    /* ***************************************************************** */
    /* **************************** 中线处理 **************************** */
    /* ***************************************************************** */

    // 加油站 & 施工区 锥桶拟合边线
    if (flag_gas > GAS_FOUND || flag_work > WORK_FOUND) {
        int dist = 0;  // 边线偏移
        if (flag_gas == GAS_RUN || flag_work == WORK_RUN)
            dist = 50;

        if (flag_track == TRACK_LEFT) {
            track_leftline(det_rptss, det_rptss_num, rpts,
                           (int)round(ANGLE_DIST / SAMPLE_DIST),
                           PIXEL_PER_METER * ROAD_WIDTH / 2 + dist);
        } else {
            track_rightline(det_rptss, det_rptss_num, rpts,
                            (int)round(ANGLE_DIST / SAMPLE_DIST),
                            PIXEL_PER_METER * ROAD_WIDTH / 2 + dist);
        }
        rpts_num = det_rptss_num;
        // 透视 -> 原图
        for (int i = 0; i < rpts_num; i++)
            map_perspective(rpts[i][0], rpts[i][1], rpts[i], 1);

    }
    // 十字根据远线
    else if (flag_cross == CROSS_IN) {
        if (flag_track == TRACK_LEFT) {
            track_leftline(far_rpts0s + far_Lpt0_rpts0s_id,
                           far_rpts0s_num - far_Lpt0_rpts0s_id, rpts,
                           (int)round(ANGLE_DIST / SAMPLE_DIST),
                           PIXEL_PER_METER * ROAD_WIDTH / 2);
            rpts_num = far_rpts0s_num - far_Lpt0_rpts0s_id;
        } else {
            track_rightline(far_rpts1s + far_Lpt1_rpts1s_id,
                            far_rpts1s_num - far_Lpt1_rpts1s_id, rpts,
                            (int)round(ANGLE_DIST / SAMPLE_DIST),
                            PIXEL_PER_METER * ROAD_WIDTH / 2);
            rpts_num = far_rpts1s_num - far_Lpt1_rpts1s_id;
        }
        // 透视 -> 原图
        for (int i = 0; i < rpts_num; i++)
            map_perspective(rpts[i][0], rpts[i][1], rpts[i], 1);

    }
    // 泛行区根据逆边线
    else if (flag_yroad > YROAD_FOUND) {
        int id = 0, dist = 30;
        if (flag_yroad == YROAD_OUT) {
            id = yroad_Lpt_id;  // 角点位置
            dist = 0;           // 边线偏移
        }

        if (flag_track == TRACK_RIGHT) {
            track_rightline(
                yroad_rptss + id, yroad_rptss_num - id, rpts,
                (int)round(ANGLE_DIST / SAMPLE_DIST),
                PIXEL_PER_METER * ROAD_WIDTH / 2 + dist);
        } else {
            track_leftline(
                yroad_rptss + id, yroad_rptss_num - id, rpts,
                (int)round(ANGLE_DIST / SAMPLE_DIST),
                PIXEL_PER_METER * ROAD_WIDTH / 2 + dist);
        }
        rpts_num = yroad_rptss_num - id;
        // 透视 -> 原图
        for (int i = 0; i < rpts_num; i++)
            map_perspective(rpts[i][0], rpts[i][1], rpts[i], 1);
    }
    // 正常巡线
    else {
        if (flag_track == TRACK_LEFT) {
            rpts = rptsc0;
            rpts_num = rptsc0_num;
        } else if (flag_track == TRACK_RIGHT) {
            rpts = rptsc1;
            rpts_num = rptsc1_num;
        } else {
            rpts = rptscs;
            rpts_num = rptscs_num;
        }
    }

    /* ***************************************************************** */
    /* **************************** 偏差计算 **************************** */
    /* ***************************************************************** */

    // 特殊元素特殊参数
    // if (flag_gas != GAS_NONE || flag_work != WORK_NONE || special_w_c) {
    //     aim_distance_f = 0.7f;
    //     aim_distance_n = 0.3f;
    //     aim_angle_p_k = _config->ai_p_k;
    //     aim_angle_p = _config->ai_p;
    //     aim_angle_d = _config->ai_d;
    // }
    // 重置参数
    // else {
    aim_distance_f = _config->aim_distance_far;
    aim_distance_n = _config->aim_distance_near;
    aim_angle_p_k = _config->steering_p_k;
    aim_angle_p = _config->steering_p;
    aim_angle_d = _config->steering_d;
    // }

    // 找最近点(起始点中线归一化)
    float min_dist = 1e10;
    int begin_id = -1;
    bool flag_rpts = false;
    for (int i = 0; i < rpts_num; i++) {
        float dx = rpts[i][0] - cx;
        float dy = rpts[i][1] - cy;
        float dist = sqrt(dx * dx + dy * dy);
        if (dist < min_dist) {
            min_dist = dist;
            begin_id = i;
        }
    }
    begin_id = begin_id + element_begin_id >= rpts_num ? begin_id
                                                       : begin_id + element_begin_id;

    // 特殊模式下, 不找最近点 (由于边线会绕一圈回来, 导致最近点为边线最后一个点, 从而中线无法正常生成)
    if (flag_garage == GARAGE_IN_LEFT || flag_garage == GARAGE_IN_RIGHT || flag_cross == CROSS_IN)
        begin_id = 0;

    // 中线有点, 同时最近点不是最后几个点
    if (begin_id >= 0 && rpts_num - begin_id >= 3) {
        // 找到中线
        flag_rpts = true;

        // 归一化中线
        rpts[begin_id][0] = cx;
        rpts[begin_id][1] = cy;
        rptsn_num = sizeof(rptsn) / sizeof(rptsn[0]);
        resample_points(rpts + begin_id, rpts_num - begin_id, rptsn, &rptsn_num,
                        SAMPLE_DIST * PIXEL_PER_METER);

        aim_idx__far = clip(round(aim_distance_f / SAMPLE_DIST), 0, rptsn_num - 1);
        aim_idx_near = clip(round(aim_distance_n / SAMPLE_DIST), 0, rptsn_num - 1);

        std::vector<POINT> v_center(4);  // 三阶贝塞尔曲线
        v_center[0] = {(int)cx, (int)cy};
        v_center[1] = {(int)rptsn[aim_idx_near][0], (int)(IMAGE_HEIGHT * (1 - aim_distance_n))};
        v_center[2] = {(int)rptsn[(int)((aim_idx__far + aim_idx_near) / 2)][0],
                       (int)(IMAGE_HEIGHT * (1 - (aim_distance_f + aim_distance_n) / 2))};
        v_center[3] = {(int)rptsn[aim_idx__far][0], (int)(IMAGE_HEIGHT * (1 - aim_distance_f))};
        bezier_line = bezier(0.03, v_center);

        // 计算远锚点偏差值
        float dx = bezier_line[bezier_line.size() - 1].x - cx;  // rptsn[aim_idx__far][0] - cx;
        float dy = cy - bezier_line[bezier_line.size() - 1].y;  // cy - rptsn[aim_idx__far][1];
        float error_far = (-atan2f(dx, dy) * 180 / PI);
        assert(!isnan(error_far));

        // 计算近锚点偏差值
        float dx_near = bezier_line[bezier_line.size() / 2].x - cx;  // rptsn[aim_idx_near][0] - cx;
        float dy_near = cy - bezier_line[bezier_line.size() / 2].y;  // cy - rptsn[aim_idx_near][1];
        float error_near = (-atan2f(dx_near, dy_near) * 180 / PI);
        assert(!isnan(error_near));

        aim_angle = error_far * 0.9 + error_near * 0.1;
        aim_sigma = sigma(rptsn + aim_idx_near, aim_idx__far - aim_idx_near);
        // aim_sigma = sigma(v_center);

    } else {
        // 中线点过少
        flag_rpts = false;
        aim_angle = aim_angle_last;
        aim_sigma = 100.0f;

        // 加油站丢线
        if (flag_gas != GAS_NONE)
            aim_angle = aim_angle_last;

        // 施工区丢线
        if (flag_work != WORK_NONE)
            aim_angle = aim_angle_last;

        // 环岛内部丢线
        if (flag_circle > 2)
            aim_angle = aim_angle_last * 1.39f;

        // 十字丢线
        if (flag_cross != CROSS_NONE)
            aim_angle = aim_angle_last;

        // 泛行区丢线
        if (flag_yroad != YROAD_NONE)
            aim_angle = aim_angle_last * 1.2f;

        // 斑马线丢线
        if (flag_garage >= GARAGE_PASS_LEFT)
            aim_angle = aim_angle_last;
        else if (flag_garage >= GARAGE_IN_LEFT)
            aim_angle = aim_angle_last * 1.35f;
    }

    /* ***************************************************************** */
    /* **************************** 速度判定 **************************** */
    /* ***************************************************************** */

    if (flag_garage == GARAGE_OUT_LEFT || flag_garage == GARAGE_OUT_RIGHT) {
        aim_speed = _config->speed_garage_out;
        /// 出库 直接给偏差
        if (_config->direction_garage == 0)
            aim_angle = _config->deviation_garage_out;
        else
            aim_angle = 0.0f - _config->deviation_garage_out;
    }  // 出库
    else if (flag_garage == GARAGE_IN_LEFT || flag_garage == GARAGE_IN_RIGHT) {
        aim_speed = _config->speed_garage_in;
        /// 入库 加大角度 P 项
        aim_angle_p += _config->deviation_garage_in_p;
    }  // 入库
    else if (flag_work == WORK_RUN) {
        aim_speed = _config->speed_work;
    }  // 施工区 内
    else if (flag_work != WORK_NONE) {
        aim_speed = (int16_t)_config->speed_work * 0.9f;
    }  // 施工区
    else if (flag_gas == GAS_RUN) {
        aim_speed = _config->speed_gas;
    }  // 加油站 内
    else if (flag_gas != GAS_NONE) {
        aim_speed = (int16_t)_config->speed_gas * 0.9f;
    }  // 加油站
    else if (flag_yroad == YROAD_IN) {
        aim_speed = _config->speed_yroad;
    }  // 泛行区 内
    else if (flag_yroad == YROAD_OUT) {
        aim_speed = _config->speed_steady;
    }  // 泛行区 出
    else if (flag_ramp == RAMP_UP) {
        aim_speed = _config->speed_ramp_up;
    }  // 坡道 上
    else if (flag_ramp == RAMP_DOWN) {
        aim_speed = _config->speed_ramp_down;
    }  // 坡道 下
    else if (flag_circle > 0 && flag_circle < 7) {
        aim_speed = _config->speed_circle;
    }  // 出入环岛
    else if (flag_circle > 6) {
        aim_speed = _config->speed_base;
    }  // 环岛内部加速
    else if (flag_cross != CROSS_NONE) {
        aim_speed = _config->speed_cross;
    }  // 十字速度
    else {
        // 根据偏差方差加减速
        if (abs(aim_sigma) < 10.0) {
            aim_speed_shift += 10;
        } else {
            aim_speed_shift -= speed_diff;
        }

        // 速度限幅
        aim_speed_shift = clip(aim_speed_shift, _config->speed_base, _config->speed_up);
        aim_speed = aim_speed_shift;
    }

    // 停车
    if (flag_garage == GARAGE_STOP) {
        aim_speed = 0;
        aim_angle = 0;
    }

    // 特殊处理
    if (special_w || special_w_c)  // 施工区
        aim_speed = (int16_t)_config->speed_work * 0.85f;
    if (special_g)  // 加油站
        aim_speed = (int16_t)_config->speed_gas * 0.85f;

    /* ***************************************************************** */
    /* **************************** 运行控制 **************************** */
    /* ***************************************************************** */

    // 偏差限幅
    aim_angle = aim_angle > 500.0f ? 500.0f : aim_angle < -500.0f ? -500.0f
                                                                  : aim_angle;

    // 偏差滤波
    float aim_angle_filter = filter(aim_angle);
    aim_angle_last = aim_angle_filter;

    // 动态 P 项, 出入库禁止
    if ((flag_garage != GARAGE_OUT_LEFT && flag_garage != GARAGE_OUT_RIGHT) &&
        (flag_garage != GARAGE_IN_LEFT && flag_garage != GARAGE_IN_RIGHT) &&
        ((is_curve0 && flag_track == TRACK_LEFT) || (is_curve1 && flag_track == TRACK_RIGHT))) {
        aim_angle_p += fabs(aim_angle_filter) * aim_angle_p_k;
        aim_angle_p = aim_angle_p > _config->steering_p * 3.0f ? _config->steering_p * 3.0f
                                                               : aim_angle_p;
    }

    // 计算舵机 PID
    int aim_angle_pwm = 0;
    aim_angle_pwm = (int)(pid_realize_a(aim_angle_filter, 0.0f, aim_angle_p, aim_angle_d) + 0.5f);
    aim_angle_pwm = 0 - clip(aim_angle_pwm, -500, 500);

    // 赋值数据包
    payload.tSpeed = aim_speed;
    payload.tAngle = (int16_t)aim_angle_pwm;
    payload.element = element_identify;

    /* ***************************************************************** */
    /* **************************** 绘制结果 **************************** */
    /* ***************************************************************** */

    if (_is_result) {
        std::string track_label;
        if (flag_track == TRACK_LEFT)
            track_label = "track: LEFT";
        else if (flag_track == TRACK_RIGHT)
            track_label = "track: RIGHT";
        else
            track_label = "track: MIDDLE";

        MAT_INFO(mat_result, cv::Point(5, 25), 0.4, "%s", track_label.c_str());
        MAT_INFO(mat_result, cv::Point(5, 40), 0.4, "speed: %d", aim_speed);
        MAT_INFO(mat_result, cv::Point(5, 55), 0.4, "angle: %.2f", aim_angle_filter);
        MAT_INFO(mat_result, cv::Point(5, 70), 0.4, "stg_p: %.4f", aim_angle_p);
        MAT_INFO(mat_result, cv::Point(120, 10), 0.4, "[%c][%c][%c]",
                 is_straight0 ? '^' : (is_curve0 ? '<' : ' '),
                 flag_rpts ? '^' : ' ',
                 is_straight1 ? '^' : (is_curve1 ? '>' : ' '));
        MAT_INFO(mat_result, cv::Point(5, 100), 0.4, "sigma: %.2f", aim_sigma);
        MAT_INFO(mat_result, cv::Point(5, 115), 0.4, "mea-0: %.2f", mea_0);
        MAT_INFO(mat_result, cv::Point(5, 130), 0.4, "mea-1: %.2f", mea_1);
        MAT_INFO(mat_result, cv::Point(5, 155), 0.4, "speed: %.2fm/s", encoder.speed);
        MAT_INFO(mat_result, cv::Point(5, 170), 0.4, "route: %.0fcm", encoder.route);
        MAT_INFO(mat_result, cv::Point(5, 185), 0.4, "log: %-20s", string_log);

        // 十字 ----------------------------------------------------
        if (flag_cross == CROSS_IN) {
            // 角点
            if (far_Lpt0_found) {  // 绿色
                map_perspective(far_rpts0s[far_Lpt0_rpts0s_id][0], far_rpts0s[far_Lpt0_rpts0s_id][1], trans, 1);
                cv::circle(mat_result, cv::Point2f(trans[0], trans[1]), 3, cv::Scalar(0, 255, 0), 2, 8);
            }
            if (far_Lpt1_found) {  // 绿色
                map_perspective(far_rpts1s[far_Lpt1_rpts1s_id][0], far_rpts1s[far_Lpt1_rpts1s_id][1], trans, 1);
                cv::circle(mat_result, cv::Point2f(trans[0], trans[1]), 3, cv::Scalar(0, 255, 0), 2, 8);
            }

            // 远线
            for (int a = 0; a < far_rpts0s_num; a++) {  // 边线等距采样 左 十字远线
                map_perspective(far_rpts0s[a][0], far_rpts0s[a][1], trans, 1);
                if ((int)trans[1] >= 0 && (int)trans[1] < mat_result.rows &&
                    (int)trans[0] >= 0 && (int)trans[0] < mat_result.cols) {
                    mat_result.at<cv::Vec3b>((int)trans[1], (int)trans[0])[0] = 0;
                    mat_result.at<cv::Vec3b>((int)trans[1], (int)trans[0])[1] = 238;
                    mat_result.at<cv::Vec3b>((int)trans[1], (int)trans[0])[2] = 238;
                }
            }
            for (int a = 0; a < far_rpts1s_num; a++) {  // 边线等距采样 右 十字远线
                map_perspective(far_rpts1s[a][0], far_rpts1s[a][1], trans, 1);
                if ((int)trans[1] >= 0 && (int)trans[1] < mat_result.rows &&
                    (int)trans[0] >= 0 && (int)trans[0] < mat_result.cols) {
                    mat_result.at<cv::Vec3b>((int)trans[1], (int)trans[0])[0] = 238;
                    mat_result.at<cv::Vec3b>((int)trans[1], (int)trans[0])[1] = 238;
                    mat_result.at<cv::Vec3b>((int)trans[1], (int)trans[0])[2] = 0;
                }
            }
        }
        // 泛行区 --------------------------------------------------
        else if (flag_yroad > YROAD_FOUND) {
            // 角点
            if (yroad_Lpt_found) {  // 绿色
                map_perspective(yroad_rptss[yroad_Lpt_id][0], yroad_rptss[yroad_Lpt_id][1], trans, 1);
                cv::circle(mat_result, cv::Point2f(trans[0], trans[1]), 3, cv::Scalar(0, 255, 0), 2, 8);
            }

            // 边线
            for (int a = 0; a < yroad_rptss_num; a++) {  // 边线等距采样 泛行区边线
                map_perspective(yroad_rptss[a][0], yroad_rptss[a][1], trans, 1);
                if ((int)trans[1] >= 0 && (int)trans[1] < mat_result.rows &&
                    (int)trans[0] >= 0 && (int)trans[0] < mat_result.cols) {
                    mat_result.at<cv::Vec3b>((int)trans[1], (int)trans[0])[0] = 0;
                    mat_result.at<cv::Vec3b>((int)trans[1], (int)trans[0])[1] = 238;
                    mat_result.at<cv::Vec3b>((int)trans[1], (int)trans[0])[2] = 238;
                }
            }
        }
        // 其他情况 ------------------------------------------------
        else {
            // 施工区 & 加油站---------------------------------------
            if (flag_gas != GAS_NONE || flag_work != WORK_NONE) {
                // 锥桶
                for (auto p : edge_det)
                    cv::circle(mat_result, cv::Point(p.y, p.x), 2, cv::Scalar(255, 255, 255), 2);

                // 边线
                for (auto p : edge_left)
                    cv::circle(mat_result, cv::Point(p.y, p.x), 1, cv::Scalar(0, 238, 238), -1);
                for (auto p : edge_right)
                    cv::circle(mat_result, cv::Point(p.y, p.x), 1, cv::Scalar(238, 238, 0), -1);
            }
            // 正常巡线 --------------------------------------------
            else {
                // 角点
                if (Lpt0_found) {  // 绿色
                    map_perspective(rpts0s[Lpt0_rpts0s_id][0], rpts0s[Lpt0_rpts0s_id][1], trans, 1);
                    cv::circle(mat_result, cv::Point2f(trans[0], trans[1]), 3, cv::Scalar(0, 255, 0), 2, 8);
                }
                if (Lpt1_found) {  // 绿色
                    map_perspective(rpts1s[Lpt1_rpts1s_id][0], rpts1s[Lpt1_rpts1s_id][1], trans, 1);
                    cv::circle(mat_result, cv::Point2f(trans[0], trans[1]), 3, cv::Scalar(0, 255, 0), 2, 8);
                }

                // 边线
                for (int a = 0; a < rpts0s_num; a++) {  // 边线等距采样 左
                    map_perspective(rpts0s[a][0], rpts0s[a][1], trans, 1);
                    if ((int)trans[1] >= 0 && (int)trans[1] < mat_result.rows &&
                        (int)trans[0] >= 0 && (int)trans[0] < mat_result.cols) {
                        mat_result.at<cv::Vec3b>((int)trans[1], (int)trans[0])[0] = 0;
                        mat_result.at<cv::Vec3b>((int)trans[1], (int)trans[0])[1] = 238;
                        mat_result.at<cv::Vec3b>((int)trans[1], (int)trans[0])[2] = 238;
                    }
                }
                for (int a = 0; a < rpts1s_num; a++) {  // 边线等距采样 右
                    map_perspective(rpts1s[a][0], rpts1s[a][1], trans, 1);
                    if ((int)trans[1] >= 0 && (int)trans[1] < mat_result.rows &&
                        (int)trans[0] >= 0 && (int)trans[0] < mat_result.cols) {
                        mat_result.at<cv::Vec3b>((int)trans[1], (int)trans[0])[0] = 238;
                        mat_result.at<cv::Vec3b>((int)trans[1], (int)trans[0])[1] = 238;
                        mat_result.at<cv::Vec3b>((int)trans[1], (int)trans[0])[2] = 0;
                    }
                }
            }

            // 中线
            for (int a = 0; a < rptscs_num; a++) {  // 原图中线
                if ((int)rptscs[a][1] >= 0 && (int)rptscs[a][1] < mat_result.rows &&
                    (int)rptscs[a][0] >= 0 && (int)rptscs[a][0] < mat_result.cols) {
                    mat_result.at<cv::Vec3b>((int)rptscs[a][1], (int)rptscs[a][0])[0] = 0;
                    mat_result.at<cv::Vec3b>((int)rptscs[a][1], (int)rptscs[a][0])[1] = 0;
                    mat_result.at<cv::Vec3b>((int)rptscs[a][1], (int)rptscs[a][0])[2] = 0;
                }
            }
            for (int a = 0; a < rptsc0_num; a++) {  // 左中线
                if ((int)rptsc0[a][1] >= 0 && (int)rptsc0[a][1] < mat_result.rows &&
                    (int)rptsc0[a][0] >= 0 && (int)rptsc0[a][0] < mat_result.cols) {
                    mat_result.at<cv::Vec3b>((int)rptsc0[a][1], (int)rptsc0[a][0])[0] = 0;
                    mat_result.at<cv::Vec3b>((int)rptsc0[a][1], (int)rptsc0[a][0])[1] = 238;
                    mat_result.at<cv::Vec3b>((int)rptsc0[a][1], (int)rptsc0[a][0])[2] = 238;
                }
            }
            for (int a = 0; a < rptsc1_num; a++) {  // 右中线
                if ((int)rptsc1[a][1] >= 0 && (int)rptsc1[a][1] < mat_result.rows &&
                    (int)rptsc1[a][0] >= 0 && (int)rptsc1[a][0] < mat_result.cols) {
                    mat_result.at<cv::Vec3b>((int)rptsc1[a][1], (int)rptsc1[a][0])[0] = 238;
                    mat_result.at<cv::Vec3b>((int)rptsc1[a][1], (int)rptsc1[a][0])[1] = 238;
                    mat_result.at<cv::Vec3b>((int)rptsc1[a][1], (int)rptsc1[a][0])[2] = 0;
                }
            }
        }

        if (flag_rpts) {
            // 贝塞尔曲线
            for (auto p : bezier_line)
                circle(mat_result, Point(p.x, p.y), 1, Scalar(0, 0, 255), -1);

            // 归一化中线
            for (int a = 0; a < rptsn_num; a++) {
                if ((int)rptsn[a][1] >= 0 && (int)rptsn[a][1] < mat_result.rows &&
                    (int)rptsn[a][0] >= 0 && (int)rptsn[a][0] < mat_result.cols) {
                    mat_result.at<cv::Vec3b>((int)rptsn[a][1], (int)rptsn[a][0])[0] = 0;
                    mat_result.at<cv::Vec3b>((int)rptsn[a][1], (int)rptsn[a][0])[1] = 0;
                    mat_result.at<cv::Vec3b>((int)rptsn[a][1], (int)rptsn[a][0])[2] = 255;
                }
            }

            // 预瞄点
            /*
            cv::circle(mat_result, cv::Point2f(rptsn[aim_idx__far][0], rptsn[aim_idx__far][1]),
                       3, cv::Scalar(0, 0, 255), 2, 8);
            cv::circle(mat_result, cv::Point2f(rptsn[aim_idx_near][0], rptsn[aim_idx_near][1]),
                       3, cv::Scalar(0, 0, 255), 2, 8);
            */
        }
    }
}

/* ******************************************************************* */
/* ******************************************************************* */
/* ******************************************************************* */