#include "preprocessor/ColorProcessor.hpp"
#include <iostream>

// 默认参数构造函数
ColorProcessorParams::ColorProcessorParams() {
    // LAB颜色空间阈值 (经验值)
    red_lab = {cv::Scalar(0, 150, 0), cv::Scalar(255, 255, 255)};    // 红色在LAB空间
    blue_lab = {cv::Scalar(100, 0, 0), cv::Scalar(255, 255, 255)};   // 蓝色在LAB空间
    
    // HSV颜色空间阈值
    // 红色在HSV中有两个范围（0-10和170-180）
    red_hsv = {cv::Scalar(0, 150, 50), cv::Scalar(10, 255, 255)};    // 红色范围1
    blue_hsv = {cv::Scalar(100, 150, 50), cv::Scalar(130, 255, 255)}; // 蓝色范围
    
    // 图像增强参数
    clahe_clip_limit = 2.0;
    clahe_tile_size = 8;
    gamma_correction = 1.2f;
    
    // 形态学操作参数
    morph_open_size = 3;    // 开运算核大小，去除小噪声
    morph_close_size = 5;   // 闭运算核大小，连接相邻区域
}

// ColorProcessor类实现
ColorProcessor::ColorProcessor(const ColorProcessorParams& params) 
    : params_(params) {
    std::cout << "ColorProcessor initialized with parameters." << std::endl;
}

cv::Mat ColorProcessor::process(const cv::Mat& frame, bool is_red_target) {
    ColorProcessResult result = processDetailed(frame, is_red_target);
    return result.target_binary;
}

ColorProcessor::ColorProcessResult ColorProcessor::processDetailed(const cv::Mat& frame,
                                                                   bool is_red_target) {
    ColorProcessResult result;
    if (frame.empty()) {
        std::cerr << "Error: Empty frame in ColorProcessor!" << std::endl;
        return result;
    }

    result.enhanced = enhanceImage(frame);
    separateColors(result.enhanced, result.red_binary, result.blue_binary);
    result.target_binary = is_red_target ? result.red_binary : result.blue_binary;

    createDebugImage(frame, result.enhanced, result.red_binary, result.blue_binary, result.target_binary);
    result.debug_image = debug_image_;
    return result;
}

void ColorProcessor::separateColors(const cv::Mat& frame, cv::Mat& red_binary, cv::Mat& blue_binary) {
    // 分别使用LAB和HSV空间分离颜色，然后合并结果
    cv::Mat red_lab = separateColorLAB(frame, 0);  // 0表示红色
    cv::Mat red_hsv = separateColorHSV(frame, 0);
    red_binary = mergeColorChannels(red_lab, red_hsv);
    red_binary = applyMorphology(red_binary, true);  // true表示红色
    
    cv::Mat blue_lab = separateColorLAB(frame, 1);  // 1表示蓝色
    cv::Mat blue_hsv = separateColorHSV(frame, 1);
    blue_binary = mergeColorChannels(blue_lab, blue_hsv);
    blue_binary = applyMorphology(blue_binary, false); // false表示蓝色
}

cv::Mat ColorProcessor::enhanceImage(const cv::Mat& frame) {
    cv::Mat enhanced = frame.clone();
    
    // 1. 应用CLAHE（对比度受限的自适应直方图均衡化）
    if (params_.clahe_clip_limit > 0) {
        std::vector<cv::Mat> channels;
        cv::split(enhanced, channels);
        
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(
            params_.clahe_clip_limit, 
            cv::Size(params_.clahe_tile_size, params_.clahe_tile_size)
        );
        
        for (int i = 0; i < channels.size(); i++) {
            clahe->apply(channels[i], channels[i]);
        }
        
        cv::merge(channels, enhanced);
    }
    
    // 2. Gamma校正
    if (params_.gamma_correction != 1.0f) {
        cv::Mat lookup_table(1, 256, CV_8U);
        uchar* p = lookup_table.ptr();
        float gamma_inv = 1.0f / params_.gamma_correction;
        for (int i = 0; i < 256; ++i) {
            p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, gamma_inv) * 255.0);
        }
        cv::LUT(enhanced, lookup_table, enhanced);
    }
    
    return enhanced;
}

cv::Mat ColorProcessor::separateColorLAB(const cv::Mat& frame, int color_type) {
    cv::Mat lab_image;
    cv::cvtColor(frame, lab_image, cv::COLOR_BGR2Lab);
    
    cv::Mat binary;
    if (color_type == 0) {  // 红色
        // 红色在LAB空间的a通道值较高
        cv::Mat channels[3];
        cv::split(lab_image, channels);
        
        // 使用a通道分离红色
        cv::threshold(channels[1], binary, params_.red_lab.lower[1], 
                     params_.red_lab.upper[1], cv::THRESH_BINARY);
    } else {  // 蓝色
        // 蓝色在LAB空间的b通道值较高
        cv::Mat channels[3];
        cv::split(lab_image, channels);
        
        // 使用b通道分离蓝色
        cv::threshold(channels[2], binary, params_.blue_lab.lower[2], 
                     params_.blue_lab.upper[2], cv::THRESH_BINARY);
    }
    
    return binary;
}

cv::Mat ColorProcessor::separateColorHSV(const cv::Mat& frame, int color_type) {
    cv::Mat hsv_image;
    cv::cvtColor(frame, hsv_image, cv::COLOR_BGR2HSV);
    
    if (color_type == 0) {  // 红色
        return processRedSpecial(hsv_image);
    } else {  // 蓝色
        cv::Mat binary;
        cv::inRange(hsv_image, params_.blue_hsv.lower, 
                   params_.blue_hsv.upper, binary);
        return binary;
    }
}

