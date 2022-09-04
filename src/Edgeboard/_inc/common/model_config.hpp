#pragma once
#include <chrono>  // NOLINT
#include <fstream>
#include <iostream>
#include <vector>

#include "../utils/json.hpp"
#include "../utils/logger_helper.hpp"

#undef LOG_TAG
#define LOG_TAG "Control Json"

using namespace std;
using json = nlohmann::json;

// 车模控制配置项
struct ControlConfig {
    std::string config_dir;
    std::string speed_level;

    uint16_t threshold;            // 二值化阈值
    uint16_t threshold_detection;  // 目标检测阈值

    uint8_t direction_garage;  // 出库方向   0: 左   1: 右
    uint8_t direction_yroad;   // 泛行区方向  0: 左   1: 右
    uint8_t direction_work;    // 施工区方向  0: 左   1: 右
    uint8_t direction_gas;     // 加油站方向  0: 左   1: 右

    uint8_t check_work_;    // 是否识别施工区
    uint8_t check_gas_;     // 是否识别加油站
    uint8_t check_yroad_;   // 是否识别泛行区
    uint8_t check_ramp_;    // 是否识别坡道
    uint8_t check_circle_;  // 是否识别环岛
    uint8_t check_cross_;   // 是否识别十字

    float ai_p_k;  // 特殊元素 动态 P
    float ai_p;    // 特殊元素 P
    float ai_d;    // 特殊元素 D

    uint8_t gas_rou_ready;   // 加油站准备进入积分
    uint8_t gas_rou_run1;    // 加油站内部 #1 积分
    uint8_t gas_rou_run2;    // 加油站内部 #2 积分
    uint8_t gas_rou_out1;    // 加油站驶离 #1 积分
    uint8_t gas_rou_out2;    // 加油站驶离 #2 积分
    uint8_t gas_rou_none;    // 加油站结束积分
    uint8_t gas_rou_over;    // 加油站完成积分
    uint8_t work_rou_ready;  // 施工区准备进入积分
    uint8_t work_rou_run;    // 施工区内部积分
    uint8_t work_rou_out;    // 施工区驶离积分
    uint8_t work_rou_none;   // 施工区完成积分
    uint8_t work_rou_over;   // 施工区完成积分

    float aim_distance_far;   // 远预瞄点
    float aim_distance_near;  // 近预瞄点

    uint16_t speed_base;        // 速度基准
    uint16_t speed_up;          // 直道速度
    uint16_t speed_steady;      // 元素稳定速度
    uint16_t speed_garage_in;   // 入库速度
    uint16_t speed_garage_out;  // 出库速度
    uint16_t speed_work;        // 出入施工区速度
    uint16_t speed_gas;         // 出入加油站速度
    uint16_t speed_yroad;       // 泛行区速度
    uint16_t speed_circle;      // 出入环岛速度
    uint16_t speed_cross;       // 十字速度
    uint16_t speed_ramp_up;     // 上坡速度
    uint16_t speed_ramp_down;   // 下坡速度

    float deviation_garage_in_p;  // 入库偏差 (P项增大值)
    float deviation_garage_out;   // 出库偏差

    float steering_p_k;  // 转向环动态 P 系数
    float steering_p;    // 转向环 P
    float steering_d;    // 转向环 D

