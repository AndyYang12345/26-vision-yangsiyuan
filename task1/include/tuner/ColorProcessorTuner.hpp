#pragma once

#include <opencv2/opencv.hpp>
#include "preprocessor/ColorProcessor.hpp"
#include <string>
#include <functional>

/**
 * @brief 颜色处理器调参器
 * 提供GUI界面实时调整颜色处理参数
 */
class ColorProcessorTuner {
public:
    /**
     * @brief 构造函数
     * @param window_name 窗口名称
     * @param initial_params 初始参数
     */
    ColorProcessorTuner(const std::string& window_name = "Color Processor Tuner",
                       const ColorProcessorParams& initial_params = ColorProcessorParams());
    
    /**
     * @brief 析构函数
     */
    ~ColorProcessorTuner();
    
    /**
     * @brief 创建调参窗口
     */
    void createTuningWindow();
    
    /**
     * @brief 更新图像显示
     * @param original_frame 原始图像
     * @param processed_frame 处理后的图像
     * @param binary_result 二值化结果
     */
    void updateDisplay(const cv::Mat& original_frame,
                      const cv::Mat& processed_frame,
                      const cv::Mat& binary_result);
    
    /**
     * @brief 获取当前参数
     * @return 当前参数
     */
    ColorProcessorParams getCurrentParams() const { return current_params_; }
    
    /**
     * @brief 保存参数到文件
     * @param filename 文件名
     */
    void saveParams(const std::string& filename);
    
    /**
     * @brief 从文件加载参数
     * @param filename 文件名
     */
    void loadParams(const std::string& filename);
    
    /**
     * @brief 设置回调函数，当参数改变时调用
     * @param callback 回调函数
     */
    void setOnParamsChangedCallback(std::function<void(const ColorProcessorParams&)> callback);
    
    /**
     * @brief 检查是否需要更新参数
     * @return 是否需要更新
     */
    bool needsUpdate() const { return params_changed_; }
    
    /**
     * @brief 标记参数已更新
     */
    void markUpdated() { params_changed_ = false; }
    
    /**
     * @brief 运行调参循环
     * @param frame_source 帧获取函数（返回是否成功）
     */
    void run(std::function<bool(cv::Mat&)> frame_source);
    
private:
    std::string window_name_;
    ColorProcessorParams current_params_;
    ColorProcessorParams default_params_;
    bool params_changed_;
    std::function<void(const ColorProcessorParams&)> on_params_changed_;
    
    // 滑动条值（用于避免浮点精度问题）
    struct TrackbarValues {
        // LAB空间阈值
        int lab_red_a_low = 150;
        int lab_red_a_high = 255;
        int lab_blue_b_low = 100;
        int lab_blue_b_high = 255;
        
        // HSV空间阈值
        int hsv_red_h1_low = 0;
        int hsv_red_h1_high = 10;
        int hsv_red_h2_low = 170;
        int hsv_red_h2_high = 180;
        int hsv_red_s_low = 150;
        int hsv_red_s_high = 255;
        int hsv_red_v_low = 50;
        int hsv_red_v_high = 255;
        
        int hsv_blue_h_low = 100;
        int hsv_blue_h_high = 130;
        int hsv_blue_s_low = 150;
        int hsv_blue_s_high = 255;
        int hsv_blue_v_low = 50;
        int hsv_blue_v_high = 255;
        
        // 图像增强参数
        int clahe_clip_limit = 20;  // 实际值 = value/10.0
        int clahe_tile_size = 8;
        int gamma_correction = 12;   // 实际值 = value/10.0
        
        // 形态学参数
        int morph_open_size = 3;
        int morph_close_size = 5;
        
        // 显示控制
        int show_red = 1;
        int show_blue = 1;
        int show_debug = 1;
    } trackbar_values_;
    
    /**
     * @brief 创建滑动条
     */
    void createTrackbars();
    
    /**
     * @brief 更新参数结构体
     */
    void updateParamsFromTrackbars();
    
    /**
     * @brief 更新滑动条从参数
     */
    void updateTrackbarsFromParams();
    
    /**
     * @brief 静态回调函数
     */
    static void onTrackbarChanged(int, void* userdata);
    
    /**
     * @brief 创建参数显示图像
     * @return 参数显示图像
     */
    cv::Mat createParamsDisplayImage() const;
};