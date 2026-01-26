#include "tuner/ColorProcessorTuner.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>


ColorProcessorTuner::ColorProcessorTuner(const std::string& window_name,
                                       const ColorProcessorParams& initial_params)
    : main_window_name_(window_name)
    , lab_window_name_(window_name + " - LAB")
    , hsv_red_window_name_(window_name + " - HSV Red")
    , hsv_blue_window_name_(window_name + " - HSV Blue")
    , enhance_window_name_(window_name + " - Enhancement")
    , morph_window_name_(window_name + " - Morphology")
    , current_params_(initial_params)
    , default_params_(initial_params)
    , params_changed_(false)
    , on_params_changed_(nullptr)
    , current_page_(0)
    , param_windows_visible_(false) {
    if (initial_params.red_hsv.lower[0] == 0) {  // 检查是否是默认值
        // 扩展红色范围以包含橙色/黄色
        current_params_.red_hsv.lower = cv::Scalar(0, 100, 100);    // H:0-20 包含橙色
        current_params_.red_hsv.upper = cv::Scalar(20, 255, 255);   // 调整上限
    }
    updateTrackbarsFromParams();
}

ColorProcessorTuner::~ColorProcessorTuner() {
    cv::destroyAllWindows();
}

void ColorProcessorTuner::createTuningWindow() {
    // 创建主窗口
    cv::namedWindow(main_window_name_, cv::WINDOW_NORMAL);
    cv::resizeWindow(main_window_name_, 1000, 700);
    
    // 设置鼠标回调
    cv::setMouseCallback(main_window_name_, onMouse, this);
    
    // 创建控制面板
    createControlPanel();
    
    std::cout << "Color Processor Tuner created." << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  点击主窗口右侧按钮切换参数页面" << std::endl;
    std::cout << "  's' - Save parameters" << std::endl;
    std::cout << "  'l' - Load parameters" << std::endl;
    std::cout << "  'r' - Reset to default" << std::endl;
    std::cout << "  'p' - Show/Hide parameter windows" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
}

void ColorProcessorTuner::createControlPanel() {
    // 控制面板在主窗口右侧区域显示，不需要单独窗口
}

void ColorProcessorTuner::createLABWindow() {
    cv::namedWindow(lab_window_name_, cv::WINDOW_NORMAL);
    cv::resizeWindow(lab_window_name_, 400, 250);
    cv::moveWindow(lab_window_name_, 1050, 50);
    
    cv::createTrackbar("Red A Low", lab_window_name_, 
                      &trackbar_values_.lab_red_a_low, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Red A High", lab_window_name_, 
                      &trackbar_values_.lab_red_a_high, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Blue B Low", lab_window_name_, 
                      &trackbar_values_.lab_blue_b_low, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Blue B High", lab_window_name_, 
                      &trackbar_values_.lab_blue_b_high, 255, 
                      onTrackbarChanged, this);
}

void ColorProcessorTuner::createHSVRedWindow() {
    cv::namedWindow(hsv_red_window_name_, cv::WINDOW_NORMAL);
    cv::resizeWindow(hsv_red_window_name_, 400, 300);
    cv::moveWindow(hsv_red_window_name_, 1050, 50);
    
    cv::createTrackbar("Red H1 Low", hsv_red_window_name_, 
                      &trackbar_values_.hsv_red_h1_low, 10, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Red H1 High", hsv_red_window_name_, 
                      &trackbar_values_.hsv_red_h1_high, 10, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Red H2 Low", hsv_red_window_name_, 
                      &trackbar_values_.hsv_red_h2_low, 180, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Red H2 High", hsv_red_window_name_, 
                      &trackbar_values_.hsv_red_h2_high, 180, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Red S Low", hsv_red_window_name_, 
                      &trackbar_values_.hsv_red_s_low, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Red S High", hsv_red_window_name_, 
                      &trackbar_values_.hsv_red_s_high, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Red V Low", hsv_red_window_name_, 
                      &trackbar_values_.hsv_red_v_low, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Red V High", hsv_red_window_name_, 
                      &trackbar_values_.hsv_red_v_high, 255, 
                      onTrackbarChanged, this);
}

