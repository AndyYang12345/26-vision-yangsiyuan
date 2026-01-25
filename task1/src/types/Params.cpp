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
    }

    return params;
}
