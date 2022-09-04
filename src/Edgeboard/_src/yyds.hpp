#pragma once

#include <unistd.h>

#include <csignal>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "../_inc/core/capture.hpp"
#include "../_inc/core/detection.hpp"
#include "../_inc/core/display.hpp"
#include "../_inc/road/imgprocess.hpp"
#include "../_inc/utils/argparse.hpp"
#include "../_inc/utils/logger_helper.hpp"
#include "../_inc/utils/serial.hpp"
#include "../_inc/utils/serial_packets.hpp"
#include "../_inc/utils/stop_watch.hpp"

// 系统信号回调函数
void callbackSignal(int signum);