void ColorProcessorTuner::createHSVBlueWindow() {
    cv::namedWindow(hsv_blue_window_name_, cv::WINDOW_NORMAL);
    cv::resizeWindow(hsv_blue_window_name_, 400, 250);
    cv::moveWindow(hsv_blue_window_name_, 1050, 50);
    
    cv::createTrackbar("Blue H Low", hsv_blue_window_name_, 
                      &trackbar_values_.hsv_blue_h_low, 180, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Blue H High", hsv_blue_window_name_, 
                      &trackbar_values_.hsv_blue_h_high, 180, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Blue S Low", hsv_blue_window_name_, 
                      &trackbar_values_.hsv_blue_s_low, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Blue S High", hsv_blue_window_name_, 
                      &trackbar_values_.hsv_blue_s_high, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Blue V Low", hsv_blue_window_name_, 
                      &trackbar_values_.hsv_blue_v_low, 255, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Blue V High", hsv_blue_window_name_, 
                      &trackbar_values_.hsv_blue_v_high, 255, 
                      onTrackbarChanged, this);
}

void ColorProcessorTuner::createEnhanceWindow() {
    cv::namedWindow(enhance_window_name_, cv::WINDOW_NORMAL);
    cv::resizeWindow(enhance_window_name_, 400, 200);
    cv::moveWindow(enhance_window_name_, 1050, 50);
    
    cv::createTrackbar("CLAHE Clip", enhance_window_name_, 
                      &trackbar_values_.clahe_clip_limit, 100, 
                      onTrackbarChanged, this);
    cv::createTrackbar("CLAHE Tile", enhance_window_name_, 
                      &trackbar_values_.clahe_tile_size, 32, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Gamma (x0.1)", enhance_window_name_, 
                      &trackbar_values_.gamma_correction, 50, 
                      onTrackbarChanged, this);
}

void ColorProcessorTuner::createMorphologyWindow() {
    cv::namedWindow(morph_window_name_, cv::WINDOW_NORMAL);
    cv::resizeWindow(morph_window_name_, 400, 150);
    cv::moveWindow(morph_window_name_, 1050, 50);
    
    cv::createTrackbar("Open Size", morph_window_name_, 
                      &trackbar_values_.morph_open_size, 20, 
                      onTrackbarChanged, this);
    cv::createTrackbar("Close Size", morph_window_name_, 
                      &trackbar_values_.morph_close_size, 20, 
                      onTrackbarChanged, this);
}

void ColorProcessorTuner::switchToPage(int page) {
    // 隐藏所有参数窗口
    cv::destroyWindow(lab_window_name_);
    cv::destroyWindow(hsv_red_window_name_);
    cv::destroyWindow(hsv_blue_window_name_);
    cv::destroyWindow(enhance_window_name_);
    cv::destroyWindow(morph_window_name_);
    
    current_page_ = page;
    
    // 如果参数窗口可见，显示当前页面的窗口
    if (param_windows_visible_) {
        switch(page) {
            case 0: createLABWindow(); break;
            case 1: createHSVRedWindow(); break;
            case 2: createHSVBlueWindow(); break;
            case 3: createEnhanceWindow(); break;
            case 4: createMorphologyWindow(); break;
        }
    }
}

void ColorProcessorTuner::showParamWindows(bool show) {
    param_windows_visible_ = show;
    
    if (show) {
        switchToPage(current_page_);
    } else {
        cv::destroyWindow(lab_window_name_);
        cv::destroyWindow(hsv_red_window_name_);
        cv::destroyWindow(hsv_blue_window_name_);
        cv::destroyWindow(enhance_window_name_);
        cv::destroyWindow(morph_window_name_);
    }
}

void ColorProcessorTuner::updateDisplay(const cv::Mat& original_frame,
                                       const cv::Mat& processed_frame,
                                       const cv::Mat& binary_result) {
    if (original_frame.empty()) return;
    
    // 创建主窗口显示图像
    cv::Mat main_display = createMainDisplay(original_frame, processed_frame, binary_result);
    
    // 显示主窗口
    cv::imshow(main_window_name_, main_display);
}

