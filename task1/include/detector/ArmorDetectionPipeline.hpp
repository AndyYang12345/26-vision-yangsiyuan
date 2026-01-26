// ArmorDetectionPipeline.hpp - 简化版流水线
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "../preprocessor/ColorProcessor.hpp"
#include "../types/Armor.hpp"
#include "../types/Params.hpp"
#include "ArmorClassifier.hpp"
#include "ArmorDetector.hpp"
#include "LightBarDetector.hpp"

struct PreprocessResult {
    cv::Mat original;
    cv::Mat binary_raw;
    cv::Mat binary;
};

struct LightBarDetectionResult {
    std::vector<std::vector<cv::Point>> contours;
    std::vector<LightBar> lightbars;
};

struct ArmorDetectionResult {
    std::vector<Armor> armors;
};

struct DetectionResult {
    bool success = false;
    double processing_time = 0.0;
    PreprocessResult preprocess;
    LightBarDetectionResult lightbars;
    ArmorDetectionResult armors;
    cv::Mat contours_vis;
    cv::Mat lightbars_vis;
    cv::Mat armors_vis;
    cv::Mat number_roi;
};

class ArmorDetectionPipeline {
public:
    explicit ArmorDetectionPipeline(const Params& params = Params(),
                                    const std::string& classifier_model_path = "");

    DetectionResult process(const cv::Mat& frame, bool enemy_is_red);
    void updateParams(const Params& params);
    void updateClassifier(const std::string& model_path);

private:
    Params params_;
    LightBarDetector lightbar_detector_;
    ArmorDetector armor_detector_;
    std::unique_ptr<ArmorClassifier> classifier_;

    void ensureDefaultParams();
    cv::Mat binarizeHSV(const cv::Mat& frame, bool enemy_is_red) const;
    cv::Mat binarizeBRDiff(const cv::Mat& frame, bool enemy_is_red) const;
    cv::Mat applyMorphology(const cv::Mat& binary) const;
    cv::Mat drawContours(const cv::Mat& base,
                         const std::vector<std::vector<cv::Point>>& contours) const;
    cv::Mat drawLightBars(const cv::Mat& base,
                          const std::vector<LightBar>& lightbars) const;
    cv::Mat drawArmors(const cv::Mat& base,
                       const std::vector<Armor>& armors) const;
    cv::Rect clampRect(const cv::Rect& rect, const cv::Size& size) const;
    std::string armorNumberToString(ArmorNumber number) const;
};
