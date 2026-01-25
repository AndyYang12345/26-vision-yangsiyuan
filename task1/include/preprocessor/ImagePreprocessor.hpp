#pragma once
#include <opencv2/opencv.hpp>
#include "../types/Params.hpp"

class ImagePreprocessor {
public:
    explicit ImagePreprocessor(const ImagePreprocessParams& params);
    
    // 处理单帧图像
    cv::Mat process(const cv::Mat& frame);
    
    // 获取二值化图像
    cv::Mat getBinaryImage() const { return binary_image_; }
    
    // 更新参数
    void updateParams(const ImagePreprocessParams& params);
    
private:
    ImagePreprocessParams params_;
    cv::Mat binary_image_;
    
    // 私有方法
    cv::Mat preprocessImage(const cv::Mat& frame);
    cv::Mat extractColor(const cv::Mat& frame, const cv::Scalar& color_range);
};