cv::Mat ColorProcessorTuner::createMainDisplay(const cv::Mat& original_frame,
                                              const cv::Mat& processed_frame,
                                              const cv::Mat& binary_result) const {
    // 创建主显示图像 (1000x700)
    cv::Mat display = cv::Mat::zeros(700, 1000, CV_8UC3);
    display.setTo(cv::Scalar(40, 40, 40));
    
    // 左侧：处理结果显示 (700x700)
    cv::Mat left_side = display(cv::Rect(0, 0, 700, 700));
    
    if (!processed_frame.empty() && !processed_frame.empty()) {
        // 如果调试图像太大，调整大小
        cv::Mat resized_debug;
        cv::resize(processed_frame, resized_debug, cv::Size(700, 525));
        
        // 将调试图像放到左侧上半部分
        cv::Rect debug_roi(0, 0, 700, 525);
        resized_debug.copyTo(left_side(debug_roi));
        
        // 在调试图像下方添加二进制结果
        if (!binary_result.empty()) {
            cv::Mat binary_display;
            cv::cvtColor(binary_result, binary_display, cv::COLOR_GRAY2BGR);
            cv::resize(binary_display, binary_display, cv::Size(700, 175));
            
            cv::Rect binary_roi(0, 525, 700, 175);
            binary_display.copyTo(left_side(binary_roi));
            
            // 添加标签
            cv::putText(left_side, "Binary Result", cv::Point(10, 545),
                       cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
        }
    }
    
    // 右侧：控制面板 (300x700)
    cv::Mat right_side = display(cv::Rect(700, 0, 300, 700));
    cv::Mat control_panel = createControlPanelImage();
    control_panel.copyTo(right_side);
    
    return display;
}

cv::Mat ColorProcessorTuner::createControlPanelImage() const {
    cv::Mat panel = cv::Mat::zeros(700, 300, CV_8UC3);
    panel.setTo(cv::Scalar(60, 60, 60));
    
    // 标题
    cv::putText(panel, "COLOR TUNER", cv::Point(80, 30),
               cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 255), 2);
    
    // 当前页面指示器
    std::vector<std::string> page_names = {"LAB", "HSV Red", "HSV Blue", "Enhance", "Morph"};
    std::string page_status = "Current: " + page_names[current_page_];
    if (!param_windows_visible_) page_status += " (Hidden)";
    
    cv::putText(panel, page_status, cv::Point(10, 60),
               cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(200, 200, 100), 1);
    
    // 按钮区域
    int y_start = 80;
    int button_height = 40;
    int button_spacing = 5;
    
    // 定义按钮
    struct Button {
        cv::Rect rect;
        std::string text;
        int page_index;
        bool active;
    };
    
    std::vector<Button> buttons = {
        {cv::Rect(20, y_start, 260, button_height), "LAB Parameters", 0, current_page_ == 0},
        {cv::Rect(20, y_start + button_height + button_spacing, 260, button_height), "HSV Red Parameters", 1, current_page_ == 1},
        {cv::Rect(20, y_start + 2*(button_height + button_spacing), 260, button_height), "HSV Blue Parameters", 2, current_page_ == 2},
        {cv::Rect(20, y_start + 3*(button_height + button_spacing), 260, button_height), "Enhancement Parameters", 3, current_page_ == 3},
        {cv::Rect(20, y_start + 4*(button_height + button_spacing), 260, button_height), "Morphology Parameters", 4, current_page_ == 4},
    };
    
    // 绘制按钮
    for (const auto& btn : buttons) {
        // 按钮颜色
        cv::Scalar color = btn.active ? 
                          cv::Scalar(0, 150, 255) :  // 激活状态：橙色
                          cv::Scalar(80, 80, 80);    // 非激活状态：灰色
        
        // 绘制按钮背景
        cv::rectangle(panel, btn.rect, color, -1);
        cv::rectangle(panel, btn.rect, cv::Scalar(200, 200, 200), 1);
        
        // 绘制按钮文字
        cv::putText(panel, btn.text,
                   cv::Point(btn.rect.x + 10, btn.rect.y + 25),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5,
                   cv::Scalar(255, 255, 255), 1);
    }
    
    // 参数窗口控制按钮
    int toggle_y = y_start + 5*(button_height + button_spacing) + 20;
    cv::Rect toggle_btn(20, toggle_y, 260, 40);
    cv::Scalar toggle_color = param_windows_visible_ ? 
                             cv::Scalar(0, 200, 0) : 
                             cv::Scalar(0, 0, 200);
    
    cv::rectangle(panel, toggle_btn, toggle_color, -1);
    cv::rectangle(panel, toggle_btn, cv::Scalar(200, 200, 200), 1);
    
    std::string toggle_text = param_windows_visible_ ? 
                             "Hide Parameter Windows" : 
                             "Show Parameter Windows";
    
    cv::putText(panel, toggle_text,
               cv::Point(toggle_btn.x + 10, toggle_btn.y + 25),
               cv::FONT_HERSHEY_SIMPLEX, 0.5,
               cv::Scalar(255, 255, 255), 1);
    
    // 参数显示区域
    int params_y = toggle_y + 60;
    cv::Rect params_rect(10, params_y, 280, 300);
    cv::rectangle(panel, params_rect, cv::Scalar(100, 100, 100), 1);
    
    // 显示当前参数
    int text_y = params_y + 20;
    int line_height = 20;
    
    auto addText = [&](const std::string& text, const cv::Scalar& color = cv::Scalar(220, 220, 220)) {
        cv::putText(panel, text, cv::Point(15, text_y),
                   cv::FONT_HERSHEY_SIMPLEX, 0.4, color, 1);
        text_y += line_height;
    };
    
    addText("Current Parameters:", cv::Scalar(0, 255, 255));
    addText("");
    
    // 根据当前页面显示不同的参数
    switch(current_page_) {
        case 0: // LAB
            addText("LAB Space:");
            addText("  Red A: " + std::to_string(current_params_.red_lab.lower[1]) + 
                   " - " + std::to_string(current_params_.red_lab.upper[1]));
            addText("  Blue B: " + std::to_string(current_params_.blue_lab.lower[0]) + 
                   " - " + std::to_string(current_params_.blue_lab.upper[0]));
            break;
            
        case 1: // HSV Red
            addText("HSV Red:");
            addText("  H1: " + std::to_string(current_params_.red_hsv.lower[0]) + 
                   "-" + std::to_string(current_params_.red_hsv.upper[0]));
            addText("  H2: 170-180");
            addText("  S: " + std::to_string(current_params_.red_hsv.lower[1]) + 
                   " - " + std::to_string(current_params_.red_hsv.upper[1]));
            addText("  V: " + std::to_string(current_params_.red_hsv.lower[2]) + 
                   " - " + std::to_string(current_params_.red_hsv.upper[2]));
            break;
            
        case 2: // HSV Blue
            addText("HSV Blue:");
            addText("  H: " + std::to_string(current_params_.blue_hsv.lower[0]) + 
                   " - " + std::to_string(current_params_.blue_hsv.upper[0]));
            addText("  S: " + std::to_string(current_params_.blue_hsv.lower[1]) + 
                   " - " + std::to_string(current_params_.blue_hsv.upper[1]));
            addText("  V: " + std::to_string(current_params_.blue_hsv.lower[2]) + 
                   " - " + std::to_string(current_params_.blue_hsv.upper[2]));
            break;
            
        case 3: // Enhancement
            addText("Enhancement:");
            addText("  CLAHE Clip: " + std::to_string(current_params_.clahe_clip_limit));
            addText("  CLAHE Tile: " + std::to_string(current_params_.clahe_tile_size));
            addText("  Gamma: " + std::to_string(current_params_.gamma_correction));
            break;
            
        case 4: // Morphology
            addText("Morphology:");
            addText("  Open Size: " + std::to_string(current_params_.morph_open_size));
            addText("  Close Size: " + std::to_string(current_params_.morph_close_size));
            break;
    }
    
    // 键盘快捷键说明
    text_y = params_y + 280;
    addText("Keyboard Shortcuts:", cv::Scalar(0, 255, 0));
    addText("  's' - Save parameters");
    addText("  'l' - Load parameters");
    addText("  'r' - Reset to default");
    addText("  'p' - Toggle param windows");
    addText("  ESC - Exit");
    
    return panel;
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
        cv::setTrackbarPos("Red A Low", lab_window_name_, trackbar_values_.lab_red_a_low);
    }
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

