#include "_sideline.hpp"

/* ********************************************************************** */
/* ****************************** 透视变换 ******************************* */
/* ******************* (0:原图 -> 俯视, 1:俯视 -> 原图) ******************** */

// 原图 -> 俯视  透视矩阵
cv::Mat warpMatrix =
    (cv::Mat_<float>(3, 3) << 4.830985915493104, 7.481690140845294,
     -559.9154929577662, 1.3374922084896e-14, 14.74647887323986,
     -725.1690140845323, 7.62235112648736e-17, 0.05070422535211418, 1);

// 俯视 -> 原图  透视矩阵
cv::Mat warpMatrixT =
    (cv::Mat_<float>(3, 3) << 0.7231273024969306, -0.503533906399236,
     39.74299358711961, 4.254746920719163e-16, 0.06781279847182463,
     49.17574021012407, 1.303269874835107e-18, -0.003438395415472779, 1);

// 透视变换 (0:原图 -> 俯视, 1:俯视 -> 原图)
void map_perspective(float x, float y, float loc[2], uint8_t mode) {
    float xx, yy, zz;

    if (mode == 0) {
        zz = warpMatrix.at<float>(2, 0) * x +
             warpMatrix.at<float>(2, 1) * y +
             warpMatrix.at<float>(2, 2);
        xx = (warpMatrix.at<float>(0, 0) * x +
              warpMatrix.at<float>(0, 1) * y +
              warpMatrix.at<float>(0, 2)) /
             zz;
        yy = (warpMatrix.at<float>(1, 0) * x +
              warpMatrix.at<float>(1, 1) * y +
              warpMatrix.at<float>(1, 2)) /
             zz;

        loc[0] = xx;
        loc[1] = yy;
    } else {
        zz = warpMatrixT.at<float>(2, 0) * x +
             warpMatrixT.at<float>(2, 1) * y +
             warpMatrixT.at<float>(2, 2);
        xx = (warpMatrixT.at<float>(0, 0) * x +
              warpMatrixT.at<float>(0, 1) * y +
              warpMatrixT.at<float>(0, 2)) /
             zz;
        yy = (warpMatrixT.at<float>(1, 0) * x +
              warpMatrixT.at<float>(1, 1) * y +
              warpMatrixT.at<float>(1, 2)) /
             zz;

        loc[0] = xx;
        loc[1] = yy;
    }
}

/* ********************************************************************** */
/* ******************************* 控制类 ******************************** */
/* ********************************************************************** */

// 位置式 PID 角度外环
float pid_realize_a(float actual, float set, float _p, float _d) {
    static float last_error = 0.0f;
    static float last_out_d = 0.0f;
    // static float last_actual = 0.0f;
    // static float derivative = 0.0f;

    /* 当前误差 */
    float error = set - actual;

    /* 微分先行 */
    /*
    float temp = 0.618f * _d + _p;
    float c3 = _d / temp;
    float c2 = (_d + _p) / temp;
    float c1 = 0.618f * c3;
    derivative = c1 * derivative + c2 * actual - c3 * last_actual;
    */

    /* 不完全微分 */
    float out_d = _d * 0.8f * (error - last_error) + 0.2f * last_out_d;
    // float out_d = 0.8f * derivative + 0.2f * last_out_d;

    /* 实际输出 */
    float output = _p * error + out_d;

    /* 更新参数 */
    last_error = error;
    last_out_d = out_d;
    // last_actual = actual;

    return output;
}

// 位置式 PID 角速度内环
int pid_realize_o(int actual, int set, float _p, float _d) {
    static int last_error = 0;

    /* 当前误差 */
    int error = set - actual;

    /* 实际输出 */
    int output = (int)(_p * error + _d * (error - last_error) + 0.5f);

    /* 更新参数 */
    last_error = error;

    return output;
}

