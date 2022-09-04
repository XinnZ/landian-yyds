#ifndef _SIDELINE_HPP_
#define _SIDELINE_HPP_

#include <assert.h>

#include <cstdint>
#include <opencv2/opencv.hpp>

#include "imgprocess.hpp"

/* ********************************************************************* */

#define AT_IMAGE(img, x, y) (img.at<uint8_t>(y, x))
#define AT_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define AT_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define AT_MINMAX(input, low, upper) MIN(MAX(input, low), upper)

/* ********************************************************************* */

/* 窗口绘制日志 */
#define MAT_LOG(...)                      \
    sprintf(IMG.string_log, __VA_ARGS__); \
    log_i(__VA_ARGS__);

/* 窗口绘制信息 */
#define MAT_INFO(mat, point, size, ...)                                        \
    sprintf(string_buf, __VA_ARGS__);                                          \
    cv::putText(mat, (std::string)string_buf, point, cv::FONT_HERSHEY_SIMPLEX, \
                size, cv::Scalar(0, 0, 255));

/* ********************************************************************* */

// 透视变换 (0:原图->俯视, 1:俯视->原图)
void map_perspective(float x, float y, float loc[2], uint8_t mode);

/* ********************************************************************* */

// 位置式 PID 角度外环
float pid_realize_a(float actual, float set, float _p, float _d);

// 位置式 PID 角速度内环
int pid_realize_o(int actual, int set, float _p, float _d);

// 滑动平均滤波
float filter(float value);

/* ********************************************************************* */

// 赛道点集的方差计算
double sigma(float pts[][2], int num);
double sigma(std::vector<POINT> vec);

// 点到直线的距离计算
double dis_point_line(POINT a, POINT b, POINT p);

// 两点间的距离计算
double dis_point_point(POINT a, POINT b);

// 直线拟合 (返回平均绝对误差)
float fit_line(float pts[][2], int num, int cut_h);

// 贝塞尔曲线拟合
std::vector<POINT> bezier(double dt, std::vector<POINT> input);

/* ********************************************************************* */

bool sort_y_up(POINT a, POINT b);    // 升序
bool sort_y_down(POINT a, POINT b);  // 降序
bool sort_x_up(POINT a, POINT b);    // 升序
bool sort_x_down(POINT a, POINT b);  // 降序

// 从AI检测结果中检索锥桶坐标集合
std::vector<POINT> search_targets(std::vector<DetectionPredictResult> predict);

// 搜索距离赛道左边缘最近的红色色块 (红色锥桶)
POINT search_left_near(std::vector<POINT> pointsEdge, cv::Mat matt);

// 搜索距离赛道右边缘最近的红色色块 (红色锥桶)
POINT search_right_near(std::vector<POINT> pointsEdge, cv::Mat matt);

// 搜索左方的锥桶集合 ( IMAGE_WIDTH / 3 )
std::vector<POINT> search_left_cones(std::vector<POINT> pointsCone);

// 搜索右方的锥桶集合 ( IMAGE_WIDTH * 2 / 3 )
std::vector<POINT> search_right_cones(std::vector<POINT> pointsCone);

// 搜索左上方的锥桶集合 (除了右下角)
std::vector<POINT> search_leftUp_cones(std::vector<POINT> pointsCone);

// 搜索右上方的锥桶集合 (除了左下角)
std::vector<POINT> search_rightUp_cones(std::vector<POINT> pointsCone);

// 搜索右上方的锥桶
POINT search_rightUp_cone(std::vector<POINT> pointsCone);

// 搜索左上方的锥桶
POINT search_leftUp_cone(std::vector<POINT> pointsCone);

// 搜索右下方的锥桶
POINT search_rightDown_cone(std::vector<POINT> pointsCone);

// 搜索左下方的锥桶
POINT search_leftDown_cone(std::vector<POINT> pointsCone);

/* ********************************************************************* */

// 限制范围
int clip(int x, int low, int up);

// 左手迷宫巡线
void findline_lefthand_adaptive(cv::Mat img, int block_size, int clip_value,
                                int x, int y, int pts[][2], int *num);

// 右手迷宫巡线
void findline_righthand_adaptive(cv::Mat img, int block_size, int clip_value,
                                 int x, int y, int pts[][2], int *num);

// 点集三角滤波
void blur_points(float pts_in[][2], int num, float pts_out[][2], int kernel);

// 点集等距采样  使走过的采样前折线段的距离为`dist`
void resample_points(float pts_in[][2], int num1, float pts_out[][2], int *num2,
                     float dist);

// 点集局部角度变化率
void local_angle_points(float pts_in[][2], int num, float angle_out[],
                        int dist);

// 角度变化率非极大抑制
void nms_angle(float angle_in[], int num, float angle_out[], int kernel);

// 左边线跟踪中线
void track_leftline(float pts_in[][2], int num, float pts_out[][2],
                    int approx_num, float dist);

// 右边线跟踪中线
void track_rightline(float pts_in[][2], int num, float pts_out[][2],
                     int approx_num, float dist);

/* ********************************************************************* */

#endif
