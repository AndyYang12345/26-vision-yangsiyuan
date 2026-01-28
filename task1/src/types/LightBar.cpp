#include "types/LightBar.hpp"
#include "types/Params.hpp"
#include <cmath>

// LightBar结构体的成员函数实现
bool LightBar::canPairWith(const LightBar& other, const Params& params) const {
    const auto& armor = params.armor;
    
    // 1. 基础面积过滤
    if (armor.min_lightbar_area > 0.0f) {
        if (getArea() < armor.min_lightbar_area || other.getArea() < armor.min_lightbar_area) {
            return false;
        }
    }
    
    // 2. 角度一致性检查
    if (armor.max_angle_diff > 0.0f) {
        float angle_diff = std::abs(angle - other.angle);
        angle_diff = std::min(angle_diff, 180.0f - angle_diff);
        
        if (angle_diff > armor.max_angle_diff) {
            return false;
        }
    }
    
    // 3. 长度比例检查
    if (armor.max_length_ratio > 0.0f) {
        const float length_ratio = std::max(length, other.length) / 
                                   std::max(1.0f, std::min(length, other.length));
        if (length_ratio > armor.max_length_ratio) {
            return false;
        }
    }
    
    // 4. 距离约束
    const float center_dist = cv::norm(center - other.center);
    const float mean_length = (length + other.length) * 0.5f;
    
    if (armor.min_lightbar_distance > 0.0f && mean_length > 1.0f) {
        if (center_dist / mean_length < armor.min_lightbar_distance) {
            return false;
        }
    }
    
    if (armor.max_lightbar_distance > 0.0f && mean_length > 1.0f) {
        if (center_dist / mean_length > armor.max_lightbar_distance) {
            return false;
        }
    }
    
    // 5. 高度对齐检查
    if (armor.max_height_diff_ratio > 0.0f && mean_length > 1.0f) {
        const float y_diff = std::abs(center.y - other.center.y);
        if (y_diff / mean_length > armor.max_height_diff_ratio) {
            return false;
        }
    }
    
    // 6. 宽高比检查
    if (mean_length > 1.0f) {
        const float armor_width = center_dist;
        const float armor_height = mean_length;
        const float aspect_ratio = armor_width / armor_height;
        
        if ((armor.min_aspect_ratio > 0.0f && aspect_ratio < armor.min_aspect_ratio) ||
            (armor.max_aspect_ratio > 0.0f && aspect_ratio > armor.max_aspect_ratio)) {
            return false;
        }
    }
    
    // 7. 关键：梯形特征检测（高度一致性检查）
    if (armor.min_height_consistency > 0.0f) {
        const cv::Point2f top1 = getTopPoint();
        const cv::Point2f top2 = other.getTopPoint();
        const cv::Point2f bottom1 = getBottomPoint();
        const cv::Point2f bottom2 = other.getBottomPoint();
        
        const float top_y_diff = std::abs(top1.y - top2.y);
        const float bottom_y_diff = std::abs(bottom1.y - bottom2.y);
        
        if (std::max(top_y_diff, bottom_y_diff) > 3.0f) {
            const float height_consistency = std::min(top_y_diff, bottom_y_diff) / 
                                           std::max(top_y_diff, bottom_y_diff);
            
            if (height_consistency < armor.min_height_consistency) {
                return false; // 梯形特征，幽灵装甲板
            }
        }
    }
    
    // 8. 对角线比例检查
    if (armor.max_diagonal_ratio > 0.0f) {
        const float diag1 = cv::norm(getTopPoint() - other.getBottomPoint());
        const float diag2 = cv::norm(other.getTopPoint() - getBottomPoint());
        
        if (std::max(diag1, diag2) > 10.0f) {
            const float diagonal_ratio = std::min(diag1, diag2) / std::max(diag1, diag2);
            if (diagonal_ratio < armor.max_diagonal_ratio) {
                return false;
            }
        }
    }
    
    // 9. 面积比例检查
    const float area1 = getArea();
    const float area2 = other.getArea();
    if (std::max(area1, area2) > 10.0f) {
        const float area_ratio = std::min(area1, area2) / std::max(area1, area2);
        if (area_ratio < 0.5f) {
            return false;
        }
    }
    
    // 10. 交叉角检查（可选）
    if (armor.max_cross_angle > 0.0f) {
        const cv::Point2f line_vec = other.center - center;
        const float line_angle = std::atan2(line_vec.y, line_vec.x) * 180.0f / CV_PI;
        
        float cross_angle1 = std::fmod(std::abs(line_angle - angle), 180.0f);
        float cross_angle2 = std::fmod(std::abs(line_angle - other.angle), 180.0f);
        cross_angle1 = std::min(cross_angle1, 180.0f - cross_angle1);
        cross_angle2 = std::min(cross_angle2, 180.0f - cross_angle2);
        
        if (std::abs(cross_angle1 - 90.0f) > armor.max_cross_angle ||
            std::abs(cross_angle2 - 90.0f) > armor.max_cross_angle) {
            return false;
        }
    }
    
    // 11. 平行度检查
    if (armor.min_parallel_score > 0.0f) {
        const float rad1 = angle * CV_PI / 180.0f;
        const float rad2 = other.angle * CV_PI / 180.0f;
        
        const cv::Point2f dir1(std::cos(rad1), std::sin(rad1));
        const cv::Point2f dir2(std::cos(rad2), std::sin(rad2));
        
        const float parallel_score = std::abs(dir1.dot(dir2));
        if (parallel_score < armor.min_parallel_score) {
            return false;
        }
    }
    
    return true;
}