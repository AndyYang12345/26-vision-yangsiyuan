#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include "LightBar.hpp"

enum ArmorSize { SMALL_ARMOR, LARGE_ARMOR };
enum ArmorNumber { UNKNOWN, NUMBER_1, NUMBER_2, /* ... */ };

struct Armor {
    LightBar left_bar;             // 左灯条
    LightBar right_bar;            // 右灯条
    std::vector<cv::Point2f> corners; // 4个角点 (左上, 右上, 右下, 左下)
    ArmorSize size;                // 大小装甲板
    ArmorNumber number;            // 装甲板数字
    float confidence;              // 置信度
    cv::Rect roi;                  // 感兴趣区域
    
    // 获取装甲板中心点
    cv::Point2f getCenter() const;
    
    // 更新角点坐标
    void updateCorners();
};