// 偏差滑动平均滤波
float filter(float value) {
    static float filter_buf[3] = {0};

    filter_buf[2] = filter_buf[1];
    filter_buf[1] = filter_buf[0];
    filter_buf[0] = value;

    return (filter_buf[2] + filter_buf[1] + filter_buf[0]) / 3.0f;
}

/* *********************************************************************** */
/* ******************************* 特殊处理 ******************************* */
/* *********************************************************************** */

// 赛道点集的方差计算
double sigma(float pts[][2], int num) {
    if (num < 1)
        return 0;

    double sum = 0;
    for (int i = 0; i < num; i++) sum += pts[i][0];

    double aver = (double)sum / num;
    double sigma = 0;

    for (int i = 0; i < num; i++)
        sigma += (pts[i][0] - aver) * (pts[i][0] - aver);
    sigma /= (double)num;

    return sigma;
}
double sigma(std::vector<POINT> vec) {
    if (vec.size() < 1)
        return 0;

    double sum = 0;
    for (auto p : vec) sum += p.x;

    double aver = (double)sum / vec.size();
    double sigma = 0;

    for (auto p : vec)
        sigma += (p.x - aver) * (p.x - aver);
    sigma /= (double)vec.size();

    return sigma;
}

// 阶乘计算
int factorial(int x) {
    int f = 1;
    for (int i = 1; i <= x; i++) f *= i;

    return f;
}

// 点到直线的距离计算
double dis_point_line(POINT a, POINT b, POINT p) {
    double ab_distance = sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
    double ap_distance = sqrt((a.x - p.x) * (a.x - p.x) + (a.y - p.y) * (a.y - p.y));
    double bp_distance = sqrt((p.x - b.x) * (p.x - b.x) + (p.y - b.y) * (p.y - b.y));

    double half = (ab_distance + ap_distance + bp_distance) / 2;
    double area = sqrt(half * (half - ab_distance) * (half - ap_distance) * (half - bp_distance));

    return (2 * area / ab_distance);
}

// 两点间的距离计算
double dis_point_point(POINT a, POINT b) {
    double s = sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
    return s;
}

// 直线拟合 (返回平均绝对误差)
float fit_line(float pts[][2], int num, int cut_h) {
    if (num != 0) {
        std::vector<cv::Point> points;
        cv::Vec4f line_para;
        float k, b, mea = 0.0f;
        float trans[2];
        int y_counter = 0;

        for (int i = 0; i < num; i++, y_counter++) {
            map_perspective(pts[i][0], pts[i][1], trans, 1);
            if (trans[1] < cut_h)
                break;

            points.push_back(cv::Point(trans[0], trans[1]));
        }

        cv::fitLine(points, line_para, cv::DIST_L2, 0, 1e-2, 1e-2);

        k = line_para[1] / line_para[0];
        b = line_para[3] - k * line_para[2];

        for (int i = 0; i < y_counter; i++)
            mea += fabs(k * points[i].x + b - points[i].y);

        return (float)(mea / y_counter);
    }

    return 100.0f;
}

// 贝塞尔曲线拟合
std::vector<POINT> bezier(double dt, std::vector<POINT> input) {
    std::vector<POINT> output;
    double t = 0;
    while (t <= 1) {
        POINT p;
        double x_sum = 0.0;
        double y_sum = 0.0;
        int i = 0;
        int n = input.size() - 1;
        while (i <= n) {
            double k = factorial(n) / (factorial(i) * factorial(n - i)) * pow(t, i) *
                       pow(1 - t, n - i);
            x_sum += k * input[i].x;
            y_sum += k * input[i].y;
            i++;
        }
        p.x = x_sum;
        p.y = y_sum;
        output.push_back(p);
        t += dt;
    }
    return output;
}

/* *********************************************************************** */
/* **************************** 特殊 AI 元素处理 *************************** */
/* ***************************** 加油站 & 施工区 *************************** */

