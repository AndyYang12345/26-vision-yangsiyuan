#include <opencv2/opencv.hpp>
#include "detector/ArmorDetectionPipeline.hpp"
#include <iostream>

int main(int argc, char** argv) {
    // 检查命令行参数
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <config_path> <image_path>" << std::endl;
        return -1;
    }
    
    std::string config_path = argv[1];
    std::string image_path = argv[2];
    
    // 初始化检测管线
    ArmorDetectionPipeline pipeline(config_path);
    
    // 读取图像
    cv::Mat frame = cv::imread(image_path);
    if (frame.empty()) {
        std::cerr << "Failed to read image: " << image_path << std::endl;
        return -1;
    }
    
    // 处理图像
    DetectionResult result = pipeline.process(frame);
    
    // 输出结果
    if (result.success) {
        std::cout << "Detected " << result.armors.size() << " armors." << std::endl;
        for (const auto& armor : result.armors) {
            std::cout << "Armor ID: " << armor.id << ", Position: " << armor.position << std::endl;
        }
    } else {
        std::cout << "No armors detected." << std::endl;
    }
    
    // 显示调试图像
    cv::imshow("Debug Image", result.debug_image);
    cv::waitKey(0);
    
    return 0;
}  