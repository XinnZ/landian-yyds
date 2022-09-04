#ifndef _DEFINE_HPP_
#define _DEFINE_HPP_

#define IMAGE_WIDTH (300)
#define IMAGE_HEIGHT (200)

#define BEGIN_X (120)  // 巡线横坐标起始点
#define BEGIN_Y (190)  // 巡线纵坐标起始点

#define ROAD_WIDTH (0.40)      // 赛道宽度 (0.4)
#define BLOCK_SIZE (7)         // 自适应阈值的block大小 (7)
#define CLIP_VALUE (2)         // 自适应阈值的阈值裁减量 (2)
#define LINE_BLUR_KERNEL (7)   // 边线三角滤波核的大小 (7)
#define PIXEL_PER_METER (242)  // 俯视图中, 每个像素对应的长度 (102)
#define SAMPLE_DIST (0.02)     // 边线等距采样的间距 (0.02)
#define ANGLE_DIST (0.2)       // 计算边线转角时, 三个计算点的距离 (0.2)

#define PI (3.14159265)

enum flag_track_e {
    TRACK_LEFT,    // 寻左线
    TRACK_MIDDLE,  // 寻中线
    TRACK_RIGHT,   // 寻右线
};

// 重构坐标类
struct POINT {
    int x = 0;           //横坐标
    int y = 0;           //纵坐标
    float slope = 0.0f;  //累计斜率

    POINT(){};
    POINT(int x, int y) : x(x), y(y){};
};

#endif
