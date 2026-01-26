#include "types/LightBar.hpp"
#include "types/Params.hpp"
#include <cmath>

bool LightBar::canPairWith(const LightBar& other, const Params& params) const {
    const float angle_diff = std::abs(angle - other.angle);
    if (params.lightbar.min_angle_diff > 0.0f && angle_diff > params.lightbar.min_angle_diff) {
        return false;
    }

    const float center_dist = cv::norm(center - other.center);
    const float mean_length = 0.5f * (length + other.length);
    if (params.armor.min_lightbar_distance > 0.0f &&
        center_dist / std::max(1.0f, mean_length) < params.armor.min_lightbar_distance) {
        return false;
    }

    return true;
}
