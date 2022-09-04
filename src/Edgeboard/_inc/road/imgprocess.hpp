#pragma once
#include <cstdint>
#include <opencv2/opencv.hpp>

#include "../common/model_config.hpp"
#include "../core/detection.hpp"
#include "../utils/logger_helper.hpp"
#include "../utils/serial_packets.hpp"
#include "_define.hpp"
#include "_extern.hpp"
#include "_sideline.hpp"
#include "elements/_circle.hpp"
#include "elements/_cross.hpp"
#include "elements/_garage.hpp"
#include "elements/_gas.hpp"
#include "elements/_ramp.hpp"
#include "elements/_work.hpp"
#include "elements/_yroad.hpp"

class ImageProcess {
   public:
    ImageProcess(){};
    ~ImageProcess(){};

   public:
    int init(std::string config_path, bool is_result);
    void process(cv::Mat &mat_origin, cv::Mat &mat_result, Payload_t &payload);

   private:
    bool _is_result = false;  // 是否生成处理后的图像

    float cx = IMAGE_WIDTH / 2.0f;  // 车轮对应点 (纯跟踪起始点)
    float cy = IMAGE_HEIGHT * 0.99f;

    int aim_idx__far = 0;  // 远预锚点位置
    int aim_idx_near = 0;  // 近预锚点位置

    float mea_0 = 0.0f;  // 左直线拟合平均绝对误差
    float mea_1 = 0.0f;  // 右直线拟合平均绝对误差

    std::vector<POINT> bezier_line;  // 中线贝塞尔曲线拟合

    float aim_angle_p_k;
    float aim_angle_p;
    float aim_angle_d;

    uint16_t speed_diff = 0;  // 减速率

   public:
    std::shared_ptr<ControlConfig> _config;           // 跑车配置文件
    std::vector<DetectionPredictResult> _det_result;  // 目标检测结果

    cv::Mat mat_bin;  // 原图转二值化图
    cv::Mat mat_lab;  // 原图转 LAB 图

    flag_track_e flag_track = TRACK_MIDDLE;  // 巡线选择

    int begin_x_l = BEGIN_X;                // 巡线横坐标起始点 左
    int begin_x_r = IMAGE_WIDTH - BEGIN_X;  // 巡线横坐标起始点 右
    int begin_y_t = BEGIN_Y;                // 巡线纵坐标起始点

    int16_t aim_speed = 0;        // 速度量
    int16_t aim_speed_shift = 0;  // 变速计数器
    float aim_angle = 0.0f;       // 偏差量
    float aim_angle_last = 0.0f;  // 偏差量 上一帧
    float aim_sigma = 0.0f;       // 偏差方差
    float aim_distance_f = 0.0f;  // 远锚点
    float aim_distance_n = 0.0f;  // 近锚点

    int element_begin_id = 0;         // 特殊元素中线起始点
    float element_over_route = 0.0f;  // 元素结束路程
    bool element_over = true;         // 元素结束标志
    int element_identify = 0;         // 识别元素

    char string_buf[64];               // 窗口信息缓存
    char string_log[32] = "yyds car";  // 窗口日志缓存

    bool special_w_c = false;  // 施工区到十字慢速处理
    bool special_w = false;    // 施工区结束特殊处理
    bool special_g = false;    // 加油站结束特殊处理

    /* *********************************************************************** */
    /* ****************************** 下位机数据 ****************************** */
    /* *********************************************************************** */

   public:
    Encoder_t encoder = {0};

    /* *********************************************************************** */
    /* ***************************** 目标检测数据 ***************************** */
    /* *********************************************************************** */

   public:
    bool detection_new = false;
    string detection_labels[11] = {"background", "ramp", "three", "work",
                                   "red", "gas", "wone", "wtwo"};

    enum DetectionLabel {
        label_ramp = 1,  // 坡道
        label_three,     // 泛行区
        label_work,      // 施工区
        label_red,       // 锥桶
        label_gas,       // 加油站
        label_wone,      // 赛道 1
        label_wtwo,      // 赛道 2
    };

    /* *********************************************************************** */
    /* ******************************* 角点数据 ******************************* */
    /* *********************************************************************** */

   private:
    // L 角点置信度 (角度)
    float Lpt0_found_conf, Lpt1_found_conf;
    // L 角点二次判断
    bool Lpt0_found_last = false, Lpt1_found_last = false;

   public:
    // L 角点
    int Lpt0_rpts0s_id, Lpt1_rpts1s_id;
    bool Lpt0_found, Lpt1_found;
    // 长直道
    bool is_straight0, is_straight1;
    // 弯道 左 右 强制
    bool is_curve0, is_curve1;

    /* *********************************************************************** */
    /* ******************************* 边线数据 ******************************* */
    /* *********************************************************************** */

   public:
    // 原图左右中线数据定义
    int ipts0[IMAGE_HEIGHT][2];  // 左: 0
    int ipts1[IMAGE_HEIGHT][2];  // 右: 1
    int iptsc[IMAGE_HEIGHT];     // 中: c
    int ipts0_num, ipts1_num, rptsc_num;
    // 透视变换后左右中线  滤波: b
    float rpts0b[IMAGE_HEIGHT][2];
    float rpts1b[IMAGE_HEIGHT][2];
    float rptscb[IMAGE_HEIGHT][2];
    // 透视变换后左右中线  等距采样: s
    float rpts0s[IMAGE_HEIGHT][2];
    float rpts1s[IMAGE_HEIGHT][2];
    float rptscs[IMAGE_HEIGHT][2];
    int rpts0s_num, rpts1s_num, rptscs_num;
    // 左右边线局部角度变化率: a
    float rpts0a[IMAGE_HEIGHT];
    float rpts1a[IMAGE_HEIGHT];
    // 左右边线局部角度变化率非极大抑制: an
    float rpts0an[IMAGE_HEIGHT];
    float rpts1an[IMAGE_HEIGHT];
    // 左右边线偏移中线: c
    float rptsc0[IMAGE_HEIGHT][2];
    float rptsc1[IMAGE_HEIGHT][2];
    int rptsc0_num, rptsc1_num;

   public:
    std::vector<POINT> edge_det;        // AI元素检测边缘点集
    std::vector<POINT> edge_left;       // 赛道左边缘点集   注意: 此点集 x 与 y 位置相反 !!!
    std::vector<POINT> edge_right;      // 赛道右边缘点集
    std::vector<POINT> last_edge_left;  // 记录上一场边缘点集 (丢失边)
    std::vector<POINT> last_edge_right;

   public:
    int det_ipts[IMAGE_HEIGHT][2];     // 加油站 & 施工区 边线原图
    float det_rpts[IMAGE_HEIGHT][2];   // 加油站 & 施工区 边线透视
    float det_rptsb[IMAGE_HEIGHT][2];  // 加油站 & 施工区 边线滤波
    float det_rptss[IMAGE_HEIGHT][2];  // 加油站 & 施工区 边线等距采样
    int det_ipts_num, det_rptss_num;

   private:
    // 中线
    float (*rpts)[2];
    int rpts_num;
    // 归一化中线
    float rptsn[IMAGE_HEIGHT][2];
    int rptsn_num;

    // 透视变换临时变量
    float trans[2];

    /* *********************************************************************** */
    /* *********************************************************************** */
    /* *********************************************************************** */
};
