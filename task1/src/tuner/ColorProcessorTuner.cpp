#include "tuner/ColorProcessorTuner.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>

ColorProcessorTuner::ColorProcessorTuner(const std::string& window_name,
                                       const ColorProcessorParams& initial_params)
    : window_name_(window_name)
    , current_params_(initial_params)
    , default_params_(initial_params)
    , params_changed_(false)
    , on_params_changed_(nullptr) {
    updateTrackbarsFromParams();
}

ColorProcessorTuner::~ColorProcessorTuner() {
    cv::destroyWindow(window_name_);
}

void ColorProcessorTuner::createTuningWindow() {
    cv::namedWindow(window_name_, cv::WINDOW_NORMAL);
    cv::resizeWindow(window_name_, 800, 600);
    
    createTrackbars();
    
    std::cout << "Color Processor Tuner created." << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  's' - Save parameters" << std::endl;
    std::cout << "  'l' - Load parameters" << std::endl;
    std::cout << "  'r' - Reset to default" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
}

void ColorProcessorTuner::createTrackbars() {
    // LAB空间阈值滑动条
    cv::createTrackbar("LAB Red A Low", window_name_, 
                      &trackbar_values_.lab_red_a_low, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("LAB Red A High", window_name_, 
                      &trackbar_values_.lab_red_a_high, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("LAB Blue B Low", window_name_, 
                      &trackbar_values_.lab_blue_b_low, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("LAB Blue B High", window_name_, 
                      &trackbar_values_.lab_blue_b_high, 255, 
                      onTrackbarChanged, this);
    
    // HSV红色阈值滑动条（范围1）
    cv::createTrackbar("HSV Red H1 Low", window_name_, 
                      &trackbar_values_.hsv_red_h1_low, 180, 
                      onTrackbarChanged, this);
    cv::createTrackbar("HSV Red H1 High", window_name_, 
                      &trackbar_values_.hsv_red_h1_high, 180, 
                      onTrackbarChanged, this);
    cv::createTrackbar("HSV Red H2 Low", window_name_, 
                      &trackbar_values_.hsv_red_h2_low, 180, 
                      onTrackbarChanged, this);
    cv::createTrackbar("HSV Red H2 High", window_name_, 
                      &trackbar_values_.hsv_red_h2_high, 180, 
                      onTrackbarChanged, this);
    cv::createTrackbar("HSV Red S Low", window_name_, 
                      &trackbar_values_.hsv_red_s_low, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("HSV Red S High", window_name_, 
                      &trackbar_values_.hsv_red_s_high, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("HSV Red V Low", window_name_, 
                      &trackbar_values_.hsv_red_v_low, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("HSV Red V High", window_name_, 
                      &trackbar_values_.hsv_red_v_high, 255, 
                      onTrackbarChanged, this);
    
    // HSV蓝色阈值滑动条
    cv::createTrackbar("HSV Blue H Low", window_name_, 
                      &trackbar_values_.hsv_blue_h_low, 180, 
                      onTrackbarChanged, this);
    cv::createTrackbar("HSV Blue H High", window_name_, 
                      &trackbar_values_.hsv_blue_h_high, 180, 
                      onTrackbarChanged, this);
    cv::createTrackbar("HSV Blue S Low", window_name_, 
                      &trackbar_values_.hsv_blue_s_low, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("HSV Blue S High", window_name_, 
                      &trackbar_values_.hsv_blue_s_high, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("HSV Blue V Low", window_name_, 
                      &trackbar_values_.hsv_blue_v_low, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("HSV Blue V High", window_name_, 
                      &trackbar_values_.hsv_blue_v_high, 255, 
                      onTrackbarChanged, this);
    
    // 图像增强参数
    cv::createTrackbar("CLAHE Clip", window_name_, 
                      &trackbar_values_.clahe_clip_limit, 100, 
                      onTrackbarChanged, this);
    cv::createTrackbar("CLAHE Tile", window_name_, 
                      &trackbar_values_.clahe_tile_size, 32, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Gamma (x0.1)", window_name_, 
                      &trackbar_values_.gamma_correction, 50, 
                      onTrackbarChanged, this);
    
    // 形态学参数
    cv::createTrackbar("Morph Open", window_name_, 
                      &trackbar_values_.morph_open_size, 20, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Morph Close", window_name_, 
                      &trackbar_values_.morph_close_size, 20, 
                      onTrackbarChanged, this);
    
    // 显示控制
    cv::createTrackbar("Show Red", window_name_, 
                      &trackbar_values_.show_red, 1, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Show Blue", window_name_, 
                      &trackbar_values_.show_blue, 1, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Show Debug", window_name_, 
                      &trackbar_values_.show_debug, 1, 
                      onTrackbarChanged, this);
}

void ColorProcessorTuner::onTrackbarChanged(int, void* userdata) {
    ColorProcessorTuner* tuner = static_cast<ColorProcessorTuner*>(userdata);
    if (tuner) {
        tuner->updateParamsFromTrackbars();
        tuner->params_changed_ = true;
        
        // 调用回调函数
        if (tuner->on_params_changed_) {
            tuner->on_params_changed_(tuner->current_params_);
        }
    }
}

void ColorProcessorTuner::updateParamsFromTrackbars() {
    // 更新LAB空间阈值
    current_params_.red_lab.lower = cv::Scalar(0, trackbar_values_.lab_red_a_low, 0);
    current_params_.red_lab.upper = cv::Scalar(255, trackbar_values_.lab_red_a_high, 255);
    current_params_.blue_lab.lower = cv::Scalar(trackbar_values_.lab_blue_b_low, 0, 0);
    current_params_.blue_lab.upper = cv::Scalar(255, 255, 255);
    
    // 更新HSV空间阈值
    current_params_.red_hsv.lower = cv::Scalar(trackbar_values_.hsv_red_h1_low, 
                                              trackbar_values_.hsv_red_s_low,
                                              trackbar_values_.hsv_red_v_low);
    current_params_.red_hsv.upper = cv::Scalar(trackbar_values_.hsv_red_h1_high,
                                              trackbar_values_.hsv_red_s_high,
                                              trackbar_values_.hsv_red_v_high);
    // 注意：红色有两个范围，这里只存储第一个范围
    
    current_params_.blue_hsv.lower = cv::Scalar(trackbar_values_.hsv_blue_h_low,
                                               trackbar_values_.hsv_blue_s_low,
                                               trackbar_values_.hsv_blue_v_low);
    current_params_.blue_hsv.upper = cv::Scalar(trackbar_values_.hsv_blue_h_high,
                                               trackbar_values_.hsv_blue_s_high,
                                               trackbar_values_.hsv_blue_v_high);
    
    // 更新图像增强参数
    current_params_.clahe_clip_limit = trackbar_values_.clahe_clip_limit / 10.0f;
    current_params_.clahe_tile_size = trackbar_values_.clahe_tile_size;
    current_params_.gamma_correction = trackbar_values_.gamma_correction / 10.0f;
    
    // 更新形态学参数
    current_params_.morph_open_size = trackbar_values_.morph_open_size;
    current_params_.morph_close_size = trackbar_values_.morph_close_size;
    
    // 确保下限不大于上限
    if (trackbar_values_.lab_red_a_low > trackbar_values_.lab_red_a_high) {
        trackbar_values_.lab_red_a_low = trackbar_values_.lab_red_a_high;
        cv::setTrackbarPos("LAB Red A Low", window_name_, trackbar_values_.lab_red_a_low);
    }
    // 其他阈值的检查类似...
}

void ColorProcessorTuner::updateTrackbarsFromParams() {
    // 从参数更新滑动条位置
    trackbar_values_.lab_red_a_low = current_params_.red_lab.lower[1];
    trackbar_values_.lab_red_a_high = current_params_.red_lab.upper[1];
    trackbar_values_.lab_blue_b_low = current_params_.blue_lab.lower[0];
    trackbar_values_.lab_blue_b_high = current_params_.blue_lab.upper[0];
    
    trackbar_values_.hsv_red_h1_low = current_params_.red_hsv.lower[0];
    trackbar_values_.hsv_red_h1_high = current_params_.red_hsv.upper[0];
    trackbar_values_.hsv_red_s_low = current_params_.red_hsv.lower[1];
    trackbar_values_.hsv_red_s_high = current_params_.red_hsv.upper[1];
    trackbar_values_.hsv_red_v_low = current_params_.red_hsv.lower[2];
    trackbar_values_.hsv_red_v_high = current_params_.red_hsv.upper[2];
    
    trackbar_values_.hsv_blue_h_low = current_params_.blue_hsv.lower[0];
    trackbar_values_.hsv_blue_h_high = current_params_.blue_hsv.upper[0];
    trackbar_values_.hsv_blue_s_low = current_params_.blue_hsv.lower[1];
    trackbar_values_.hsv_blue_s_high = current_params_.blue_hsv.upper[1];
    trackbar_values_.hsv_blue_v_low = current_params_.blue_hsv.lower[2];
    trackbar_values_.hsv_blue_v_high = current_params_.blue_hsv.upper[2];
    
    trackbar_values_.clahe_clip_limit = static_cast<int>(current_params_.clahe_clip_limit * 10);
    trackbar_values_.clahe_tile_size = current_params_.clahe_tile_size;
    trackbar_values_.gamma_correction = static_cast<int>(current_params_.gamma_correction * 10);
    
    trackbar_values_.morph_open_size = current_params_.morph_open_size;
    trackbar_values_.morph_close_size = current_params_.morph_close_size;
}

void ColorProcessorTuner::updateDisplay(const cv::Mat& original_frame,
                                       const cv::Mat& processed_frame,
                                       const cv::Mat& binary_result) {
    if (original_frame.empty()) return;
    
    // 创建显示图像
    cv::Mat display;
    if (trackbar_values_.show_debug && !processed_frame.empty()) {
        // 显示调试图像
        display = processed_frame.clone();
    } else {
        // 显示原始图像和结果叠加
        std::vector<cv::Mat> images;
        
        // 原始图像
        cv::Mat original_display;
        cv::resize(original_frame, original_display, cv::Size(400, 300));
        cv::putText(original_display, "Original", cv::Point(10, 30),
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
        images.push_back(original_display);
        
        // 二值化结果
        if (!binary_result.empty()) {
            cv::Mat binary_display;
            cv::cvtColor(binary_result, binary_display, cv::COLOR_GRAY2BGR);
            cv::resize(binary_display, binary_display, cv::Size(400, 300));
            cv::putText(binary_display, "Binary Result", cv::Point(10, 30),
                       cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
            images.push_back(binary_display);
        }
        
        // 参数显示
        cv::Mat params_display = createParamsDisplayImage();
        images.push_back(params_display);
        
        // 水平拼接
        if (images.size() == 3) {
            cv::hconcat(images, display);
        } else {
            display = images[0];
        }
    }
    
    // 显示图像
    cv::imshow(window_name_, display);
}

cv::Mat ColorProcessorTuner::createParamsDisplayImage() const {
    cv::Mat params_display = cv::Mat::zeros(300, 400, CV_8UC3);
    params_display.setTo(cv::Scalar(50, 50, 50));  // 深灰色背景
    
    int y_pos = 30;
    int line_height = 25;
    
    // 添加参数信息
    auto addText = [&](const std::string& text, const cv::Scalar& color = cv::Scalar(255, 255, 255)) {
        cv::putText(params_display, text, cv::Point(10, y_pos),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, color, 1);
        y_pos += line_height;
    };
    
    addText("=== Color Processor Parameters ===", cv::Scalar(0, 255, 255));
    addText("");
    
    addText("LAB Space:");
    addText("  Red A: " + std::to_string(current_params_.red_lab.lower[1]) + 
           " - " + std::to_string(current_params_.red_lab.upper[1]));
    addText("  Blue B: " + std::to_string(current_params_.blue_lab.lower[0]) + 
           " - " + std::to_string(current_params_.blue_lab.upper[0]));
    addText("");
    
    addText("HSV Space (Red):");
    addText("  H: " + std::to_string(current_params_.red_hsv.lower[0]) + 
           " - " + std::to_string(current_params_.red_hsv.upper[0]));
    addText("  S: " + std::to_string(current_params_.red_hsv.lower[1]) + 
           " - " + std::to_string(current_params_.red_hsv.upper[1]));
    addText("  V: " + std::to_string(current_params_.red_hsv.lower[2]) + 
           " - " + std::to_string(current_params_.red_hsv.upper[2]));
    addText("");
    
    addText("HSV Space (Blue):");
    addText("  H: " + std::to_string(current_params_.blue_hsv.lower[0]) + 
           " - " + std::to_string(current_params_.blue_hsv.upper[0]));
    addText("  S: " + std::to_string(current_params_.blue_hsv.lower[1]) + 
           " - " + std::to_string(current_params_.blue_hsv.upper[1]));
    addText("  V: " + std::to_string(current_params_.blue_hsv.lower[2]) + 
           " - " + std::to_string(current_params_.blue_hsv.upper[2]));
    addText("");
    
    addText("Image Enhancement:");
    addText("  CLAHE Clip: " + std::to_string(current_params_.clahe_clip_limit));
    addText("  CLAHE Tile: " + std::to_string(current_params_.clahe_tile_size));
    addText("  Gamma: " + std::to_string(current_params_.gamma_correction));
    addText("");
    
    addText("Morphology:");
    addText("  Open Size: " + std::to_string(current_params_.morph_open_size));
    addText("  Close Size: " + std::to_string(current_params_.morph_close_size));
    addText("");
    
    addText("=== Controls ===", cv::Scalar(0, 255, 0));
    addText("  's' - Save parameters");
    addText("  'l' - Load parameters");
    addText("  'r' - Reset to default");
    addText("  ESC - Exit");
    
    return params_display;
}

void ColorProcessorTuner::saveParams(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件保存参数: " << filename << std::endl;
        return;
    }
    
    file << "# Color Processor Parameters" << std::endl;
    file << "# Generated by ColorProcessorTuner" << std::endl;
    file << std::endl;
    
    // LAB空间参数
    file << "[LAB]" << std::endl;
    file << "red_a_low = " << current_params_.red_lab.lower[1] << std::endl;
    file << "red_a_high = " << current_params_.red_lab.upper[1] << std::endl;
    file << "blue_b_low = " << current_params_.blue_lab.lower[0] << std::endl;
    file << "blue_b_high = " << current_params_.blue_lab.upper[0] << std::endl;
    file << std::endl;
    
    // HSV空间参数
    file << "[HSV_RED]" << std::endl;
    file << "h_low = " << current_params_.red_hsv.lower[0] << std::endl;
    file << "h_high = " << current_params_.red_hsv.upper[0] << std::endl;
    file << "s_low = " << current_params_.red_hsv.lower[1] << std::endl;
    file << "s_high = " << current_params_.red_hsv.upper[1] << std::endl;
    file << "v_low = " << current_params_.red_hsv.lower[2] << std::endl;
    file << "v_high = " << current_params_.red_hsv.upper[2] << std::endl;
    file << std::endl;
    
    file << "[HSV_BLUE]" << std::endl;
    file << "h_low = " << current_params_.blue_hsv.lower[0] << std::endl;
    file << "h_high = " << current_params_.blue_hsv.upper[0] << std::endl;
    file << "s_low = " << current_params_.blue_hsv.lower[1] << std::endl;
    file << "s_high = " << current_params_.blue_hsv.upper[1] << std::endl;
    file << "v_low = " << current_params_.blue_hsv.lower[2] << std::endl;
    file << "v_high = " << current_params_.blue_hsv.upper[2] << std::endl;
    file << std::endl;
    
    // 图像增强参数
    file << "[ENHANCEMENT]" << std::endl;
    file << "clahe_clip = " << current_params_.clahe_clip_limit << std::endl;
    file << "clahe_tile = " << current_params_.clahe_tile_size << std::endl;
    file << "gamma = " << current_params_.gamma_correction << std::endl;
    file << std::endl;
    
    // 形态学参数
    file << "[MORPHOLOGY]" << std::endl;
    file << "open_size = " << current_params_.morph_open_size << std::endl;
    file << "close_size = " << current_params_.morph_close_size << std::endl;
    
    file.close();
    std::cout << "参数已保存到: " << filename << std::endl;
}

void ColorProcessorTuner::loadParams(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件加载参数: " << filename << std::endl;
        return;
    }
    
    std::string line;
    std::string current_section;
    
    while (std::getline(file, line)) {
        // 跳过注释和空行
        if (line.empty() || line[0] == '#') continue;
        
        // 检查是否是章节
        if (line[0] == '[' && line.back() == ']') {
            current_section = line.substr(1, line.size() - 2);
            continue;
        }
        
        // 解析键值对
        size_t equals_pos = line.find('=');
        if (equals_pos == std::string::npos) continue;
        
        std::string key = line.substr(0, equals_pos);
        key.erase(key.find_last_not_of(" \t") + 1);
        key.erase(0, key.find_first_not_of(" \t"));
        
        std::string value_str = line.substr(equals_pos + 1);
        value_str.erase(value_str.find_last_not_of(" \t") + 1);
        value_str.erase(0, value_str.find_first_not_of(" \t"));
        
        int value = std::stoi(value_str);
        
        // 根据章节和键更新参数
        if (current_section == "LAB") {
            if (key == "red_a_low") current_params_.red_lab.lower[1] = value;
            else if (key == "red_a_high") current_params_.red_lab.upper[1] = value;
            else if (key == "blue_b_low") current_params_.blue_lab.lower[0] = value;
            else if (key == "blue_b_high") current_params_.blue_lab.upper[0] = value;
        }
        else if (current_section == "HSV_RED") {
            if (key == "h_low") current_params_.red_hsv.lower[0] = value;
            else if (key == "h_high") current_params_.red_hsv.upper[0] = value;
            else if (key == "s_low") current_params_.red_hsv.lower[1] = value;
            else if (key == "s_high") current_params_.red_hsv.upper[1] = value;
            else if (key == "v_low") current_params_.red_hsv.lower[2] = value;
            else if (key == "v_high") current_params_.red_hsv.upper[2] = value;
        }
        else if (current_section == "HSV_BLUE") {
            if (key == "h_low") current_params_.blue_hsv.lower[0] = value;
            else if (key == "h_high") current_params_.blue_hsv.upper[0] = value;
            else if (key == "s_low") current_params_.blue_hsv.lower[1] = value;
            else if (key == "s_high") current_params_.blue_hsv.upper[1] = value;
            else if (key == "v_low") current_params_.blue_hsv.lower[2] = value;
            else if (key == "v_high") current_params_.blue_hsv.upper[2] = value;
        }
        else if (current_section == "ENHANCEMENT") {
            if (key == "clahe_clip") current_params_.clahe_clip_limit = value;
            else if (key == "clahe_tile") current_params_.clahe_tile_size = value;
            else if (key == "gamma") current_params_.gamma_correction = value;
        }
        else if (current_section == "MORPHOLOGY") {
            if (key == "open_size") current_params_.morph_open_size = value;
            else if (key == "close_size") current_params_.morph_close_size = value;
        }
    }
    
    file.close();
    updateTrackbarsFromParams();
    params_changed_ = true;
    
    std::cout << "参数已从文件加载: " << filename << std::endl;
}

void ColorProcessorTuner::setOnParamsChangedCallback(std::function<void(const ColorProcessorParams&)> callback) {
    on_params_changed_ = callback;
}

void ColorProcessorTuner::run(std::function<bool(cv::Mat&)> frame_source) {
    cv::Mat frame;
    ColorProcessor processor(current_params_);
    
    while (true) {
        // 获取帧
        if (!frame_source(frame)) {
            break;
        }
        
        if (frame.empty()) {
            std::cerr << "空帧！" << std::endl;
            break;
        }
        
        // 检查是否需要更新处理器参数
        if (needsUpdate()) {
            processor.updateParams(current_params_);
            markUpdated();
        }
        
        // 处理图像
        cv::Mat binary_result = processor.process(frame, true);
        cv::Mat debug_image = processor.getDebugImage();
        
        // 更新显示
        updateDisplay(frame, debug_image, binary_result);
        
        // 处理键盘输入
        int key = cv::waitKey(30);
        if (key == 27) { // ESC
            break;
        } else if (key == 's' || key == 'S') {
            saveParams("color_params.cfg");
        } else if (key == 'l' || key == 'L') {
            loadParams("color_params.cfg");
        } else if (key == 'r' || key == 'R') {
            // 重置为默认参数
            current_params_ = default_params_;
            updateTrackbarsFromParams();
            params_changed_ = true;
            std::cout << "参数已重置为默认值" << std::endl;
        }
    }
}