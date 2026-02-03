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
    // 基础几何参数
    float min_aspect_ratio = 0.0f;
    float max_aspect_ratio = 0.0f;
    float min_lightbar_distance = 0.0f;
    float max_lightbar_distance = 0.0f;
    float max_angle_diff = 0.0f;
    float max_height_diff_ratio = 0.0f;
    float max_length_ratio = 0.0f;
    float min_lightbar_area = 0.0f;
    
    // 第一代抗幽灵参数（已存在）
    float max_cross_angle = 0.0f;
    float min_symmetry_score = 0.0f;
    float min_parallel_score = 0.0f;
    float max_center_offset_ratio = 0.0f;
    
    // 新增：第二代抗幽灵参数（梯形特征检测）
    float min_height_consistency = 0.0f;  // 高度一致性（梯形检测）
    float max_diagonal_ratio = 0.0f;      // 最大对角线比例
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
    
};
