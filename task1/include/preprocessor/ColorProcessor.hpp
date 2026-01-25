#pragma once
#include <opencv2/opencv.hpp>

class ColorProcessor {
public:
    // 转换颜色空间
    static cv::Mat convertColorSpace(const cv::Mat& frame, int code);
    
    // 提取特定颜色
    static cv::Mat extractColor(const cv::Mat& frame, 
                               const cv::Scalar& lower_bound,
                               const cv::Scalar& upper_bound);
    
    // 红蓝颜色分离（针对RM装甲板）
    static void separateRB(const cv::Mat& frame, 
                          cv::Mat& red_binary, 
                          cv::Mat& blue_binary);
};