cv::Mat ColorProcessor::processRedSpecial(const cv::Mat& hsv_image) {
    // 红色在HSV空间有两个范围：0-10和170-180
    cv::Mat red_binary1, red_binary2;
    
    // 范围1: 0-10
    cv::Scalar lower1(params_.red_hsv.lower[0], params_.red_hsv.lower[1], params_.red_hsv.lower[2]);
    cv::Scalar upper1(10, params_.red_hsv.upper[1], params_.red_hsv.upper[2]);
    cv::inRange(hsv_image, lower1, upper1, red_binary1);
    
    // 范围2: 170-180
    cv::Scalar lower2(170, params_.red_hsv.lower[1], params_.red_hsv.lower[2]);
    cv::Scalar upper2(180, params_.red_hsv.upper[1], params_.red_hsv.upper[2]);
    cv::inRange(hsv_image, lower2, upper2, red_binary2);
    
    // 合并两个范围
    cv::Mat red_binary;
    cv::bitwise_or(red_binary1, red_binary2, red_binary);
    
    return red_binary;
}

cv::Mat ColorProcessor::mergeColorChannels(const cv::Mat& lab_binary, const cv::Mat& hsv_binary) {
    cv::Mat merged;
    cv::bitwise_and(lab_binary, hsv_binary, merged);
    return merged;
}

cv::Mat ColorProcessor::applyMorphology(const cv::Mat& binary, bool is_red) {
    if (binary.empty()) return binary;
    
    cv::Mat result = binary.clone();
    int kernel_size = is_red ? params_.morph_open_size : params_.morph_open_size + 1;
    
    // 1. 开运算去除小噪声
    cv::Mat kernel_open = cv::getStructuringElement(
        cv::MORPH_RECT, 
        cv::Size(kernel_size, kernel_size)
    );
    cv::morphologyEx(result, result, cv::MORPH_OPEN, kernel_open);
    
    // 2. 闭运算连接相邻区域
    kernel_size = is_red ? params_.morph_close_size : params_.morph_close_size + 1;
    cv::Mat kernel_close = cv::getStructuringElement(
        cv::MORPH_RECT, 
        cv::Size(kernel_size, kernel_size)
    );
    cv::morphologyEx(result, result, cv::MORPH_CLOSE, kernel_close);
    
    return result;
}

void ColorProcessor::createDebugImage(const cv::Mat& original, const cv::Mat& enhanced,
                                     const cv::Mat& red_binary, const cv::Mat& blue_binary,
                                     const cv::Mat& target_binary) {
    // 创建3x2的调试图像网格
    const int cell_width = original.cols / 3;
    const int cell_height = original.rows / 2;
    
    debug_image_ = cv::Mat::zeros(cell_height * 2, cell_width * 3, CV_8UC3);
    
    // 1. 原始图像
    cv::Mat roi = debug_image_(cv::Rect(0, 0, cell_width, cell_height));
    cv::resize(original, roi, cv::Size(cell_width, cell_height));
    cv::putText(roi, "Original", cv::Point(10, 30), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
    
    // 2. 增强图像
    roi = debug_image_(cv::Rect(cell_width, 0, cell_width, cell_height));
    cv::resize(enhanced, roi, cv::Size(cell_width, cell_height));
    cv::putText(roi, "Enhanced", cv::Point(10, 30), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
    
    // 3. 红色二值图
    roi = debug_image_(cv::Rect(cell_width * 2, 0, cell_width, cell_height));
    cv::Mat red_display;
    cv::cvtColor(red_binary, red_display, cv::COLOR_GRAY2BGR);
    cv::resize(red_display, roi, cv::Size(cell_width, cell_height));
    cv::putText(roi, "Red Binary", cv::Point(10, 30), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
    
    // 4. 蓝色二值图
    roi = debug_image_(cv::Rect(0, cell_height, cell_width, cell_height));
    cv::Mat blue_display;
    cv::cvtColor(blue_binary, blue_display, cv::COLOR_GRAY2BGR);
    cv::resize(blue_display, roi, cv::Size(cell_width, cell_height));
    cv::putText(roi, "Blue Binary", cv::Point(10, 30), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 0, 0), 2);
    
    // 5. 目标二值图
    roi = debug_image_(cv::Rect(cell_width, cell_height, cell_width, cell_height));
    cv::Mat target_display;
    cv::cvtColor(target_binary, target_display, cv::COLOR_GRAY2BGR);
    cv::resize(target_display, roi, cv::Size(cell_width, cell_height));
    cv::putText(roi, "Target Binary", cv::Point(10, 30), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);
    
    // 6. 原始图像上叠加目标
    roi = debug_image_(cv::Rect(cell_width * 2, cell_height, cell_width, cell_height));
    cv::Mat overlay = original.clone();
    if (!target_binary.empty()) {
        // 创建彩色掩码
        cv::Mat color_mask;
        cv::cvtColor(target_binary, color_mask, cv::COLOR_GRAY2BGR);
        color_mask.setTo(cv::Scalar(0, 255, 255), target_binary); // 黄色显示
        
        // 叠加到原图
        cv::addWeighted(overlay, 0.7, color_mask, 0.3, 0, overlay);
    }
    cv::resize(overlay, roi, cv::Size(cell_width, cell_height));
    cv::putText(roi, "Overlay", cv::Point(10, 30), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
}

void ColorProcessor::updateParams(const ColorProcessorParams& params) {
    params_ = params;
    std::cout << "ColorProcessor parameters updated." << std::endl;
}
