#pragma once

#include <opencv2/opencv.hpp>
#include <memory>
#include <vector>

/**
 * @brief 颜色处理参数结构体
 */
struct ColorProcessorParams {
    // 颜色分离阈值
    struct ColorThreshold {
        cv::Scalar lower;  // 下界
        cv::Scalar upper;  // 上界
    };
    
    ColorThreshold red_lab;    // LAB空间的红色阈值
    ColorThreshold blue_lab;   // LAB空间的蓝色阈值
    ColorThreshold red_hsv;    // HSV空间的红色阈值
    ColorThreshold blue_hsv;   // HSV空间的蓝色阈值
    
    // 图像增强参数
    int clahe_clip_limit;      // CLAHE对比度限制
    int clahe_tile_size;       // CLAHE网格大小
    float gamma_correction;    // Gamma校正值
    
    // 形态学操作参数
    int morph_open_size;       // 开运算核大小
    int morph_close_size;      // 闭运算核大小
    
    // 构造函数：设置默认参数
    ColorProcessorParams();
};

/**
 * @brief 颜色处理器类 - 负责装甲板的颜色分离和预处理
 */
class ColorProcessor {
public:
    /**
     * @brief 构造函数
     * @param params 颜色处理参数
     */
    explicit ColorProcessor(const ColorProcessorParams& params = ColorProcessorParams());
    
    /**
     * @brief 析构函数
     */
    ~ColorProcessor() = default;
    
    /**
     * @brief 处理单帧图像，分离红蓝颜色
     * @param frame 输入图像 (BGR格式)
     * @param is_red_target 是否以红色为目标
     * @return 目标颜色二值图像
     */
    cv::Mat process(const cv::Mat& frame, bool is_red_target = true);
    
    /**
     * @brief 分别获取红色和蓝色的二值图像
     * @param frame 输入图像
     * @param red_binary 输出红色二值图像
     * @param blue_binary 输出蓝色二值图像
     */
    void separateColors(const cv::Mat& frame, cv::Mat& red_binary, cv::Mat& blue_binary);
    
    /**
     * @brief 更新处理参数
     * @param params 新的参数
     */
    void updateParams(const ColorProcessorParams& params);
    
    /**
     * @brief 获取当前参数
     * @return 当前参数
     */
    ColorProcessorParams getParams() const { return params_; }
    
    /**
     * @brief 应用图像增强
     * @param frame 输入图像
     * @return 增强后的图像
     */
    cv::Mat enhanceImage(const cv::Mat& frame);
    
    /**
     * @brief 获取调试图像（用于可视化）
     * @return 包含各阶段处理的调试图像
     */
    cv::Mat getDebugImage() const { return debug_image_; }
    
private:
    ColorProcessorParams params_;  // 处理参数
    cv::Mat debug_image_;          // 调试图像
    
    /**
     * @brief 在LAB颜色空间分离颜色
     * @param frame 输入图像
     * @param color_type 颜色类型 (0:红色, 1:蓝色)
     * @return 二值图像
     */
    cv::Mat separateColorLAB(const cv::Mat& frame, int color_type);
    
    /**
     * @brief 在HSV颜色空间分离颜色
     * @param frame 输入图像
     * @param color_type 颜色类型 (0:红色, 1:蓝色)
     * @return 二值图像
     */
    cv::Mat separateColorHSV(const cv::Mat& frame, int color_type);
    
    /**
     * @brief 合并LAB和HSV的分离结果
     * @param lab_binary LAB空间结果
     * @param hsv_binary HSV空间结果
     * @return 合并后的二值图像
     */
    cv::Mat mergeColorChannels(const cv::Mat& lab_binary, const cv::Mat& hsv_binary);
    
    /**
     * @brief 应用形态学操作
     * @param binary 输入二值图像
     * @param is_red 是否为红色（使用不同的核大小）
     * @return 处理后的二值图像
     */
    cv::Mat applyMorphology(const cv::Mat& binary, bool is_red);
    
    /**
     * @brief 红色特殊处理（处理红色在HSV空间中的环绕问题）
     * @param hsv_image HSV图像
     * @return 红色二值图像
     */
    cv::Mat processRedSpecial(const cv::Mat& hsv_image);
    
    /**
     * @brief 创建调试图像
     * @param original 原始图像
     * @param enhanced 增强图像
     * @param red_binary 红色二值图
     * @param blue_binary 蓝色二值图
     * @param target_binary 目标二值图
     */
    void createDebugImage(const cv::Mat& original, const cv::Mat& enhanced,
                         const cv::Mat& red_binary, const cv::Mat& blue_binary,
                         const cv::Mat& target_binary);
};