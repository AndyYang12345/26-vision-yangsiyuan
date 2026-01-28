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
    
    // ========== 新增成员函数（不改变内存布局） ==========
    
    // 获取灯条的顶部中点
    cv::Point2f getTopPoint() const {
        cv::Point2f vertices[4];
        rect.points(vertices);
        
        // 找出Y值最小的两个点（顶部）
        int min_idx1 = 0, min_idx2 = 1;
        for (int i = 2; i < 4; i++) {
            if (vertices[i].y < vertices[min_idx1].y) {
                min_idx2 = min_idx1;
                min_idx1 = i;
            } else if (vertices[i].y < vertices[min_idx2].y) {
                min_idx2 = i;
            }
        }
        
        return (vertices[min_idx1] + vertices[min_idx2]) * 0.5f;
    }
    
    // 获取灯条的底部中点
    cv::Point2f getBottomPoint() const {
        cv::Point2f vertices[4];
        rect.points(vertices);
        
        // 找出Y值最大的两个点（底部）
        int max_idx1 = 0, max_idx2 = 1;
        for (int i = 2; i < 4; i++) {
            if (vertices[i].y > vertices[max_idx1].y) {
                max_idx2 = max_idx1;
                max_idx1 = i;
            } else if (vertices[i].y > vertices[max_idx2].y) {
                max_idx2 = i;
            }
        }
        
        return (vertices[max_idx1] + vertices[max_idx2]) * 0.5f;
    }
    
    // 获取灯条面积
    float getArea() const {
        return rect.size.area();
    }
    
    // 获取所有角点
    std::vector<cv::Point2f> getAllCorners() const {
        cv::Point2f vertices[4];
        rect.points(vertices);
        return {vertices, vertices + 4};
    }
    
    // 判断两个灯条是否可能配对成装甲板
    bool canPairWith(const LightBar& other, const Params& params) const;
};