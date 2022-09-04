#ifndef _DATAPACKET_HPP_
#define _DATAPACKET_HPP_

#include <cstdint>

// 向下位机传输的数据包
struct Payload_t {
    int16_t tSpeed;   // 速度
    int16_t tAngle;   // 偏移
    uint8_t element;  // 元素
};

// 从下位机接受的数据包 车模速度
struct Encoder_t {
    float speed;  // 速度
    float route;  // 路程
};

#endif
