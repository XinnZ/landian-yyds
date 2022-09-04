#pragma once
#include <unistd.h>

#include <opencv2/opencv.hpp>
#include <thread>

#include "../utils/blocking_queue.h"
#include "../utils/logger_helper.hpp"

#define KEY_BOARD_ESC (27)
#define KEY_BOARD_S (83)
#define KEY_BOARD_s (115)
#define KEY_BOARD_ARROW_LEFT (65361)
#define KEY_BOARD_ARROW_RIGHT (65363)

class Display {
   public:
    Display(bool window, bool video) : _windows(window), _video(video){};
    ~Display(){};

   private:
    bool _windows = false;
    bool _video = false;
    bool _loop = false;

    bool _save_flag = false;
    int _save_counter = 0;

    BlockingQueue<cv::Mat> _queue;

    std::thread _worker_thread;

    std::string _win_name = "yyds car";

    cv::VideoWriter _out_video;

   public:
    int init();
    int start();
    int stop();

    void put(cv::Mat frame);

   private:
    void run();

    void OnKeyBoardCallBack(int key_value);
    // static void OnMouseCallBack(int event, int x, int y, int flags, void
    // *param);
};
