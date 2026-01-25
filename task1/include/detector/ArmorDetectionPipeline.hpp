// ArmorDetectionPipeline.hpp - 集成所有模块
#pragma once
#include <memory>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "../preprocessor/ImagePreprocessor.hpp"
#include "../solver/PnPSolver.hpp"
#include "../types/Armor.hpp"
#include "../types/Params.hpp"
#include "ArmorClassifier.hpp"
#include "ArmorDetector.hpp"
#include "LightBarDetector.hpp"

struct DetectionResult {
    std::vector<Armor> armors;
    cv::Mat debug_image;
    bool success;
    double processing_time;
};

class ArmorDetectionPipeline {
public:
    ArmorDetectionPipeline(const std::string& config_path);
    
    // 处理单帧图像
    DetectionResult process(const cv::Mat& frame);
    
    // 更新参数
    void updateParams(const Params& params);
    
    // 设置目标颜色
    void setTargetColor(bool is_red);
    
    // 获取处理时间统计
    struct TimingInfo {
        double preprocess_time;
        double detection_time;
        double classification_time;
        double pnp_time;
        double total_time;
    };
    TimingInfo getTimingInfo() const { return timing_info_; }
    
private:
    // 模块组件
    std::unique_ptr<ImagePreprocessor> preprocessor_;
    std::shared_ptr<LightBarDetector> lightbar_detector_;
    std::unique_ptr<ArmorDetector> armor_detector_;
    std::unique_ptr<ArmorClassifier> classifier_;
    std::unique_ptr<PnPSolver> pnp_solver_;
    
    // 参数
    Params params_;
    
    // 时间统计
    TimingInfo timing_info_;
    
    // 私有方法
    void resetTiming();
    void updateTiming(const std::string& stage, double time);
};
