/**
 * @file logger_helper.hpp
 * @author Pomin
 * @brief 基于 easylogger C库的C++兼容头文件
 * @date 2022-05-08
 *
 * @copyright Copyright (c) 2022
 *
 **/
#ifndef _LOGGER_HELPER_H
#define _LOGGER_HELPER_H

#include <unistd.h>

#include <iostream>
#include <sstream>

#include "logger/inc/elog.h"

using namespace std;

#define LOG_END '@'

class logger_helper {
   private:
   public:
    ostringstream buf;  /*  缓冲区    */
    string tag;         /*  标识符    */
    string file;        /*  文件位置  */
    string fun;         /*  函数名称  */
    uint32_t lvl;       /*  日志等级  */
    uint32_t line;      /*  行数定位  */
    uint32_t exit;      /*  检错退出  */
    uint32_t precision; /*  浮点精度  */

    void init_logger(void);

    /* 精度转换 */
    template <typename T>
    std::string to_string_with_precision(const T a_value, const int n) {
        std::ostringstream out;
        out.precision(n);
        // out.width(9);
        out << std::fixed << a_value;
        return out.str();
    }

    /* 重载 << */
    /* 字符 */
    logger_helper& operator<<(char c) {
        /* 终止符 */
        if (c == LOG_END && LOG_LVL >= this->lvl) {
            elog_output(this->lvl, this->tag.c_str(), this->file.c_str(),
                        this->fun.c_str(), this->line, this->buf.str().c_str());
            this->buf.clear();
            this->buf.str("");
            if (this->exit) {
                terminate();
            }
        } else {
            this->buf << c;
        }
        return *this;
    }

    /* 布尔 */
    logger_helper& operator<<(bool b) {
        this->buf << (b ? "true" : "false");
        return *this;
    }

    /* 其他的都用模板和 ostringstream 流 */
    template <typename U>
    logger_helper& operator<<(U str) {
        this->buf << to_string_with_precision(str, this->precision);
        return *this;
    }
};

extern logger_helper logger;

#define LOG_SET(x, _lvl)  \
    x.file = __FILE__;    \
    x.fun = __FUNCTION__; \
    x.tag = LOG_TAG;      \
    x.line = __LINE__;    \
    x.lvl = _lvl

#define LVL_A ELOG_LVL_ASSERT
#define LVL_E ELOG_LVL_ERROR
#define LVL_W ELOG_LVL_WARN
#define LVL_I ELOG_LVL_INFO
#define LVL_D ELOG_LVL_DEBUG
#define LVL_V ELOG_LVL_VERBOSE

#define LOG_A                   \
    do {                        \
        LOG_SET(logger, LVL_A); \
    } while (0);                \
    logger
#define LOG_E                   \
    do {                        \
        LOG_SET(logger, LVL_E); \
    } while (0);                \
    logger
#define LOG_W                   \
    do {                        \
        LOG_SET(logger, LVL_W); \
    } while (0);                \
    logger
#define LOG_I                   \
    do {                        \
        LOG_SET(logger, LVL_I); \
    } while (0);                \
    logger
#define LOG_D                   \
    do {                        \
        LOG_SET(logger, LVL_D); \
    } while (0);                \
    logger
#define LOG_V                   \
    do {                        \
        LOG_SET(logger, LVL_V); \
    } while (0);                \
    logger

#define LOG_(_lvl) LOG_##_lvl
#define LOG_IF(is_ok, _lvl)              \
    if (is_ok)                           \
        do {                             \
            LOG_SET(logger, LVL_##_lvl); \
        } while (0);                     \
    if (is_ok)                           \
    logger
#define CHECK(is_ok)        \
    logger.exit = !(is_ok); \
    LOG_IF(!(is_ok), E)

#define log_(_lvl) log_##_lvl
#define log_if(is_ok, _lvl, ...) \
    if (is_ok)                   \
    log_(_lvl)(__VA_ARGS__)
#define check(is_ok, ...)             \
    log_if(!(is_ok), e, __VA_ARGS__); \
    if (!(is_ok))                     \
    terminate()

#endif  // _LOGGER_HELPER_H
