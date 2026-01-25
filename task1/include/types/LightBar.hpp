#pragma once
#include <opencv2/opencv.hpp>

struct Params;

struct LightBar {
    cv::RotatedRect rect;          // 旋转矩形
    cv::Point2f center;            // 中心点
    float length;                  // 长度
    float width;                   // 宽度
    float angle;                   // 角度
    cv::Scalar color;              // 颜色 (BGR)
    
    // 判断两个灯条是否可能配对成装甲板
    bool canPairWith(const LightBar& other, const Params& params) const;
};