void ColorProcessorTuner::onMouse(int event, int x, int y, int flags, void* userdata) {
    ColorProcessorTuner* tuner = static_cast<ColorProcessorTuner*>(userdata);
    if (!tuner) return;
    
    if (event == cv::EVENT_LBUTTONDOWN) {
        tuner->handleButtonClick(x, y);
    }
}

void ColorProcessorTuner::handleButtonClick(int x, int y) {
    // 检查点击是否在右侧控制面板区域 (x从700开始，宽300)
    if (x >= 700 && x < 1000) {
        int panel_x = x - 700;
        int panel_y = y;
        
        // 检查页面选择按钮 (y: 80-320)
        if (panel_y >= 80 && panel_y < 320) {
            int button_index = (panel_y - 80) / 45;  // 每个按钮高40，间距5
            if (button_index >= 0 && button_index < 5) {
                switchToPage(button_index);
            }
        }
        
        // 检查参数窗口切换按钮 (y: 340-380)
        if (panel_y >= 340 && panel_y < 380) {
            param_windows_visible_ = !param_windows_visible_;
            showParamWindows(param_windows_visible_);
        }
    }
}

void ColorProcessorTuner::run(std::function<bool(cv::Mat&)> frame_source) {
    cv::Mat frame;
    
    // 创建颜色处理器
    ColorProcessor processor(current_params_);
    
    // 显示参数窗口（默认显示）
    showParamWindows(true);
    
    while (true) {
        // 获取帧
        if (!frame_source(frame)) {
            break;
        }
        
        if (frame.empty()) {
            std::cerr << "Empty frame!" << std::endl;
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
            std::cout << "Parameters reset to default" << std::endl;
        } else if (key == 'p' || key == 'P') {
            param_windows_visible_ = !param_windows_visible_;
            showParamWindows(param_windows_visible_);
        } else if (key >= '1' && key <= '5') {
            // 数字键1-5快速切换页面
            int page = key - '1';
            if (page >= 0 && page < 5) {
                switchToPage(page);
            }
        }
    }
    
    // 隐藏所有参数窗口
    showParamWindows(false);
}

