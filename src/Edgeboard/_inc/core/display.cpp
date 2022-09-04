#include "display.hpp"

#undef LOG_TAG
#define LOG_TAG "Display"

int Display::init() {
    if (_windows) {
        cv::namedWindow(_win_name, cv::WINDOW_NORMAL);
        cv::resizeWindow(_win_name, 600, 400);
        cv::moveWindow(_win_name, 0, 0);
        // cv::setMouseCallback(_win_name, OnMouseCallBack, this);

        LOG_I << "Create Windows Success." << LOG_END;
    }

    if (_video) {
        _out_video.open("yyds.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
                        60.0, cv::Size(600, 400), true);
        if (!_out_video.isOpened())
            return -1;

        LOG_I << "Create Video Success." << LOG_END;
    }

    return 0;
}

int Display::start() {
    if (!(_windows || _video))
        return 0;

    _loop = true;
    _worker_thread = std::thread(&Display::run, this);

    log_i("Start!");
    return 0;
}

int Display::stop() {
    if (!(_windows || _video))
        return 0;

    _loop = false;
    _worker_thread.join();
    _queue.ShutDown();
    _out_video.release();

    cv::destroyAllWindows();

    log_i("Stop.");
    return 0;
}

void Display::put(cv::Mat frame) {
    if (!(_windows || _video))
        return;

    if (_queue.Size() < 10)
        _queue.Put(frame);
}

void Display::run() {
    cv::Mat frame_display;

    while (_loop) {
        if (_queue.Size() >= 1) {
            cv::resize(_queue.Take(), frame_display, cv::Size(600, 400));

            if (_windows) {
                cv::imshow(_win_name, frame_display);

                OnKeyBoardCallBack(cv::waitKeyEx(1));
                if (_save_flag) {
                    _save_flag = 0;
                    char imageName[25];
                    std::sprintf(imageName, "./pic/%04d%s", _save_counter++, ".jpg");
                    cv::imwrite(imageName, frame_display);
                    LOG_I << "Save Pic:" << imageName << LOG_END;
                }
            }

            if (_video) {
                _out_video << frame_display;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Display::OnKeyBoardCallBack(int key_value) {
    if (key_value == -1)
        return;

    switch (key_value) {
        case KEY_BOARD_s:
        case KEY_BOARD_S:
            _save_flag = 1;
            break;

        case KEY_BOARD_ESC:
        case KEY_BOARD_ARROW_LEFT:
        case KEY_BOARD_ARROW_RIGHT:
        default:
            break;
    }
}

/*
void Display::OnMouseCallBack(int event, int x, int y, int flags, void *param) {
    Display *display = (Display *)param;

    switch (event) {
        case CV_EVENT_LBUTTONDOWN:
        case CV_EVENT_RBUTTONDOWN:
        case CV_EVENT_LBUTTONUP:
        case CV_EVENT_MOUSEMOVE:
        default:
            break;
    }
}
*/
