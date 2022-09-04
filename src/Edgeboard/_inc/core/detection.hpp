#pragma once
#include <unistd.h>

#include <opencv2/opencv.hpp>
#include <thread>

#include "../common/model_config.hpp"
#include "../utils/blocking_queue.h"
#include "../utils/logger_helper.hpp"
#include "../utils/stop_watch.hpp"
#include "preprocess.hpp"

#undef LOG_TAG
#define LOG_TAG "Detection"

struct DetectionPredictResult {
    int type;
    float score;
    int x;
    int y;
    int width;
    int height;
};

struct DetectionResult {
    cv::Mat org_frame;                           // 原图
    cv::Mat det_frame;                           // 结果图
    std::vector<DetectionPredictResult> result;  // 预测结果
};

class DetectionPredictor {
   public:
    // 传入的是模型目录
    DetectionPredictor(std::string config_path) : _config_path(config_path){};
    ~DetectionPredictor(){};

   private:
    std::string _config_path;
    std::shared_ptr<ModelConfig> _model_config;
    std::shared_ptr<PaddlePredictor> _predictor;
    void _boundaryCorrection(DetectionPredictResult &r, int width_range, int height_range);

   public:
    int init();
    std::vector<DetectionPredictResult> predict(cv::Mat input_frame);
    void printResults(std::vector<DetectionPredictResult> &results);
    void drawResults(cv::Mat &input_frame, std::vector<DetectionPredictResult> &results);
    std::string getLabel(uint16_t type);
};

class Detection {
   public:
    Detection(){};
    ~Detection(){};

   private:
    std::shared_ptr<SystemConfig> _system_config;
    std::shared_ptr<DetectionPredictor> _predictor;
    std::shared_ptr<DetectionResult> _result;

    std::thread _worker_thread;

    BlockingQueue<cv::Mat> _queue;

    bool _loop;
    bool _can_predict;

    StopWatch stopwatch;

   private:
    void work() {
        while (_loop) {
            if (_queue.Size() >= 1) {
                _can_predict = false;

                // stopwatch.tic();

                _result->org_frame = _queue.Take().clone();
                _result->result = _predictor->predict(_result->org_frame);

                // log_i("detection time: %f", stopwatch.toc());

                _is_predicted = true;
            }
            _can_predict = true;

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    };

   public:
    bool _is_predicted = false;

    int init(std::string system_config_path) {
        _system_config = make_shared<SystemConfig>(system_config_path);

        LOG_I << "SystemConfig Path:" << system_config_path << LOG_END;
        LOG_I << "Model Config Path:" << _system_config->model_config_path
              << LOG_END;

        _predictor = std::make_shared<DetectionPredictor>(_system_config->model_config_path);
        CHECK(_predictor != nullptr) << "Predictor Create failed." << LOG_END;

        int ret = _predictor->init();
        CHECK(ret == 0) << "Predictor Create failed." << LOG_END;

        _result = std::make_shared<DetectionResult>();

        return 0;
    };

    int start() {
        _loop = true;
        _can_predict = true;
        _is_predicted = false;
        _worker_thread = std::thread(&Detection::work, this);

        log_i("Start!");
        return 0;
    };

    int stop() {
        _loop = false;
        _can_predict = false;
        _is_predicted = false;
        _worker_thread.join();
        _queue.ShutDown();

        log_i("Stop.");
        return 0;
    };

    void run(cv::Mat frame_wraper) {
        if (_can_predict && _queue.Size() < 2) {
            _queue.Put(frame_wraper);
        }
    };

    std::shared_ptr<DetectionResult> getResult() {
        if (_is_predicted) {
            _is_predicted = false;
            return _result;
        }
        return nullptr;
    };

    std::string getLabel(int type) { return _predictor->getLabel(type); };
};