bool sort_y_up(POINT a, POINT b) {  // 升序
    return (a.y < b.y);
}
bool sort_y_down(POINT a, POINT b) {  // 升序
    return (a.y > b.y);
}
bool sort_x_up(POINT a, POINT b) {  // 升序
    return (a.x < b.x);
}
bool sort_x_down(POINT a, POINT b) {  // 降序
    return (a.x > b.x);
}

// 从AI检测结果中检索锥桶坐标集合
std::vector<POINT> search_targets(std::vector<DetectionPredictResult> predict) {
    std::vector<POINT> cones;
    for (auto p : predict) {
        if (ImageProcess::DetectionLabel(p.type) == ImageProcess::label_red) {
            cones.push_back(POINT(p.y / 2 + p.height / 4,
                                  p.x / 2 + p.width / 4));
        }

        else if (ImageProcess::DetectionLabel(p.type) == ImageProcess::label_wone) {
            gas_which = 1;
            /*
            if (gas_which_is == false) {
                gas_which_is = true;
                if (gas_which_last == 0)
                    gas_which = 1;
                else
                    gas_which = 2;
            }
            */
        }

        else if (ImageProcess::DetectionLabel(p.type) == ImageProcess::label_wtwo) {
            gas_which = 2;
            /*
            if (gas_which_is == false) {
                gas_which_is = true;
                if (gas_which_last == 0)
                    gas_which = 2;
                else
                    gas_which = 1;
            }
            */
        }
    }

    return cones;
}

// 搜索距离赛道左边缘最近的红色色块 (红色锥桶)
POINT search_left_near(std::vector<POINT> pointsEdge, cv::Mat matt) {
    for (size_t i = 0; i < pointsEdge.size(); i++)
        for (int y = pointsEdge[i].y - 25; y < pointsEdge[i].y; y++)
            if ((y >= 0) && (y <= IMAGE_WIDTH - 1))
                if (AT_IMAGE(matt, y, pointsEdge[i].x) > 0)
                    return POINT(pointsEdge[i].x, y);

    return POINT(0, 0);
}

// 搜索距离赛道右边缘最近的红色色块 (红色锥桶)
POINT search_right_near(std::vector<POINT> pointsEdge, cv::Mat matt) {
    for (size_t i = 0; i < pointsEdge.size(); i++)
        for (int y = pointsEdge[i].y + 25; y > pointsEdge[i].y; y--)
            if ((y >= 0) && (y <= IMAGE_WIDTH - 1))
                if (AT_IMAGE(matt, y, pointsEdge[i].x) > 0)
                    return POINT(pointsEdge[i].x, y);

    return POINT(0, 0);
}

// 搜索左方的锥桶集合 ( IMAGE_WIDTH / 3 )
std::vector<POINT> search_left_cones(std::vector<POINT> pointsCone) {
    std::vector<POINT> points;

    for (auto p : pointsCone)
        if ((p.y < IMAGE_WIDTH / 3) && (p.x > 30) && (p.x < IMAGE_HEIGHT - 50))
            points.push_back(p);

    if (points.size() >= 2)
        std::sort(points.begin(), points.end(), sort_y_up);  // 纵坐标升序排序

    return points;
}

// 搜索右方的锥桶集合 ( IMAGE_WIDTH * 2 / 3 )
std::vector<POINT> search_right_cones(std::vector<POINT> pointsCone) {
    std::vector<POINT> points;

    for (auto p : pointsCone)
        if ((p.y > IMAGE_WIDTH * 2 / 3) && (p.x > 30) && (p.x < IMAGE_HEIGHT - 50))
            points.push_back(p);

    if (points.size() >= 2)
        std::sort(points.begin(), points.end(), sort_y_down);  // 纵坐标降序排序

    return points;
}

// 搜索左上方的锥桶集合 (除了右下角)
std::vector<POINT> search_leftUp_cones(std::vector<POINT> pointsCone) {
    std::vector<POINT> points;

    for (auto p : pointsCone) {
        if (p.x > 30) {
            if (p.x > IMAGE_HEIGHT / 2 && p.y > IMAGE_WIDTH / 2)
                continue;

            points.push_back(p);
        }
    }

    if (points.size() >= 2)
        std::sort(points.begin(), points.end(), sort_y_up);  // 纵坐标升序排序

    return points;
}

