#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include "../types/LightBar.hpp"
#include "../types/Params.hpp"

class LightBarDetector {
public:
    explicit LightBarDetector(const LightBarParams& params);
    
    // 从二值图像中检测灯条
    std::vector<LightBar> detect(const cv::Mat& binary_image);

    // 获取最近一次检测的轮廓
    const std::vector<std::vector<cv::Point>>& getLastContours() const { return last_contours_; }
    
    // 更新参数
    void updateParams(const LightBarParams& params);
    
private:
    LightBarParams params_;
    std::vector<std::vector<cv::Point>> last_contours_;
    
    // 私有方法
    std::vector<LightBar> findLightBars(const cv::Mat& binary_image);
    bool isValidLightBar(const std::vector<cv::Point>& contour);
    LightBar contourToLightBar(const std::vector<cv::Point>& contour);
};
