#pragma once
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <thread>

#include "../utils/blocking_queue.h"
#include "../utils/logger_helper.hpp"

class CaptureInterface {
   public:
    CaptureInterface(){};
    ~CaptureInterface(){};

   private:
    std::shared_ptr<cv::VideoCapture> _capture;
    std::thread _worker_thread;
    BlockingQueue<cv::Mat> _queue;

   public:
    bool _loop = false;
    bool _launch = false;

   public:
    int init();
    int start();
    int stop();

    void get(cv::Mat &frame);

   private:
    void run();
    bool _kbhit_();
};
