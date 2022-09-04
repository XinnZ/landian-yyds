#include "capture.hpp"

#undef LOG_TAG
#define LOG_TAG "Capture"

int CaptureInterface::init() {
    _capture = std::make_shared<cv::VideoCapture>();
    _capture->open(0, cv::CAP_V4L);  // ("/dev/video0", cv::CAP_V4L);
    CHECK(_capture->isOpened()) << "Create Capture Failed." << LOG_END;

    _capture->set(cv::CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));
    _capture->set(cv::CAP_PROP_FPS, 60);
    _capture->set(cv::CAP_PROP_FRAME_WIDTH, 640);
    _capture->set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    LOG_I << ">>> width  = " << _capture->get(cv::CAP_PROP_FRAME_WIDTH) << LOG_END;
    LOG_I << ">>> height = " << _capture->get(cv::CAP_PROP_FRAME_HEIGHT) << LOG_END;
    LOG_I << ">>> fps    = " << _capture->get(cv::CAP_PROP_FPS) << LOG_END;
    LOG_I << "Create Capture Success." << LOG_END;

    return 0;
}

int CaptureInterface::start() {
    _loop = true;
    _worker_thread = std::thread(&CaptureInterface::run, this);

    log_i("Start!");
    return 0;
}

int CaptureInterface::stop() {
    if (_capture->isOpened()) {
        _capture->release();
    }

    _loop = false;
    _worker_thread.join();
    _queue.ShutDown();

    log_i("Stop.");
    return 0;
}

void CaptureInterface::get(cv::Mat &frame) { frame = _queue.Take().clone(); }

void CaptureInterface::run() {
    cv::Mat _frame;

    while (_loop) {
        if (_capture->read(_frame)) {
            // 图片裁剪 (600x400)  (横坐标偏移修正 - 5)
            _frame = _frame(cv::Rect(20 - 5, 35, 600, 400));

            // 绘制田字格：基准线
            /*
            uint8_t rows = 400 / 40;
            uint8_t cols = 600 / 60;

            for (uint8_t i = 1; i < rows; i++) {
                cv::line(_frame, cv::Point(0, 40 * i), cv::Point(_frame.cols - 1, 40 * i), cv::Scalar(211, 211, 211), 1);
            }
            for (uint8_t i = 1; i < cols; i++) {
                if (i == cols / 2)
                    cv::line(_frame, cv::Point(60 * i, 0), cv::Point(60 * i, _frame.rows - 1), cv::Scalar(0, 0, 255), 1);
                else
                    cv::line(_frame, cv::Point(60 * i, 0), cv::Point(60 * i, _frame.rows - 1), cv::Scalar(211, 211, 211), 1);
            }
            */

            if (_queue.Size() > 20)
                _queue.Take();
            _queue.Put(_frame);
        } else {
            CHECK(false) << "UsbCamera read failed." << LOG_END;
        }

#undef LOG_TAG
#define LOG_TAG "System"

        // 结束程序
        if (_kbhit_()) {
            char c = fgetc(stdin);
            if (c == 'q' || c == 'Q') {
                log_i("Exit...");
                _loop = false;
                _launch = false;
            }
            if (c == 's' || c == 'S') {
                log_i("Pause...");
                _launch = false;
            }
            if (c == 'w' || c == 'W') {
                log_i("Run!");
                _launch = true;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

bool CaptureInterface::_kbhit_() {
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}
