#include "types/Armor.hpp"
#include <algorithm>

namespace {
std::vector<cv::Point2f> sortCorners(const std::vector<cv::Point2f>& corners) {
    std::vector<cv::Point2f> ordered = corners;
    std::sort(ordered.begin(), ordered.end(),
              [](const cv::Point2f& a, const cv::Point2f& b) {
                  return (a.y < b.y) || (std::abs(a.y - b.y) < 1e-3f && a.x < b.x);
              });

    std::vector<cv::Point2f> top = {ordered[0], ordered[1]};
    std::vector<cv::Point2f> bottom = {ordered[2], ordered[3]};

    if (top[0].x > top[1].x) {
        std::swap(top[0], top[1]);
    }
    if (bottom[0].x > bottom[1].x) {
        std::swap(bottom[0], bottom[1]);
    }

    return {top[0], top[1], bottom[1], bottom[0]};
}
}  // namespace

cv::Point2f Armor::getCenter() const {
    if (corners.size() != 4) {
        return (left_bar.center + right_bar.center) * 0.5f;
    }

    cv::Point2f sum(0.0f, 0.0f);
    for (const auto& pt : corners) {
        sum += pt;
    }
    return sum * 0.25f;
}

void Armor::updateCorners() {
    if (corners.size() != 4) {
        return;
    }
    corners = sortCorners(corners);
}