// 搜索右上方的锥桶集合 (除了左下角)
std::vector<POINT> search_rightUp_cones(std::vector<POINT> pointsCone) {
    std::vector<POINT> points;

    for (auto p : pointsCone) {
        if (p.x > 30) {
            if (p.x > IMAGE_HEIGHT / 2 && p.y < IMAGE_WIDTH / 2)
                continue;

            points.push_back(p);
        }
    }

    if (points.size() >= 2)
        std::sort(points.begin(), points.end(), sort_y_down);  // 纵坐标升序排序

    return points;
}

// 搜索右上方的锥桶
POINT search_rightUp_cone(std::vector<POINT> pointsCone) {
    POINT point(0, 0);
    POINT max_point(IMAGE_HEIGHT - 1, 0);
    double min_dis = 99999.0f;

    for (auto p : pointsCone) {
        if (p.x < 30)
            continue;

        if (p.x < max_point.x)
            max_point.x = p.x;
        if (p.y > max_point.y)
            max_point.y = p.y;
    }

    for (auto p : pointsCone) {
        if (abs(max_point.x - p.x) >= 40 || abs(max_point.y - p.y) >= 40)
            continue;

        double dis = dis_point_point(p, max_point);
        if (dis < min_dis) {
            min_dis = dis;
            point = p;
        }
    }

    if (point.x == 0 && point.y == 0 && pointsCone.size() > 0)
        point = max_point;

    return point;
}

// 搜索左上方的锥桶
POINT search_leftUp_cone(std::vector<POINT> pointsCone) {
    POINT point(0, 0);
    POINT max_point(IMAGE_HEIGHT - 1, IMAGE_WIDTH - 1);
    double min_dis = 99999.0f;

    for (auto p : pointsCone) {
        if (p.x < 30)
            continue;

        if (p.x < max_point.x)
            max_point.x = p.x;
        if (p.y < max_point.y)
            max_point.y = p.y;
    }

    for (auto p : pointsCone) {
        if (abs(max_point.x - p.x) >= 40 || abs(max_point.y - p.y) >= 40)
            continue;

        double dis = dis_point_point(p, max_point);
        if (dis < min_dis) {
            min_dis = dis;
            point = p;
        }
    }

    if (point.x == 0 && point.y == 0 && pointsCone.size() > 0)
        point = max_point;

    return point;
}

// 搜索右下方的锥桶
POINT search_rightDown_cone(std::vector<POINT> pointsCone) {
    POINT point(0, 0);

    for (auto p : pointsCone)
        if (p.y > (IMAGE_WIDTH / 2 - 10) && p.x > point.x)
            point = p;

    return point;
}

// 搜索左下方的锥桶
POINT search_leftDown_cone(std::vector<POINT> pointsCone) {
    POINT point(0, 0);

    for (auto p : pointsCone)
        if (p.y < (IMAGE_WIDTH / 2 + 10) && p.x > point.x)
            point = p;

    return point;
}

/* *********************************************************************** */
/* ******************************* 边线处理 ******************************* */
/* *********************************************************************** */

// 限制范围
int clip(int x, int low, int up) {
    return x > up ? up : x < low ? low
                                 : x;
}

/* 前进方向定义：
 *   0
 * 3   1
 *   2
 */
const int dir_front[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
const int dir_frontleft[4][2] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};
const int dir_frontright[4][2] = {{1, -1}, {1, 1}, {-1, 1}, {-1, -1}};

