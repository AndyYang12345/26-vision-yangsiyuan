#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include "../types/Armor.hpp"

class ArmorClassifier {
public:
    ArmorClassifier(const std::string& model_path);
    
    // 识别装甲板数字
    ArmorNumber classify(const cv::Mat& armor_roi);
    
    // 批量识别
    void classifyAll(std::vector<Armor>& armors);
    
private:
    cv::dnn::Net net_;
    cv::Size input_size_;
    
    // 预处理ROI图像
    cv::Mat preprocessROI(const cv::Mat& roi);
};
