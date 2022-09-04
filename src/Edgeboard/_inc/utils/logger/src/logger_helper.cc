/**
 * @file logger_helper.hpp
 * @author Pomin
 * @brief 基于 easylogger C库的C++兼容
 * @date 2022-05-08
 *
 * @copyright Copyright (c) 2022
 *
 **/
#include "../../logger_helper.hpp"

void logger_helper::init_logger(void) {
  /* close printf buffer */
  // setbuf(stdout, NULL);
  /* initialize EasyLogger */
  elog_init();
  /* set EasyLogger log format */
  elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
  elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_FILE |
                                   ELOG_FMT_LINE | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_FILE |
                                  ELOG_FMT_LINE | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_FILE |
                                  ELOG_FMT_LINE | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC));
  elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC));
  /* set color */
  elog_set_text_color_enabled(true);
  /* start EasyLogger */
  elog_start();
  this->exit = 0;
  this->lvl = 0;
  this->line = 0;
  this->precision = 2;

  LOG_I << "Build: " __DATE__ " " __TIME__
           "\r\n----------------------------------"
           "\r\n__   ____   ______  ____  "
           "\r\n\\ \\ / /\\ \\ / /  _ \\/ ___| "
           "\r\n \\ V /  \\ V /| | | \\___ \\ "
           "\r\n  | |    | | | |_| |___) |"
           "\r\n  |_|    |_| |____/|____/ \r\n"
           "\r\n----------------------------------"
        << LOG_END;
}
