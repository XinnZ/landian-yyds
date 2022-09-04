#include "detection.hpp"

#undef LOG_TAG
#define LOG_TAG "DetPredictor"

void DetectionPredictor::_boundaryCorrection(DetectionPredictResult &r,
                                             int width_range,
                                             int height_range) {
#define MARGIN_PIXELS (2)
    r.width = (r.width > (width_range - r.x - MARGIN_PIXELS))
                  ? (width_range - r.x - MARGIN_PIXELS)
                  : r.width;
    r.height = (r.height > (height_range - r.y - MARGIN_PIXELS))
                   ? (height_range - r.y - MARGIN_PIXELS)
                   : r.height;

    r.x = (r.x < MARGIN_PIXELS) ? MARGIN_PIXELS : r.x;
    r.y = (r.y < MARGIN_PIXELS) ? MARGIN_PIXELS : r.y;
}

// 初始化
int DetectionPredictor::init() {
    _model_config = std::make_shared<ModelConfig>(_config_path);
    CHECK(_model_config != nullptr) << "Create Model config failed, config path: "
                                    << _config_path << LOG_END;

    std::vector<Place> valid_places({
        Place{TARGET(kFPGA), PRECISION(kFP16), DATALAYOUT(kNHWC)},
        Place{TARGET(kHost), PRECISION(kFloat)},
        Place{TARGET(kARM), PRECISION(kFloat)},
    });

    paddle::lite_api::CxxConfig config;

    if (_model_config->is_combined_model) {
        config.set_model_file(_model_config->model_file);
        config.set_param_file(_model_config->params_file);
    } else {
        config.set_model_dir(_model_config->model_params_dir);
    }

    config.set_valid_places(valid_places);

    _predictor = paddle::lite_api::CreatePaddlePredictor(config);

    if (!_predictor) {
        log_e("Create Paddle Predictor Failed.");
        return -1;
    }
    log_i("Init Success.");

    return 0;
}

// 预测
std::vector<DetectionPredictResult> DetectionPredictor::predict(cv::Mat input_frame) {
    std::vector<DetectionPredictResult> predict_ret;

    auto input = _predictor->GetInput(0);
    input->Resize({1, 3, _model_config->input_height, _model_config->input_width});

    // auto *in_data = input->mutable_data<uint16_t>();
    input->mutable_data<uint16_t>();
    fpga_preprocess(input_frame, _model_config, input);

    if (_model_config->is_yolo) {
        auto img_shape = _predictor->GetInput(1);
        img_shape->Resize({1, 2});
        auto *img_shape_data = img_shape->mutable_data<int32_t>();
        img_shape_data[0] = input_frame.rows;
        img_shape_data[1] = input_frame.cols;
    }

    _predictor->Run();

    auto output = _predictor->GetOutput(0);
    float *result_data = output->mutable_data<float>();
    int size = output->shape()[0];

    for (int i = 0; i < size; i++) {
        float *data = result_data + i * 6;
        float score = data[1];
        if (score < _model_config->threshold) {
            continue;
        }
        DetectionPredictResult r;
        r.type = (int)data[0];
        r.score = score;
        if (_model_config->is_yolo) {
            r.x = data[2];
            r.y = data[3];
            r.width = data[4] - r.x;
            r.height = data[5] - r.y;
        } else {
            r.x = data[2] * input_frame.cols;
            r.y = data[3] * input_frame.rows;
            r.width = data[4] * input_frame.cols - r.x;
            r.height = data[5] * input_frame.rows - r.y;
        }
        predict_ret.push_back(r);
    }

    return predict_ret;
}

// 检测结果格式化输出
void DetectionPredictor::printResults(
    std::vector<DetectionPredictResult> &results) {
    for (uint32_t i = 0; i < results.size(); ++i) {
        DetectionPredictResult r = results[i];
        if (r.type >= 0 && (uint16_t)r.type < _model_config->labels.size()) {
            LOG_I << "label:" << _model_config->labels[r.type] << LOG_END;
            LOG_I << "\t\tindex:" << r.type << ", score:" << r.score
                  << ", loc:" << r.x << ", " << r.y << ", " << r.width << ", "
                  << r.height << LOG_END;
        }
    }
}

// 针对检测结果, 进行画框操作
void DetectionPredictor::drawResults(
    cv::Mat &input_frame, std::vector<DetectionPredictResult> &results) {
    for (uint32_t i = 0; i < results.size(); ++i) {
        DetectionPredictResult r = results[i];
        _boundaryCorrection(r, input_frame.cols, input_frame.rows);
        if (r.type >= 0 && (uint16_t)r.type < _model_config->labels.size()) {
            cv::Point origin(r.x, r.y);
            std::string label_name = _model_config->labels[r.type];
            cv::putText(input_frame, label_name, origin, cv::FONT_HERSHEY_PLAIN, 1,
                        cv::Scalar(255, 0, 0, 255), 2);
        }
        cv::Rect rect(r.x, r.y, r.width, r.height);
        cv::rectangle(input_frame, rect, Scalar(0, 0, 224), 2);
    }
}

// 根据type的id, 返回具体label
std::string DetectionPredictor::getLabel(uint16_t type) {
    if ((type < 0) || (type > _model_config->labels.size())) {
        LOG_I << "Predictor Label [" << type << "] is error." << LOG_END;
        return "BackGround";
    }

    return _model_config->labels[type];
}