    ControlConfig(std::string dir) : config_dir(dir) {
        json root;
        std::ifstream is(config_dir);
        CHECK(is.good()) << "File: " << config_dir << "not found" << LOG_END;

        is >> root;
        // LOG_I << "Config:  " << root << LOG_END;

/* 在当前节点查找与变量名相同的项并配置 */
#define P_CUR root
#define AUTOCFG(index) index = P_CUR[#index]

        AUTOCFG(threshold);
        AUTOCFG(threshold_detection);

        AUTOCFG(direction_garage);
        AUTOCFG(direction_yroad);
        AUTOCFG(direction_work);
        AUTOCFG(direction_gas);

        AUTOCFG(check_work_);
        AUTOCFG(check_gas_);
        AUTOCFG(check_yroad_);
        AUTOCFG(check_ramp_);
        AUTOCFG(check_circle_);
        AUTOCFG(check_cross_);

        AUTOCFG(ai_p_k);
        AUTOCFG(ai_p);
        AUTOCFG(ai_d);

        AUTOCFG(gas_rou_ready);
        AUTOCFG(gas_rou_run1);
        AUTOCFG(gas_rou_run2);
        AUTOCFG(gas_rou_out1);
        AUTOCFG(gas_rou_out2);
        AUTOCFG(gas_rou_none);
        AUTOCFG(gas_rou_over);
        AUTOCFG(work_rou_ready);
        AUTOCFG(work_rou_run);
        AUTOCFG(work_rou_out);
        AUTOCFG(work_rou_none);
        AUTOCFG(work_rou_over);

        AUTOCFG(speed_level);
        json speed = root[speed_level];

/* 改写当前配置项节点 */
#undef P_CUR
#define P_CUR speed

        AUTOCFG(aim_distance_far);
        AUTOCFG(aim_distance_near);

        AUTOCFG(speed_base);
        AUTOCFG(speed_up);
        AUTOCFG(speed_steady);
        AUTOCFG(speed_garage_in);
        AUTOCFG(speed_garage_out);
        AUTOCFG(speed_work);
        AUTOCFG(speed_gas);
        AUTOCFG(speed_yroad);
        AUTOCFG(speed_circle);
        AUTOCFG(speed_cross);
        AUTOCFG(speed_ramp_up);
        AUTOCFG(speed_ramp_down);

        AUTOCFG(deviation_garage_in_p);
        AUTOCFG(deviation_garage_out);

        AUTOCFG(steering_p_k);
        AUTOCFG(steering_p);
        AUTOCFG(steering_d);

        log_i("Init Success.");
    }
    ~ControlConfig() {}
};

#undef LOG_TAG
#define LOG_TAG "Model Json"

// 模型配置项
struct ModelConfig {
    std::string model_parent_dir;

    std::string network_type;

    std::string model_file;
    std::string params_file;
    std::string model_params_dir;

    std::string format;
    uint16_t input_width;
    uint16_t input_height;

    float means[3];
    float scales[3];
    float threshold;

    bool is_yolo;
    bool is_combined_model;

    std::vector<string> labels;

    void assert_check_file_exist(std::string fileName, std::string modelPath) {
        std::ifstream infile(modelPath + fileName);
        CHECK(infile.good()) << modelPath << fileName << "not exit" << LOG_END;
    }

