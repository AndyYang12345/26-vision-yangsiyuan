#include "detector/LightBarDetector.hpp"
#include <algorithm>
#include <cmath>

namespace {
bool hasValidParams(const LightBarParams& params) {
    return params.min_area > 0.0f && params.max_area_ratio > 0.0f;
}
}  // namespace

LightBarDetector::LightBarDetector(const LightBarParams& params)
    : params_(params) {
    if (!hasValidParams(params_)) {
        params_.min_area = 30.0f;
        params_.max_area_ratio = 10.0f;
        params_.min_angle_diff = 20.0f;
    }
}

void LightBarDetector::updateParams(const LightBarParams& params) {
    params_ = params;
    if (!hasValidParams(params_)) {
        params_.min_area = 30.0f;
        params_.max_area_ratio = 10.0f;
        params_.min_angle_diff = 20.0f;
    }
}

std::vector<LightBar> LightBarDetector::detect(const cv::Mat& binary_image) {
    return findLightBars(binary_image);
}

std::vector<LightBar> LightBarDetector::findLightBars(const cv::Mat& binary_image) {
    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary_image, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<LightBar> lightbars;
    last_contours_.clear();
    lightbars.reserve(contours.size());

    for (const auto& contour : contours) {
        if (!isValidLightBar(contour)) {
            continue;
        }
        last_contours_.push_back(contour);
        lightbars.push_back(contourToLightBar(contour));
    }

    return lightbars;
}

bool LightBarDetector::isValidLightBar(const std::vector<cv::Point>& contour) {
    if (contour.size() < 5) {
        return false;
    }

    double area = cv::contourArea(contour);
    if (area < params_.min_area) {
        return false;
    }

    cv::RotatedRect rect = cv::minAreaRect(contour);
    float w = rect.size.width;
    float h = rect.size.height;
    if (w < 1.0f || h < 1.0f) {
        return false;
    }

    float ratio = std::max(w, h) / std::max(1.0f, std::min(w, h));
    if (ratio > params_.max_area_ratio) {
        return false;
    }

    return true;
}

LightBar LightBarDetector::contourToLightBar(const std::vector<cv::Point>& contour) {
    LightBar bar{};
    bar.rect = cv::minAreaRect(contour);
    bar.center = bar.rect.center;
    bar.length = std::max(bar.rect.size.width, bar.rect.size.height);
    bar.width = std::min(bar.rect.size.width, bar.rect.size.height);

    float angle = bar.rect.angle;
    if (bar.rect.size.width < bar.rect.size.height) {
        angle += 90.0f;
    }
    bar.angle = angle;

    bar.color = cv::Scalar(0, 255, 255);
    return bar;
}
