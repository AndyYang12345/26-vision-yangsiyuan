#include "detector/ArmorDetector.hpp"
#include <algorithm>
#include <cmath>

namespace {
bool hasValidParams(const ArmorParams& params) {
    return params.min_aspect_ratio > 0.0f &&
           params.max_aspect_ratio > 0.0f &&
           params.min_lightbar_distance > 0.0f &&
           params.max_lightbar_distance > 0.0f &&
           params.max_angle_diff > 0.0f &&
           params.max_height_diff_ratio > 0.0f &&
           params.max_length_ratio > 0.0f &&
           params.min_lightbar_area > 0.0f;
}

std::vector<cv::Point3f> buildModelPoints(ArmorSize size) {
    const float small_w = 135.0f;
    const float small_h = 55.0f;
    const float large_w = 230.0f;
    const float large_h = 55.0f;
    const float w = (size == LARGE_ARMOR) ? large_w : small_w;
    const float h = (size == LARGE_ARMOR) ? large_h : small_h;
    const float half_w = w * 0.5f;
    const float half_h = h * 0.5f;
    return {
        {-half_w, -half_h, 0.0f},
        { half_w, -half_h, 0.0f},
        { half_w,  half_h, 0.0f},
        {-half_w,  half_h, 0.0f}
    };
}
}  // namespace

ArmorDetector::ArmorDetector(const ArmorParams& params)
    : params_(params) {
    if (!hasValidParams(params_)) {
        // 基础几何参数
        params_.min_aspect_ratio = 1.6f;
        params_.max_aspect_ratio = 5.0f;
        params_.min_lightbar_distance = 0.5f;    // 相对距离
        params_.max_lightbar_distance = 6.0f;    // 相对距离
        params_.max_angle_diff = 15.0f;
        params_.max_height_diff_ratio = 0.6f;
        params_.max_length_ratio = 1.5f;
        params_.min_lightbar_area = 30.0f;
        
        // ========== 第一代抗幽灵参数 ==========
        params_.max_cross_angle = 40.0f;          // 交叉角限制
        params_.min_symmetry_score = 0.9f;        // 对称性检查（保持你的0.9）
        params_.min_parallel_score = 0.9f;        // 平行度检查（保持你的0.9）
        params_.max_center_offset_ratio = 0.4f;   // 中心偏移
        
        // ========== 新增：第二代抗幽灵参数（梯形特征检测） ==========
        // 关键参数：高度一致性检查，专门针对幽灵装甲板的梯形特征
        params_.min_height_consistency = 0.9f;    // 默认0.7，范围0.5-0.9
        
        // 辅助参数：对角线比例检查，验证矩形特征
        params_.max_diagonal_ratio = 0.9f;        // 默认0.7，范围0.5-0.9
    }
}

void ArmorDetector::updateParams(const ArmorParams& params) {
    params_ = params;
    if (!hasValidParams(params_)) {
        // 基础几何参数（与构造函数保持一致）
        params_.min_aspect_ratio = 1.6f;
        params_.max_aspect_ratio = 5.0f;
        params_.min_lightbar_distance = 0.5f;
        params_.max_lightbar_distance = 6.0f;
        params_.max_angle_diff = 15.0f;
        params_.max_height_diff_ratio = 0.6f;
        params_.max_length_ratio = 1.5f;
        params_.min_lightbar_area = 30.0f;
        
        // ========== 第一代抗幽灵参数 ==========
        params_.max_cross_angle = 40.0f;
        params_.min_symmetry_score = 0.9f;    // 注意：这里要改成0.9，与构造函数一致
        params_.min_parallel_score = 0.9f;    // 注意：这里要改成0.9，与构造函数一致
        params_.max_center_offset_ratio = 0.4f;
        
        // ========== 新增：第二代抗幽灵参数 ==========
        params_.min_height_consistency = 0.7f;
        params_.max_diagonal_ratio = 0.7f;
    }
}

std::vector<Armor> ArmorDetector::match(const std::vector<LightBar>& lightbars) const {
    std::vector<Armor> armors;
    if (lightbars.size() < 2) {
        return armors;
    }

    for (size_t i = 0; i < lightbars.size(); ++i) {
        for (size_t j = i + 1; j < lightbars.size(); ++j) {
            const LightBar& a = lightbars[i];
            const LightBar& b = lightbars[j];

            const LightBar* left = &a;
            const LightBar* right = &b;
            if (a.center.x > b.center.x) {
                left = &b;
                right = &a;
            }

            if (!isValidPair(*left, *right)) {
                continue;
            }

            armors.push_back(buildArmor(*left, *right));
        }
    }

    return armors;
}

bool ArmorDetector::isValidPair(const LightBar& left, const LightBar& right) const {
    const float angle_diff = std::abs(left.angle - right.angle);
    const float mean_length = 0.5f * (left.length + right.length);
    const float center_dist = cv::norm(left.center - right.center);
    const float left_area = left.length * left.width;
    const float right_area = right.length * right.width;

    const float height_diff = std::abs(left.center.y - right.center.y);
    const float height_ratio = height_diff / std::max(1.0f, mean_length);
    const float distance_ratio = center_dist / std::max(1.0f, mean_length);
    const float length_ratio = std::max(left.length, right.length) / std::max(1.0f, std::min(left.length, right.length));

    if (left_area < params_.min_lightbar_area || right_area < params_.min_lightbar_area) {
        return false;
    }

    if (angle_diff > params_.max_angle_diff) {
        return false;
    }

    if (height_ratio > params_.max_height_diff_ratio) {
        return false;
    }

    if (distance_ratio < params_.min_lightbar_distance) {
        return false;
    }

    if (distance_ratio > params_.max_lightbar_distance) {
        return false;
    }

    if (length_ratio > params_.max_length_ratio) {
        return false;
    }

    const float armor_width = center_dist;
    const float armor_height = mean_length;
    const float aspect = armor_width / std::max(1.0f, armor_height);

    return aspect >= params_.min_aspect_ratio && aspect <= params_.max_aspect_ratio;
}

Armor ArmorDetector::buildArmor(const LightBar& left, const LightBar& right) const {
    Armor armor{};
    armor.left_bar = left;
    armor.right_bar = right;
    armor.size = (cv::norm(left.center - right.center) > 160.0f) ? LARGE_ARMOR : SMALL_ARMOR;
    armor.number = UNKNOWN;
    armor.confidence = 0.5f;
    armor.color = left.color;

    std::vector<cv::Point2f> points;
    cv::Point2f left_pts[4];
    cv::Point2f right_pts[4];
    left.rect.points(left_pts);
    right.rect.points(right_pts);
    for (const auto& pt : left_pts) {
        points.push_back(pt);
    }
    for (const auto& pt : right_pts) {
        points.push_back(pt);
    }

    cv::RotatedRect armor_rect = cv::minAreaRect(points);
    cv::Point2f armor_pts[4];
    armor_rect.points(armor_pts);

    armor.corners.assign(armor_pts, armor_pts + 4);
    armor.updateCorners();

    armor.roi = cv::boundingRect(armor.corners);
    armor.model_points = buildModelPoints(armor.size);
    return armor;
}
