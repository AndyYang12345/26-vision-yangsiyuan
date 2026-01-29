#include "detector/ArmorDetectionPipeline.hpp"
#include <algorithm>
#include <chrono>
#include <cstdio>

ArmorDetectionPipeline::ArmorDetectionPipeline(const Params& params,
                                               const std::string& classifier_model_path)
    : params_(params)
    , camera_matrix_((cv::Mat_<double>(3, 3) << 1600.0, 0.0, 360.0,
                                              0.0, 1600.0, 240.0,
                                              0.0, 0.0, 1.0))
    , dist_coeffs_(cv::Mat::zeros(1, 5, CV_64F))
    , rvec_cw_(cv::Mat::zeros(3, 1, CV_64F))
    , tvec_cw_(cv::Mat::zeros(3, 1, CV_64F))
    , rotation_cw_(cv::Mat::eye(3, 3, CV_64F))
    , rotation_wc_(cv::Mat::eye(3, 3, CV_64F))
    , tvec_wc_((cv::Mat_<double>(3, 1) << 50.0, 0.0, 0.0))
    , pnp_solver_(camera_matrix_, dist_coeffs_, 130.0f, 54.0f, 230.0f, 54.0f)
    , lightbar_detector_(params.lightbar)
    , armor_detector_(params.armor) {
    ensureDefaultParams();
    rotation_cw_ = (cv::Mat_<double>(3, 3) << 0.0, 1.0, 0.0,
                                             0.0, 0.0, -1.0,
                                             -1.0, 0.0, 0.0);
    rotation_wc_ = rotation_cw_.t();
    tvec_cw_ = -rotation_cw_ * tvec_wc_;
    cv::Rodrigues(rotation_cw_, rvec_cw_);
    if (!classifier_model_path.empty()) {
        classifier_ = std::make_unique<ArmorClassifier>(classifier_model_path);
    } else if (!params_.classifier_model_path.empty()) {
        classifier_ = std::make_unique<ArmorClassifier>(params_.classifier_model_path);
    }
}

void ArmorDetectionPipeline::updateParams(const Params& params) {
    params_ = params;
    ensureDefaultParams();
    lightbar_detector_.updateParams(params_.lightbar);
    armor_detector_.updateParams(params_.armor);
}

void ArmorDetectionPipeline::updateClassifier(const std::string& model_path) {
    if (model_path.empty()) {
        classifier_.reset();
        return;
    }
    classifier_ = std::make_unique<ArmorClassifier>(model_path);
}

void ArmorDetectionPipeline::ensureDefaultParams() {
    if (params_.binarize_method != 1) {
        params_.binarize_method = 0;
    }
    if (params_.hsv_red_lower1 == cv::Scalar()) {
        params_.hsv_red_lower1 = cv::Scalar(0, 100, 100);
    }
    if (params_.hsv_red_upper1 == cv::Scalar()) {
        params_.hsv_red_upper1 = cv::Scalar(10, 255, 255);
    }
    if (params_.hsv_red_lower2 == cv::Scalar()) {
        params_.hsv_red_lower2 = cv::Scalar(170, 100, 100);
    }
    if (params_.hsv_red_upper2 == cv::Scalar()) {
        params_.hsv_red_upper2 = cv::Scalar(180, 255, 255);
    }
    if (params_.hsv_blue_lower == cv::Scalar()) {
        params_.hsv_blue_lower = cv::Scalar(90, 100, 100);
    }
    if (params_.hsv_blue_upper == cv::Scalar()) {
        params_.hsv_blue_upper = cv::Scalar(130, 255, 255);
    }
    if (params_.br_diff_threshold <= 0) {
        params_.br_diff_threshold = 40;
    }
    if (params_.morph_kernel <= 0) {
        params_.morph_kernel = 3;
    }
    if (params_.lightbar.min_area <= 0.0f) {
        params_.lightbar.min_area = 30.0f;
    }
    if (params_.lightbar.max_area_ratio <= 0.0f) {
        params_.lightbar.max_area_ratio = 10.0f;
    }
    if (params_.lightbar.min_angle_diff <= 0.0f) {
        params_.lightbar.min_angle_diff = 20.0f;
    }
    if (params_.armor.min_aspect_ratio <= 0.0f) {
        params_.armor.min_aspect_ratio = 1.6f;
    }
    if (params_.armor.max_aspect_ratio <= 0.0f) {
        params_.armor.max_aspect_ratio = 5.0f;
    }
    if (params_.armor.min_lightbar_distance <= 0.0f) {
        params_.armor.min_lightbar_distance = 0.5f;
    }
    if (params_.armor.max_lightbar_distance <= 0.0f) {
        params_.armor.max_lightbar_distance = 6.0f;
    }
    if (params_.armor.max_angle_diff <= 0.0f) {
        params_.armor.max_angle_diff = 5.0f;
    }
    if (params_.armor.max_height_diff_ratio <= 0.0f) {
        params_.armor.max_height_diff_ratio = 0.6f;
    }
    if (params_.armor.max_length_ratio <= 0.0f) {
        params_.armor.max_length_ratio = 1.5f;
    }
    if (params_.armor.min_lightbar_area <= 0.0f) {
        params_.armor.min_lightbar_area = 30.0f;
    }
}