// 左手迷宫巡线
void findline_lefthand_adaptive(cv::Mat img, int block_size, int clip_value,
                                int x, int y, int pts[][2], int *num) {
    assert(num && *num >= 0);
    assert(block_size > 1 && block_size % 2 == 1);

    int half = block_size / 2;
    int step = 0, dir = 0, turn = 0;

    while ((step < *num) && (half <= x) && (x <= img.cols - half - 1) &&
           (half <= y) && (y <= img.rows - half - 1) && (turn < 4)) {
        int local_thres = 0;
        for (int dy = -half; dy <= half; dy++) {
            for (int dx = -half; dx <= half; dx++) {
                local_thres += AT_IMAGE(img, x + dx, y + dy);
            }
        }
        local_thres /= block_size * block_size;
        local_thres -= clip_value;

        int front_value =
            AT_IMAGE(img, x + dir_front[dir][0], y + dir_front[dir][1]);
        int frontleft_value =
            AT_IMAGE(img, x + dir_frontleft[dir][0], y + dir_frontleft[dir][1]);
        if (front_value < local_thres) {
            dir = (dir + 1) % 4;
            turn++;
        } else if (frontleft_value < local_thres) {
            x += dir_front[dir][0];
            y += dir_front[dir][1];
            pts[step][0] = x;
            pts[step][1] = y;
            step++;
            turn = 0;
        } else {
            x += dir_frontleft[dir][0];
            y += dir_frontleft[dir][1];
            dir = (dir + 3) % 4;
            pts[step][0] = x;
            pts[step][1] = y;
            step++;
            turn = 0;
        }
    }
    *num = step;
}

// 右手迷宫巡线
void findline_righthand_adaptive(cv::Mat img, int block_size, int clip_value,
                                 int x, int y, int pts[][2], int *num) {
    assert(num && *num >= 0);
    assert(block_size > 1 && block_size % 2 == 1);

    int half = block_size / 2;
    int step = 0, dir = 0, turn = 0;

    while ((step < *num) && (half <= x) && (x <= img.cols - half - 1) &&
           (half <= y) && (y <= img.rows - half - 1) && (turn < 4)) {
        int local_thres = 0;
        for (int dy = -half; dy <= half; dy++) {
            for (int dx = -half; dx <= half; dx++) {
                local_thres += AT_IMAGE(img, x + dx, y + dy);
            }
        }
        local_thres /= block_size * block_size;
        local_thres -= clip_value;

        int front_value =
            AT_IMAGE(img, x + dir_front[dir][0], y + dir_front[dir][1]);
        int frontright_value =
            AT_IMAGE(img, x + dir_frontright[dir][0], y + dir_frontright[dir][1]);
        if (front_value < local_thres) {
            dir = (dir + 3) % 4;
            turn++;
        } else if (frontright_value < local_thres) {
            x += dir_front[dir][0];
            y += dir_front[dir][1];
            pts[step][0] = x;
            pts[step][1] = y;
            step++;
            turn = 0;
        } else {
            x += dir_frontright[dir][0];
            y += dir_frontright[dir][1];
            dir = (dir + 1) % 4;
            pts[step][0] = x;
            pts[step][1] = y;
            step++;
            turn = 0;
        }
    }
    *num = step;
}

// 点集三角滤波
void blur_points(float pts_in[][2], int num, float pts_out[][2], int kernel) {
    assert(kernel % 2 == 1);

    int half = kernel / 2;
    for (int i = 0; i < num; i++) {
        pts_out[i][0] = pts_out[i][1] = 0;
        for (int j = -half; j <= half; j++) {
            pts_out[i][0] += pts_in[clip(i + j, 0, num - 1)][0] * (half + 1 - abs(j));
            pts_out[i][1] += pts_in[clip(i + j, 0, num - 1)][1] * (half + 1 - abs(j));
        }
        pts_out[i][0] /= (2 * half + 2) * (half + 1) / 2;
        pts_out[i][1] /= (2 * half + 2) * (half + 1) / 2;
    }
}

