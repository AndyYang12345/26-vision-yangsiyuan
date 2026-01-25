#pragma once
#include <string>
#include <opencv2/core.hpp>

struct ImagePreprocessParams {
    int gaussian_kernel;           // 高斯核大小
    int binary_threshold;          // 二值化阈值
    bool use_adaptive_thresh;      // 是否使用自适应阈值
    // ... 其他参数
};

struct LightBarParams {
    float min_area;                // 最小面积
    float max_area_ratio;          // 最大长宽比
    float min_angle_diff;          // 最小角度差
    // ... 其他参数
};

struct ArmorParams {
    float min_aspect_ratio;        // 最小宽高比
    float max_aspect_ratio;        // 最大宽高比
    float min_lightbar_distance;   // 灯条最小距离
    // ... 其他参数
};

struct Params {
    ImagePreprocessParams preprocess;
    LightBarParams lightbar;
    ArmorParams armor;
    
    // 从YAML文件加载参数
    static Params loadFromYAML(const std::string& filename);
};