void ColorProcessorTuner::loadParams(const std::string& filename) {
    // 先尝试加载参数文件
    if (!loadMainParams(filename)) {
        std::cerr << "Warning: Failed to load main parameters from " << filename << std::endl;
        return;
    }
    
    // 再尝试加载滑动条值文件（用于精确恢复）
    std::string trackbar_filename = filename + ".trackbars";
    loadTrackbarValues(trackbar_filename);
    
    // 更新显示
    params_changed_ = true;
    
    std::cout << "Parameters loaded from: " << filename << std::endl;
}

bool ColorProcessorTuner::loadMainParams(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open parameter file: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    std::string current_section;
    
    while (std::getline(file, line)) {
        // 跳过注释和空行
        if (line.empty() || line[0] == '#' || line[0] == '=') {
            continue;
        }
        
        // 检查是否是章节
        if (line[0] == '[' && line.back() == ']') {
            current_section = line.substr(1, line.size() - 2);
            continue;
        }
        
        // 解析键值对
        size_t equals_pos = line.find('=');
        if (equals_pos == std::string::npos) {
            continue; // 不是有效的键值对
        }
        
        std::string key = line.substr(0, equals_pos);
        std::string value_str = line.substr(equals_pos + 1);
        
        // 去除首尾空白字符
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value_str.erase(0, value_str.find_first_not_of(" \t"));
        value_str.erase(value_str.find_last_not_of(" \t") + 1);
        
        if (value_str.empty()) {
            continue; // 空值
        }
        
        try {
            // 根据章节和键更新参数
            if (current_section == "LAB_PARAMS") {
                if (key == "red_a_low") {
                    current_params_.red_lab.lower[1] = std::stoi(value_str);
                    trackbar_values_.lab_red_a_low = std::stoi(value_str);
                }
                else if (key == "red_a_high") {
                    current_params_.red_lab.upper[1] = std::stoi(value_str);
                    trackbar_values_.lab_red_a_high = std::stoi(value_str);
                }
                else if (key == "blue_b_low") {
                    current_params_.blue_lab.lower[0] = std::stoi(value_str);
                    trackbar_values_.lab_blue_b_low = std::stoi(value_str);
                }
                else if (key == "blue_b_high") {
                    current_params_.blue_lab.upper[0] = std::stoi(value_str);
                    trackbar_values_.lab_blue_b_high = std::stoi(value_str);
                }
            }
            else if (current_section == "HSV_RED_PARAMS") {
                if (key == "h1_low") {
                    current_params_.red_hsv.lower[0] = std::stoi(value_str);
                    trackbar_values_.hsv_red_h1_low = std::stoi(value_str);
                }
                else if (key == "h1_high") {
                    current_params_.red_hsv.upper[0] = std::stoi(value_str);
                    trackbar_values_.hsv_red_h1_high = std::stoi(value_str);
                }
                else if (key == "s_low") {
                    current_params_.red_hsv.lower[1] = std::stoi(value_str);
                    trackbar_values_.hsv_red_s_low = std::stoi(value_str);
                }
                else if (key == "s_high") {
                    current_params_.red_hsv.upper[1] = std::stoi(value_str);
                    trackbar_values_.hsv_red_s_high = std::stoi(value_str);
                }
                else if (key == "v_low") {
                    current_params_.red_hsv.lower[2] = std::stoi(value_str);
                    trackbar_values_.hsv_red_v_low = std::stoi(value_str);
                }
                else if (key == "v_high") {
                    current_params_.red_hsv.upper[2] = std::stoi(value_str);
                    trackbar_values_.hsv_red_v_high = std::stoi(value_str);
                }
            }
            else if (current_section == "HSV_BLUE_PARAMS") {
                if (key == "h_low") {
                    current_params_.blue_hsv.lower[0] = std::stoi(value_str);
                    trackbar_values_.hsv_blue_h_low = std::stoi(value_str);
                }
                else if (key == "h_high") {
                    current_params_.blue_hsv.upper[0] = std::stoi(value_str);
                    trackbar_values_.hsv_blue_h_high = std::stoi(value_str);
                }
                else if (key == "s_low") {
                    current_params_.blue_hsv.lower[1] = std::stoi(value_str);
                    trackbar_values_.hsv_blue_s_low = std::stoi(value_str);
                }
                else if (key == "s_high") {
                    current_params_.blue_hsv.upper[1] = std::stoi(value_str);
                    trackbar_values_.hsv_blue_s_high = std::stoi(value_str);
                }
                else if (key == "v_low") {
                    current_params_.blue_hsv.lower[2] = std::stoi(value_str);
                    trackbar_values_.hsv_blue_v_low = std::stoi(value_str);
                }
                else if (key == "v_high") {
                    current_params_.blue_hsv.upper[2] = std::stoi(value_str);
                    trackbar_values_.hsv_blue_v_high = std::stoi(value_str);
                }
            }
            else if (current_section == "ENHANCEMENT_PARAMS") {
                if (key == "clahe_clip") {
                    current_params_.clahe_clip_limit = std::stof(value_str);
                    trackbar_values_.clahe_clip_limit = static_cast<int>(current_params_.clahe_clip_limit * 10);
                }
                else if (key == "clahe_tile") {
                    current_params_.clahe_tile_size = std::stoi(value_str);
                    trackbar_values_.clahe_tile_size = current_params_.clahe_tile_size;
                }
                else if (key == "gamma") {
                    current_params_.gamma_correction = std::stof(value_str);
                    trackbar_values_.gamma_correction = static_cast<int>(current_params_.gamma_correction * 10);
                }
            }
            else if (current_section == "MORPHOLOGY_PARAMS") {
                if (key == "open_size") {
                    current_params_.morph_open_size = std::stoi(value_str);
                    trackbar_values_.morph_open_size = current_params_.morph_open_size;
                }
                else if (key == "close_size") {
                    current_params_.morph_close_size = std::stoi(value_str);
                    trackbar_values_.morph_close_size = current_params_.morph_close_size;
                }
            }
            else if (current_section == "DISPLAY_PARAMS") {
                if (key == "show_red") {
                    trackbar_values_.show_red = std::stoi(value_str);
                }
                else if (key == "show_blue") {
                    trackbar_values_.show_blue = std::stoi(value_str);
                }
                else if (key == "show_debug") {
                    trackbar_values_.show_debug = std::stoi(value_str);
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Warning: Error parsing line: " << line << std::endl;
            std::cerr << "Error: " << e.what() << std::endl;
            continue;
        }
    }
    
    file.close();
    return true;
}

void ColorProcessorTuner::loadTrackbarValues(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Info: No trackbar values file found: " << filename << std::endl;
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // 跳过注释和空行
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        size_t equals_pos = line.find('=');
        if (equals_pos == std::string::npos) {
            continue;
        }
        
        std::string key = line.substr(0, equals_pos);
        std::string value_str = line.substr(equals_pos + 1);
        
        // 去除首尾空白字符
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value_str.erase(0, value_str.find_first_not_of(" \t"));
        value_str.erase(value_str.find_last_not_of(" \t") + 1);
        
        if (value_str.empty()) {
            continue;
        }
        
        try {
            int value = std::stoi(value_str);
            
            // 更新对应的滑动条值
            if (key == "lab_red_a_low") trackbar_values_.lab_red_a_low = value;
            else if (key == "lab_red_a_high") trackbar_values_.lab_red_a_high = value;
            else if (key == "lab_blue_b_low") trackbar_values_.lab_blue_b_low = value;
            else if (key == "lab_blue_b_high") trackbar_values_.lab_blue_b_high = value;
            else if (key == "hsv_red_h1_low") trackbar_values_.hsv_red_h1_low = value;
            else if (key == "hsv_red_h1_high") trackbar_values_.hsv_red_h1_high = value;
            else if (key == "hsv_red_h2_low") trackbar_values_.hsv_red_h2_low = value;
            else if (key == "hsv_red_h2_high") trackbar_values_.hsv_red_h2_high = value;
            else if (key == "hsv_red_s_low") trackbar_values_.hsv_red_s_low = value;
            else if (key == "hsv_red_s_high") trackbar_values_.hsv_red_s_high = value;
            else if (key == "hsv_red_v_low") trackbar_values_.hsv_red_v_low = value;
            else if (key == "hsv_red_v_high") trackbar_values_.hsv_red_v_high = value;
            else if (key == "hsv_blue_h_low") trackbar_values_.hsv_blue_h_low = value;
            else if (key == "hsv_blue_h_high") trackbar_values_.hsv_blue_h_high = value;
            else if (key == "hsv_blue_s_low") trackbar_values_.hsv_blue_s_low = value;
            else if (key == "hsv_blue_s_high") trackbar_values_.hsv_blue_s_high = value;
            else if (key == "hsv_blue_v_low") trackbar_values_.hsv_blue_v_low = value;
            else if (key == "hsv_blue_v_high") trackbar_values_.hsv_blue_v_high = value;
            else if (key == "clahe_clip_limit") trackbar_values_.clahe_clip_limit = value;
            else if (key == "clahe_tile_size") trackbar_values_.clahe_tile_size = value;
            else if (key == "gamma_correction") trackbar_values_.gamma_correction = value;
            else if (key == "morph_open_size") trackbar_values_.morph_open_size = value;
            else if (key == "morph_close_size") trackbar_values_.morph_close_size = value;
            else if (key == "show_red") trackbar_values_.show_red = value;
            else if (key == "show_blue") trackbar_values_.show_blue = value;
            else if (key == "show_debug") trackbar_values_.show_debug = value;
        }
        catch (const std::exception& e) {
            std::cerr << "Warning: Error parsing trackbar line: " << line << std::endl;
            continue;
        }
    }
    
    file.close();
    
    // 更新参数结构体（从滑动条值）
    updateParamsFromTrackbars();
    
    // 更新当前活动窗口的滑动条位置
    updateActiveWindowTrackbars();
}

void ColorProcessorTuner::updateActiveWindowTrackbars() {
    // 根据当前页面更新对应窗口的滑动条位置
    if (param_windows_visible_) {
        switch(current_page_) {
            case 0: // LAB窗口
                if (cv::getWindowProperty(lab_window_name_, cv::WND_PROP_VISIBLE) >= 0) {
                    cv::setTrackbarPos("Red A Low", lab_window_name_, trackbar_values_.lab_red_a_low);
                    cv::setTrackbarPos("Red A High", lab_window_name_, trackbar_values_.lab_red_a_high);
                    cv::setTrackbarPos("Blue B Low", lab_window_name_, trackbar_values_.lab_blue_b_low);
                    cv::setTrackbarPos("Blue B High", lab_window_name_, trackbar_values_.lab_blue_b_high);
                }
                break;
            case 1: // HSV Red窗口
                if (cv::getWindowProperty(hsv_red_window_name_, cv::WND_PROP_VISIBLE) >= 0) {
                    cv::setTrackbarPos("Red H1 Low", hsv_red_window_name_, trackbar_values_.hsv_red_h1_low);
                    cv::setTrackbarPos("Red H1 High", hsv_red_window_name_, trackbar_values_.hsv_red_h1_high);
                    cv::setTrackbarPos("Red H2 Low", hsv_red_window_name_, trackbar_values_.hsv_red_h2_low);
                    cv::setTrackbarPos("Red H2 High", hsv_red_window_name_, trackbar_values_.hsv_red_h2_high);
                    cv::setTrackbarPos("Red S Low", hsv_red_window_name_, trackbar_values_.hsv_red_s_low);
                    cv::setTrackbarPos("Red S High", hsv_red_window_name_, trackbar_values_.hsv_red_s_high);
                    cv::setTrackbarPos("Red V Low", hsv_red_window_name_, trackbar_values_.hsv_red_v_low);
                    cv::setTrackbarPos("Red V High", hsv_red_window_name_, trackbar_values_.hsv_red_v_high);
                }
                break;
            case 2: // HSV Blue窗口
                if (cv::getWindowProperty(hsv_blue_window_name_, cv::WND_PROP_VISIBLE) >= 0) {
                    cv::setTrackbarPos("Blue H Low", hsv_blue_window_name_, trackbar_values_.hsv_blue_h_low);
                    cv::setTrackbarPos("Blue H High", hsv_blue_window_name_, trackbar_values_.hsv_blue_h_high);
                    cv::setTrackbarPos("Blue S Low", hsv_blue_window_name_, trackbar_values_.hsv_blue_s_low);
                    cv::setTrackbarPos("Blue S High", hsv_blue_window_name_, trackbar_values_.hsv_blue_s_high);
                    cv::setTrackbarPos("Blue V Low", hsv_blue_window_name_, trackbar_values_.hsv_blue_v_low);
                    cv::setTrackbarPos("Blue V High", hsv_blue_window_name_, trackbar_values_.hsv_blue_v_high);
                }
                break;
            case 3: // Enhance窗口
                if (cv::getWindowProperty(enhance_window_name_, cv::WND_PROP_VISIBLE) >= 0) {
                    cv::setTrackbarPos("CLAHE Clip", enhance_window_name_, trackbar_values_.clahe_clip_limit);
                    cv::setTrackbarPos("CLAHE Tile", enhance_window_name_, trackbar_values_.clahe_tile_size);
                    cv::setTrackbarPos("Gamma (x0.1)", enhance_window_name_, trackbar_values_.gamma_correction);
                }
                break;
            case 4: // Morph窗口
                if (cv::getWindowProperty(morph_window_name_, cv::WND_PROP_VISIBLE) >= 0) {
                    cv::setTrackbarPos("Open Size", morph_window_name_, trackbar_values_.morph_open_size);
                    cv::setTrackbarPos("Close Size", morph_window_name_, trackbar_values_.morph_close_size);
                }
                break;
        }
    }
}

void ColorProcessorTuner::setOnParamsChangedCallback(std::function<void(const ColorProcessorParams&)> callback) {
    on_params_changed_ = callback;
}

void ColorProcessorTuner::saveParams(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file for saving: " << filename << std::endl;
        return;
    }
    
    // 写入文件头
    file << "# =========================================" << std::endl;
    file << "# Color Processor Parameters" << std::endl;
    file << "# Generated by ColorProcessorTuner" << std::endl;
    file << "# Save time: " << __TIME__ << " " << __DATE__ << std::endl;
    file << "# =========================================" << std::endl;
    file << std::endl;
    
    // LAB参数
    file << "[LAB_PARAMS]" << std::endl;
    file << "# Red channel in LAB space (A component)" << std::endl;
    file << "red_a_low = " << current_params_.red_lab.lower[1] << std::endl;
    file << "red_a_high = " << current_params_.red_lab.upper[1] << std::endl;
    file << "# Blue channel in LAB space (B component)" << std::endl;
    file << "blue_b_low = " << current_params_.blue_lab.lower[0] << std::endl;
    file << "blue_b_high = " << current_params_.blue_lab.upper[0] << std::endl;
    file << std::endl;
    
    // HSV红色参数
    file << "[HSV_RED_PARAMS]" << std::endl;
    file << "# Red color in HSV space (two ranges)" << std::endl;
    file << "h1_low = " << current_params_.red_hsv.lower[0] << std::endl;
    file << "h1_high = " << current_params_.red_hsv.upper[0] << std::endl;
    file << "# Note: Red has second range: 170-180" << std::endl;
    file << "s_low = " << current_params_.red_hsv.lower[1] << std::endl;
    file << "s_high = " << current_params_.red_hsv.upper[1] << std::endl;
    file << "v_low = " << current_params_.red_hsv.lower[2] << std::endl;
    file << "v_high = " << current_params_.red_hsv.upper[2] << std::endl;
    file << std::endl;
    
    // HSV蓝色参数
    file << "[HSV_BLUE_PARAMS]" << std::endl;
    file << "# Blue color in HSV space" << std::endl;
    file << "h_low = " << current_params_.blue_hsv.lower[0] << std::endl;
    file << "h_high = " << current_params_.blue_hsv.upper[0] << std::endl;
    file << "s_low = " << current_params_.blue_hsv.lower[1] << std::endl;
    file << "s_high = " << current_params_.blue_hsv.upper[1] << std::endl;
    file << "v_low = " << current_params_.blue_hsv.lower[2] << std::endl;
    file << "v_high = " << current_params_.blue_hsv.upper[2] << std::endl;
    file << std::endl;
    
    // 图像增强参数
    file << "[ENHANCEMENT_PARAMS]" << std::endl;
    file << "# Image enhancement parameters" << std::endl;
    file << "clahe_clip = " << current_params_.clahe_clip_limit << std::endl;
    file << "clahe_tile = " << current_params_.clahe_tile_size << std::endl;
    file << "gamma = " << current_params_.gamma_correction << std::endl;
    file << std::endl;
    
    // 形态学参数
    file << "[MORPHOLOGY_PARAMS]" << std::endl;
    file << "# Morphological operation parameters" << std::endl;
    file << "open_size = " << current_params_.morph_open_size << std::endl;
    file << "close_size = " << current_params_.morph_close_size << std::endl;
    file << std::endl;
    
    // 显示控制参数
    file << "[DISPLAY_PARAMS]" << std::endl;
    file << "# Display control parameters" << std::endl;
    file << "show_red = " << trackbar_values_.show_red << std::endl;
    file << "show_blue = " << trackbar_values_.show_blue << std::endl;
    file << "show_debug = " << trackbar_values_.show_debug << std::endl;
    
    file.close();
    
    std::cout << "Parameters saved to: " << filename << std::endl;
}