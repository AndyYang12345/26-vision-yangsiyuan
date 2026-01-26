#include "types/Params.hpp"

namespace {
template <typename T>
void readIfPresent(const cv::FileNode& node, const char* key, T& out) {
    const cv::FileNode child = node[key];
    if (!child.empty()) {
        child >> out;
    }
}
}  // namespace

Params Params::loadFromYAML(const std::string& filename) {
    Params params{};

    cv::FileStorage fs(filename, cv::FileStorage::READ);
    if (!fs.isOpened()) {
        return params;
    }

    const cv::FileNode preprocess = fs["preprocess"];
    if (!preprocess.empty()) {
        readIfPresent(preprocess, "gaussian_kernel", params.preprocess.gaussian_kernel);
        readIfPresent(preprocess, "binary_threshold", params.preprocess.binary_threshold);
        readIfPresent(preprocess, "use_adaptive_thresh", params.preprocess.use_adaptive_thresh);
    }

    const cv::FileNode lightbar = fs["lightbar"];
    if (!lightbar.empty()) {
        readIfPresent(lightbar, "min_area", params.lightbar.min_area);
        readIfPresent(lightbar, "max_area_ratio", params.lightbar.max_area_ratio);
        readIfPresent(lightbar, "min_angle_diff", params.lightbar.min_angle_diff);
    }

    const cv::FileNode armor = fs["armor"];
    if (!armor.empty()) {
        readIfPresent(armor, "min_aspect_ratio", params.armor.min_aspect_ratio);
        readIfPresent(armor, "max_aspect_ratio", params.armor.max_aspect_ratio);
        readIfPresent(armor, "min_lightbar_distance", params.armor.min_lightbar_distance);
        readIfPresent(armor, "max_lightbar_distance", params.armor.max_lightbar_distance);
        readIfPresent(armor, "max_angle_diff", params.armor.max_angle_diff);
        readIfPresent(armor, "max_height_diff_ratio", params.armor.max_height_diff_ratio);
        readIfPresent(armor, "max_length_ratio", params.armor.max_length_ratio);
        readIfPresent(armor, "min_lightbar_area", params.armor.min_lightbar_area);
    }

    const cv::FileNode binarize = fs["binarize"];
    if (!binarize.empty()) {
        readIfPresent(binarize, "method", params.binarize_method);
        readIfPresent(binarize, "br_diff_threshold", params.br_diff_threshold);
        readIfPresent(binarize, "morph_kernel", params.morph_kernel);
    }

    const cv::FileNode hsv = fs["hsv"];
    if (!hsv.empty()) {
        readIfPresent(hsv, "red_lower1", params.hsv_red_lower1);
        readIfPresent(hsv, "red_upper1", params.hsv_red_upper1);
        readIfPresent(hsv, "red_lower2", params.hsv_red_lower2);
        readIfPresent(hsv, "red_upper2", params.hsv_red_upper2);
        readIfPresent(hsv, "blue_lower", params.hsv_blue_lower);
        readIfPresent(hsv, "blue_upper", params.hsv_blue_upper);
    }

    const cv::FileNode classifier = fs["classifier"];
    if (!classifier.empty()) {
        readIfPresent(classifier, "model_path", params.classifier_model_path);
    }

    return params;
}
