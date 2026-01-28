#include "types/LightBar.hpp"
#include "types/Params.hpp"
#include <cmath>

bool LightBar::canPairWith(const LightBar& other, const Params& params) const {
    const auto& armor_params = params.armor;  // 使用统一的参数结构
    
    // ========== 1. 基础面积过滤（防止噪声干扰） ==========
    if (armor_params.min_lightbar_area > 0.0f) {
        const float area1 = length * (length * 0.3f);  // 近似面积 = 长 × 宽(假设宽是长的30%)
        const float area2 = other.length * (other.length * 0.3f);
        if (area1 < armor_params.min_lightbar_area || 
            area2 < armor_params.min_lightbar_area) {
            return false;  // 灯条面积太小，可能是噪声
        }
    }
    
    // ========== 2. 角度一致性检查 ==========
    if (armor_params.max_angle_diff > 0.0f) {
        const float angle_diff = std::abs(angle - other.angle);
        
        // 处理角度周期性问题（180°=0°）
        float normalized_diff = std::fmod(angle_diff, 180.0f);
        if (normalized_diff > 90.0f) {
            normalized_diff = 180.0f - normalized_diff;
        }
        
        if (normalized_diff > armor_params.max_angle_diff) {
            return false;  // 角度差超过阈值
        }
    }
    
    // ========== 3. 长度比例检查 ==========
    if (armor_params.max_length_ratio > 0.0f) {
        const float length_ratio = std::max(length, other.length) / 
                                   std::min(length, other.length + 0.001f);  // 防止除零
        if (length_ratio > armor_params.max_length_ratio) {
            return false;  // 长度差异过大
        }
    }
    
    // ========== 4. 距离约束（核心） ==========
    const float center_dist = cv::norm(center - other.center);
    const float mean_length = (length + other.length) * 0.5f;
    
    // 4.1 最小相对距离约束
    if (armor_params.min_lightbar_distance > 0.0f) {
        if (center_dist / std::max(1.0f, mean_length) < armor_params.min_lightbar_distance) {
            return false;  // 距离太近，可能是同一灯条的误分割
        }
    }
    
    // 4.2 最大绝对距离约束（新增）
    if (armor_params.max_lightbar_distance > 0.0f) {
        if (center_dist > armor_params.max_lightbar_distance) {
            return false;  // 距离太远，不可能是同一装甲板
        }
    }
    
    // ========== 5. 高度对齐检查 ==========
    if (armor_params.max_height_diff_ratio > 0.0f) {
        const float y_diff = std::abs(center.y - other.center.y);
        if (mean_length > 0.1f) {  // 避免极小灯条导致的除零
            const float height_diff_ratio = y_diff / mean_length;
            if (height_diff_ratio > armor_params.max_height_diff_ratio) {
                return false;  // Y方向未对齐
            }
        }
    }
    
    // ========== 6. 宽高比预测与检查（重要！） ==========
    if (armor_params.min_aspect_ratio > 0.0f || armor_params.max_aspect_ratio > 0.0f) {
        // 计算预测的装甲板区域
        const float armor_width = center_dist;
        const float armor_height = mean_length;
        
        if (armor_height > 0.1f) {  // 避免除零
            const float aspect_ratio = armor_width / armor_height;
            
            // 小装甲板（英雄/哨兵）和大装甲板（步兵）有不同的宽高比
            const bool within_range = 
                (armor_params.min_aspect_ratio <= 0.0f || aspect_ratio >= armor_params.min_aspect_ratio) &&
                (armor_params.max_aspect_ratio <= 0.0f || aspect_ratio <= armor_params.max_aspect_ratio);
            
            if (!within_range) {
                return false;  // 宽高比不符合装甲板特征
            }
        }
    }
    
    // ========== 7. 方向平行度检查（使用向量点积） ==========
    // 即使角度差合格，也检查方向是否真的平行
    const float rad1 = angle * CV_PI / 180.0f;
    const float rad2 = other.angle * CV_PI / 180.0f;
    
    cv::Point2f dir1(std::cos(rad1), std::sin(rad1));
    cv::Point2f dir2(std::cos(rad2), std::sin(rad2));
    
    const float dot_product = std::abs(dir1.dot(dir2));  // 绝对值，方向相反也算平行
    const float parallel_threshold = 0.9f;  // cos(25°) ≈ 0.9
    
    if (dot_product < parallel_threshold) {
        return false;  // 不够平行
    }
    
    // ========== 8. X方向顺序检查（防止左右颠倒配对） ==========
    // 确保按正确的左右顺序配对
    const bool correct_order = 
        (center.x < other.center.x && angle > 0) ||  // 左灯条向右倾斜
        (center.x > other.center.x && angle < 0);    // 右灯条向左倾斜
    
    if (!correct_order && std::abs(angle) > 10.0f) {  // 角度较大时才检查
        return false;  // 灯条倾斜方向与位置不匹配
    }
    
    return true;  // 通过所有检查
}