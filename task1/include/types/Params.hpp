#pragma once
#include <string>
#include <opencv2/core.hpp>

struct ImagePreprocessParams {
    int gaussian_kernel = 0;           // 高斯核大小
    int binary_threshold = 0;          // 二值化阈值
    bool use_adaptive_thresh = false;  // 是否使用自适应阈值
    // ... 其他参数
};

struct LightBarParams {
    float min_area = 0.0f;                // 最小面积
    float max_area_ratio = 0.0f;          // 最大长宽比
    float min_angle_diff = 0.0f;          // 最小角度差
    // ... 其他参数
};

struct ArmorParams {
    float min_aspect_ratio = 0.0f;        // 最小宽高比
    float max_aspect_ratio = 0.0f;        // 最大宽高比
    float min_lightbar_distance = 0.0f;   // 灯条最小距离
    float max_lightbar_distance = 0.0f;   // 灯条最大距离
    float max_angle_diff = 0.0f;          // 最大角度差
    float max_height_diff_ratio = 0.0f;   // 最大高度差比例
    float max_length_ratio = 0.0f;        // 最大长度比
    float min_lightbar_area = 0.0f;       // 灯条最小面积
    // ... 其他参数
};

struct Params {
    ImagePreprocessParams preprocess;
    LightBarParams lightbar;
    ArmorParams armor;

    int binarize_method = 0;
    cv::Scalar hsv_red_lower1 = cv::Scalar();
    cv::Scalar hsv_red_upper1 = cv::Scalar();
    cv::Scalar hsv_red_lower2 = cv::Scalar();
    cv::Scalar hsv_red_upper2 = cv::Scalar();
    cv::Scalar hsv_blue_lower = cv::Scalar();
    cv::Scalar hsv_blue_upper = cv::Scalar();
    int br_diff_threshold = 0;
    int morph_kernel = 0;
    std::string classifier_model_path = "";
    
    // 从YAML文件加载参数
    static Params loadFromYAML(const std::string& filename);
};
