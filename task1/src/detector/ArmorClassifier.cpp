#include "detector/ArmorClassifier.hpp"
#include <algorithm>

ArmorClassifier::ArmorClassifier(const std::string& model_path) {
    if (!model_path.empty()) {
        net_ = cv::dnn::readNet(model_path);
    }
    input_size_ = cv::Size(28, 28);
}

ArmorNumber ArmorClassifier::classify(const cv::Mat& armor_roi) {
    if (armor_roi.empty() || net_.empty()) {
        return UNKNOWN;
    }

    cv::Mat input = preprocessROI(armor_roi);
    cv::Mat blob = cv::dnn::blobFromImage(input, 1.0 / 255.0, input_size_);
    net_.setInput(blob);
    cv::Mat output = net_.forward();

    cv::Point class_id;
    double max_val = 0.0;
    cv::minMaxLoc(output.reshape(1, 1), nullptr, &max_val, nullptr, &class_id);
    int idx = class_id.x;

    switch (idx) {
        case 1: return NUMBER_1;
        case 2: return NUMBER_2;
        case 3: return NUMBER_3;
        case 4: return NUMBER_4;
        case 5: return NUMBER_5;
        case 6: return NUMBER_6;
        case 7: return NUMBER_7;
        case 8: return NUMBER_8;
        case 9: return NUMBER_9;
        default: return UNKNOWN;
    }
}

void ArmorClassifier::classifyAll(std::vector<Armor>& armors) {
    for (auto& armor : armors) {
        armor.number = UNKNOWN;
    }
}

cv::Mat ArmorClassifier::preprocessROI(const cv::Mat& roi) {
    cv::Mat gray;
    if (roi.channels() == 3) {
        cv::cvtColor(roi, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = roi.clone();
    }
    cv::Mat resized;
    cv::resize(gray, resized, input_size_);
    return resized;
}
