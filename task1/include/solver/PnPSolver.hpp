#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include "../types/Armor.hpp"

class PnPSolver {
public:
    PnPSolver(const cv::Mat& camera_matrix,
              const cv::Mat& dist_coeffs,
              float small_armor_width = 130.0f,   // 单位：mm
              float small_armor_height = 54.0f,
              float large_armor_width = 230.0f,
              float large_armor_height = 54.0f);
    
    // 求解单个装甲板的位姿
    bool solve(const Armor& armor, 
               cv::Mat& rvec, 
               cv::Mat& tvec,
               bool use_extrinsic_guess = false) const;
    
    // 获取3D点（根据装甲板大小）
    std::vector<cv::Point3f> get3DPoints(ArmorSize size) const;
    
    // 重投影误差评估
    double evaluateReprojectionError(const Armor& armor,
                                     const cv::Mat& rvec,
                                     const cv::Mat& tvec) const;
    
private:
    cv::Mat camera_matrix_;
    cv::Mat dist_coeffs_;
    float small_w_, small_h_;
    float large_w_, large_h_;
};