    ModelConfig(std::string model_path) : model_parent_dir(model_path + "/") {
        std::string json_config_path = model_parent_dir + "config.json";
        std::ifstream is(json_config_path);

        CHECK(is.good()) << json_config_path << "not exit" << LOG_END;

        json value;
        is >> value;
        // LOG_I << "Model:  " << value << LOG_END;

        input_width = value["input_width"];
        input_height = value["input_height"];
        format = value["format"];
        std::transform(format.begin(), format.end(), format.begin(), ::toupper);

        std::vector<float> mean = value["mean"];
        for (uint32_t i = 0; i < mean.size(); ++i) {
            means[i] = mean[i];
        }

        std::vector<float> scale = value["scale"];
        for (uint32_t i = 0; i < scale.size(); ++i) {
            scales[i] = scale[i];
        }

        if (value["threshold"] != nullptr) {
            threshold = value["threshold"];
        } else {
            threshold = 0.9;
            LOG_E << "Json key: threshold not found, default :" << threshold << LOG_END;
        }

        is_yolo = false;
        if (value["network_type"] != nullptr) {
            network_type = value["network_type"];
            if (network_type == "YOLOV3") {
                is_yolo = true;
            }
        }

        if ((value["model_file_name"] != nullptr) &&
            (value["params_file_name"] != nullptr) &&
            (value["model_dir"] == nullptr)) {
            is_combined_model = true;
            params_file =
                model_parent_dir + value["params_file_name"].get<std::string>();
            model_file =
                model_parent_dir + value["model_file_name"].get<std::string>();
            model_params_dir = "";
        } else if ((value["model_file_name"] == nullptr) &&
                   (value["params_file_name"] == nullptr) &&
                   (value["model_dir"] != nullptr)) {
            is_combined_model = false;
            model_params_dir =
                model_parent_dir + value["model_dir"].get<std::string>();
            params_file = "";
            model_file = "";
        } else {
            CHECK(false) << "Json config Error !!!! \n combined_model: "
                            "need params_file_name model_file_name, separate_model: "
                            "need model_dir only."
                         << LOG_END;
        }

        if (value["labels_file_name"] != nullptr) {
            std::string labels_file_name = value["labels_file_name"];
            std::string label_path = model_parent_dir + labels_file_name;
            std::ifstream file(label_path);
            if (file.is_open()) {
                std::string line;
                while (getline(file, line)) {
                    labels.push_back(line);
                }
                file.close();
            } else {
                LOG_E << "Open Lable File failed, file path: " << label_path << LOG_END;
            }
        }

        if (is_combined_model) {
            assert_check_file_exist(value["model_file_name"], model_parent_dir);
            assert_check_file_exist(value["params_file_name"], model_parent_dir);
        } else {
            assert_check_file_exist(value["model_dir"], model_parent_dir);
        }

        log_i("Init Success.");
    }
    ~ModelConfig() {}
};

#undef LOG_TAG
#define LOG_TAG "System Json"

// 获取文件路径
inline std::string get_file_path(const std::string &fname) {
    size_t pos = fname.find_last_of("/");
    return (std::string::npos == pos) ? "" : fname.substr(0, pos);
}

// 系统配置项
struct SystemConfig {
    std::string config_dir;
    std::string model_config_path;

    std::string input_type;
    std::string input_path;

    bool use_fpga_preprocess;

    // for debug
    bool predict_time_log_enable;
    bool predict_log_enable;
    bool display_enable;

    SystemConfig(std::string dir) : config_dir(dir) {
        json value;
        std::ifstream is(config_dir);

        CHECK(is.good()) << "File path:[" << config_dir << "] not find." << LOG_END;

        is >> value;

        // LOG_I << "Config: " << value << LOG_END;

        std::string confiig_path = get_file_path(dir) + "/";
        model_config_path = value["model_config"];
        model_config_path = confiig_path + model_config_path;

        json input_config = value["input"];
        input_type = input_config["type"];

        if (input_config["path"] != nullptr) {
            input_path = input_config["path"];
            if (input_type == "image") {
                input_path = confiig_path + input_path;
            }
        }

        if (value["fpga_preprocess"] == nullptr) {
            use_fpga_preprocess = true;
        } else {
            use_fpga_preprocess = value["fpga_preprocess"];
        }

        if (value["debug"] != nullptr) {
            json debug_config = value["debug"];
            if (debug_config["predict_time_log_enable"] == nullptr) {
                predict_time_log_enable = true;
            } else {
                predict_time_log_enable = debug_config["predict_time_log_enable"];
            }

            if (debug_config["predict_log_enable"] == nullptr) {
                predict_log_enable = true;
            } else {
                predict_log_enable = debug_config["predict_log_enable"];
            }

            if (debug_config["display_enable"] == nullptr) {
                display_enable = true;
            } else {
                display_enable = debug_config["display_enable"];
            }
        } else {
            display_enable = false;
            predict_log_enable = false;
            predict_time_log_enable = false;
        }

        log_i("Init Success.");
    }
    ~SystemConfig() {}
};