DetectionResult ArmorDetectionPipeline::process(const cv::Mat& frame, bool enemy_is_red) {
    DetectionResult result;
    if (frame.empty()) {
        return result;
    }

    auto start = std::chrono::steady_clock::now();

    result.preprocess.original = frame.clone();
    if (params_.binarize_method == 1) {
        result.preprocess.binary_raw = binarizeBRDiff(frame, enemy_is_red);
    } else {
        result.preprocess.binary_raw = binarizeHSV(frame, enemy_is_red);
    }
    result.preprocess.binary = applyMorphology(result.preprocess.binary_raw);
    if (!result.preprocess.binary.empty()) {
        const double white_ratio =
            static_cast<double>(cv::countNonZero(result.preprocess.binary)) /
            static_cast<double>(result.preprocess.binary.total());
        if (white_ratio > 0.9 || white_ratio < 0.001) {
            result.preprocess.binary = result.preprocess.binary_raw.clone();
        }
    }
    if (!result.preprocess.binary.empty()) {
        const double min_area = std::max(1.0f, params_.lightbar.min_area);
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(result.preprocess.binary, contours, hierarchy,
                         cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        cv::Mat filtered = cv::Mat::zeros(result.preprocess.binary.size(), CV_8UC1);
        for (const auto& contour : contours) {
            if (cv::contourArea(contour) >= min_area) {
                cv::drawContours(filtered, std::vector<std::vector<cv::Point>>{contour},
                                 -1, cv::Scalar(255), cv::FILLED);
            }
        }
        result.preprocess.binary = filtered;
    }

    result.lightbars.lightbars = lightbar_detector_.detect(result.preprocess.binary);
    result.lightbars.contours = lightbar_detector_.getLastContours();
    const cv::Scalar target_color = enemy_is_red ? cv::Scalar(0, 0, 255) : cv::Scalar(255, 0, 0);
    for (auto& bar : result.lightbars.lightbars) {
        bar.color = target_color;
    }

    result.armors.armors = armor_detector_.match(result.lightbars.lightbars);

    if (!result.armors.armors.empty()) {
        auto best = std::max_element(result.armors.armors.begin(),
                                     result.armors.armors.end(),
                                     [](const Armor& a, const Armor& b) {
                                         return a.roi.area() < b.roi.area();
                                     });
        cv::Rect roi = clampRect(best->roi, frame.size());
        if (roi.area() > 0) {
            result.number_roi = frame(roi).clone();
            if (classifier_) {
                best->number = classifier_->classify(result.number_roi);
            }
        }
    }

    result.contours_vis = drawContours(result.preprocess.binary_raw, result.lightbars.contours);
    result.lightbars_vis = drawLightBars(frame, result.lightbars.lightbars);
    result.armors_vis = drawArmors(frame, result.armors.armors);

    auto end = std::chrono::steady_clock::now();
    result.processing_time = std::chrono::duration<double, std::milli>(end - start).count();
    result.success = true;
    return result;
}

cv::Mat ArmorDetectionPipeline::binarizeHSV(const cv::Mat& frame, bool enemy_is_red) const {
    cv::Mat hsv;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
    cv::Mat binary;
    if (enemy_is_red) {
        cv::Mat mask1;
        cv::Mat mask2;
        cv::inRange(hsv, params_.hsv_red_lower1, params_.hsv_red_upper1, mask1);
        cv::inRange(hsv, params_.hsv_red_lower2, params_.hsv_red_upper2, mask2);
        cv::bitwise_or(mask1, mask2, binary);
    } else {
        cv::inRange(hsv, params_.hsv_blue_lower, params_.hsv_blue_upper, binary);
    }
    return binary;
}

cv::Mat ArmorDetectionPipeline::binarizeBRDiff(const cv::Mat& frame, bool enemy_is_red) const {
    std::vector<cv::Mat> channels;
    cv::split(frame, channels);
    auto makeBinary = [&](bool red_target) {
        cv::Mat diff;
        if (red_target) {
            cv::subtract(channels[2], channels[0], diff);
        } else {
            cv::subtract(channels[0], channels[2], diff);
        }
        cv::Mat binary;
        cv::threshold(diff, binary, params_.br_diff_threshold, 255, cv::THRESH_BINARY);
        return binary;
    };

    cv::Mat binary = makeBinary(enemy_is_red);
    if (!binary.empty()) {
        const double white_ratio =
            static_cast<double>(cv::countNonZero(binary)) /
            static_cast<double>(binary.total());
        if (white_ratio < 0.001) {
            cv::Mat alt = makeBinary(!enemy_is_red);
            if (!alt.empty()) {
                const double alt_ratio =
                    static_cast<double>(cv::countNonZero(alt)) /
                    static_cast<double>(alt.total());
                if (alt_ratio > white_ratio) {
                    binary = alt;
                }
            }
        }
    }
    return binary;
}

cv::Mat ArmorDetectionPipeline::applyMorphology(const cv::Mat& binary) const {
    if (binary.empty()) {
        return binary;
    }
    int k = std::max(1, params_.morph_kernel);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(k, k));
    cv::Mat result;
    cv::morphologyEx(binary, result, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(result, result, cv::MORPH_OPEN, kernel);
    return result;
}

cv::Mat ArmorDetectionPipeline::drawContours(const cv::Mat& base,
                                             const std::vector<std::vector<cv::Point>>& contours) const {
    cv::Mat vis;
    if (base.channels() == 1) {
        cv::cvtColor(base, vis, cv::COLOR_GRAY2BGR);
    } else {
        vis = base.clone();
    }
    cv::drawContours(vis, contours, -1, cv::Scalar(0, 255, 0), 1);
    return vis;
}

cv::Mat ArmorDetectionPipeline::drawLightBars(const cv::Mat& base,
                                              const std::vector<LightBar>& lightbars) const {
    cv::Mat vis = base.clone();
    for (const auto& bar : lightbars) {
        cv::Point2f pts[4];
        bar.rect.points(pts);
        for (int i = 0; i < 4; ++i) {
            cv::line(vis, pts[i], pts[(i + 1) % 4], cv::Scalar(0, 255, 255), 1);
        }
    }
    return vis;
}

cv::Mat ArmorDetectionPipeline::drawArmors(const cv::Mat& base,
                                           const std::vector<Armor>& armors) const {
    cv::Mat vis = base.clone();
    for (const auto& armor : armors) {
        if (armor.corners.size() == 4) {
            for (int i = 0; i < 4; ++i) {
                cv::line(vis, armor.corners[i], armor.corners[(i + 1) % 4],
                         cv::Scalar(0, 255, 0), 2);
            }
            cv::line(vis, armor.corners[0], armor.corners[2], cv::Scalar(0, 255, 0), 1);
            cv::line(vis, armor.corners[1], armor.corners[3], cv::Scalar(0, 255, 0), 1);

            cv::Mat rvec;
            cv::Mat tvec;
            if (pnp_solver_.solve(armor, rvec, tvec)) {
                drawAxes(vis, rvec, tvec);
                cv::Mat tvec_world = rotation_wc_ * tvec + tvec_wc_;
                const cv::Point2f center = armor.getCenter();
                char text[128];
                std::snprintf(text, sizeof(text), "W:(%.1f,%.1f,%.1f)",
                              tvec_world.at<double>(0),
                              tvec_world.at<double>(1),
                              tvec_world.at<double>(2));
                cv::putText(vis, text, center + cv::Point2f(6.0f, -6.0f),
                            cv::FONT_HERSHEY_SIMPLEX, 0.5,
                            cv::Scalar(0, 200, 255), 1);
            }
        } else if (armor.roi.area() > 0) {
            cv::rectangle(vis, armor.roi, cv::Scalar(0, 255, 0), 2);
            cv::Point2f tl(armor.roi.x, armor.roi.y);
            cv::Point2f br(armor.roi.x + armor.roi.width, armor.roi.y + armor.roi.height);
            cv::Point2f tr(br.x, tl.y);
            cv::Point2f bl(tl.x, br.y);
            cv::line(vis, tl, br, cv::Scalar(0, 255, 0), 1);
            cv::line(vis, tr, bl, cv::Scalar(0, 255, 0), 1);
        }
    }

    if (!armors.empty() && armors.front().corners.size() == 4) {
        const auto& corners = armors.front().corners;
        const int x = 10;
        int y = 20;
        for (size_t i = 0; i < corners.size(); ++i) {
            const std::string text =
                "P" + std::to_string(i) + ": (" +
                std::to_string(static_cast<int>(corners[i].x)) + "," +
                std::to_string(static_cast<int>(corners[i].y)) + ")";
            cv::putText(vis, text, cv::Point(x, y),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
            y += 18;
        }
    }
    drawWorldZAxis(vis);
    return vis;
}

void ArmorDetectionPipeline::drawAxes(cv::Mat& vis,
                                      const cv::Mat& rvec,
                                      const cv::Mat& tvec) const {
    const float axis_length = 60.0f;
    std::vector<cv::Point3f> axis_points = {
        {0.0f, 0.0f, 0.0f},
        {axis_length, 0.0f, 0.0f},
        {0.0f, axis_length, 0.0f},
        {0.0f, 0.0f, axis_length}
    };

    std::vector<cv::Point2f> projected;
    cv::projectPoints(axis_points, rvec, tvec, camera_matrix_, dist_coeffs_, projected);
    if (projected.size() != 4) {
        return;
    }

    const cv::Point2f& origin = projected[0];
    cv::line(vis, origin, projected[1], cv::Scalar(0, 0, 255), 2);  // X - Red
    cv::line(vis, origin, projected[2], cv::Scalar(0, 255, 0), 2);  // Y - Green
    cv::line(vis, origin, projected[3], cv::Scalar(255, 0, 0), 2);  // Z - Blue

    std::vector<cv::Point3f> world_z_points = {
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 100.0f}
    };
    std::vector<cv::Point2f> world_projected;
    cv::projectPoints(world_z_points, rvec, tvec, camera_matrix_, dist_coeffs_, world_projected);
    if (world_projected.size() == 2) {
        cv::line(vis, world_projected[0], world_projected[1],
                 cv::Scalar(255, 128, 0), 2);
    }
}

void ArmorDetectionPipeline::drawWorldZAxis(cv::Mat& vis) const {
    std::vector<cv::Point3f> world_z_points = {
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 50.0f}
    };
    std::vector<cv::Point2f> projected;
    cv::projectPoints(world_z_points, rvec_cw_, tvec_cw_,
                      camera_matrix_, dist_coeffs_, projected);
    if (projected.size() == 2) {
        cv::line(vis, projected[0], projected[1], cv::Scalar(0, 128, 255), 2);
    }
}

cv::Rect ArmorDetectionPipeline::clampRect(const cv::Rect& rect, const cv::Size& size) const {
    int x = std::max(0, rect.x);
    int y = std::max(0, rect.y);
    int w = std::min(rect.width, size.width - x);
    int h = std::min(rect.height, size.height - y);
    if (w <= 0 || h <= 0) {
        return cv::Rect();
    }
    return cv::Rect(x, y, w, h);
}

std::string ArmorDetectionPipeline::armorNumberToString(ArmorNumber number) const {
    switch (number) {
        case NUMBER_1: return "1";
        case NUMBER_2: return "2";
        case NUMBER_3: return "3";
        case NUMBER_4: return "4";
        case NUMBER_5: return "5";
        case NUMBER_6: return "6";
        case NUMBER_7: return "7";
        case NUMBER_8: return "8";
        case NUMBER_9: return "9";
        default: return "Unknown";
    }
}