// 点集等距采样  使走过的采样前折线段的距离为`dist`
void resample_points(float pts_in[][2], int num1, float pts_out[][2], int *num2,
                     float dist) {
    int remain = 0, len = 0;
    for (int i = 0; i < num1 - 1 && len < *num2; i++) {
        float x0 = pts_in[i][0];
        float y0 = pts_in[i][1];
        float dx = pts_in[i + 1][0] - x0;
        float dy = pts_in[i + 1][1] - y0;
        float dn = sqrt(dx * dx + dy * dy);
        dx /= dn;
        dy /= dn;

        while (remain < dn && len < *num2) {
            x0 += dx * remain;
            pts_out[len][0] = x0;
            y0 += dy * remain;
            pts_out[len][1] = y0;

            len++;
            dn -= remain;
            remain = dist;
        }
        remain -= dn;
    }
    *num2 = len;
}

// 点集局部角度变化率
void local_angle_points(float pts_in[][2], int num, float angle_out[],
                        int dist) {
    for (int i = 0; i < num; i++) {
        if (i <= 0 || i >= num - 1) {
            angle_out[i] = 0;
            continue;
        }
        float dx1 = pts_in[i][0] - pts_in[clip(i - dist, 0, num - 1)][0];
        float dy1 = pts_in[i][1] - pts_in[clip(i - dist, 0, num - 1)][1];
        float dn1 = sqrtf(dx1 * dx1 + dy1 * dy1);
        float dx2 = pts_in[clip(i + dist, 0, num - 1)][0] - pts_in[i][0];
        float dy2 = pts_in[clip(i + dist, 0, num - 1)][1] - pts_in[i][1];
        float dn2 = sqrtf(dx2 * dx2 + dy2 * dy2);
        float c1 = dx1 / dn1;
        float s1 = dy1 / dn1;
        float c2 = dx2 / dn2;
        float s2 = dy2 / dn2;
        angle_out[i] = atan2f(c1 * s2 - c2 * s1, c2 * c1 + s2 * s1);
    }
}

// 角度变化率非极大抑制
void nms_angle(float angle_in[], int num, float angle_out[], int kernel) {
    assert(kernel % 2 == 1);

    int half = kernel / 2;
    for (int i = 0; i < num; i++) {
        angle_out[i] = angle_in[i];
        for (int j = -half; j <= half; j++) {
            if (fabs(angle_in[clip(i + j, 0, num - 1)]) > fabs(angle_out[i])) {
                angle_out[i] = 0;
                break;
            }
        }
    }
}

// 左边线跟踪中线
void track_leftline(float pts_in[][2], int num, float pts_out[][2],
                    int approx_num, float dist) {
    for (int i = 0; i < num; i++) {
        float dx = pts_in[clip(i + approx_num, 0, num - 1)][0] -
                   pts_in[clip(i - approx_num, 0, num - 1)][0];
        float dy = pts_in[clip(i + approx_num, 0, num - 1)][1] -
                   pts_in[clip(i - approx_num, 0, num - 1)][1];
        float dn = sqrt(dx * dx + dy * dy);

        dx /= dn;
        dy /= dn;

        pts_out[i][0] = pts_in[i][0] - dy * dist;
        pts_out[i][1] = pts_in[i][1] + dx * dist;
    }
}

// 右边线跟踪中线
void track_rightline(float pts_in[][2], int num, float pts_out[][2],
                     int approx_num, float dist) {
    for (int i = 0; i < num; i++) {
        float dx = pts_in[clip(i + approx_num, 0, num - 1)][0] -
                   pts_in[clip(i - approx_num, 0, num - 1)][0];
        float dy = pts_in[clip(i + approx_num, 0, num - 1)][1] -
                   pts_in[clip(i - approx_num, 0, num - 1)][1];
        float dn = sqrt(dx * dx + dy * dy);

        dx /= dn;
        dy /= dn;

        pts_out[i][0] = pts_in[i][0] + dy * dist;
        pts_out[i][1] = pts_in[i][1] - dx * dist;
    }
}

/* *********************************************************************** */
/* *********************************************************************** */
/* *********************************************************************** */