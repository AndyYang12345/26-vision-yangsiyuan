#include <iostream>
#include <opencv2/opencv.hpp>
#include "tuner/ColorProcessorTuner.hpp"

int main(int argc, char** argv) {
    std::cout << "========================================" << std::endl;
    std::cout << "    Simple Color Processor Tuner       " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  Click buttons on the right panel to switch parameter pages" << std::endl;
    std::cout << "  Adjust sliders in parameter windows" << std::endl;
    std::cout << "  Real-time effect preview in main window" << std::endl;
    std::cout << std::endl;
    std::cout << "Keyboard Shortcuts:" << std::endl;
    std::cout << "  '1'-'5'  : Quick switch to page 1-5" << std::endl;
    std::cout << "  's'      : Save parameters to file" << std::endl;
    std::cout << "  'l'      : Load parameters from file" << std::endl;
    std::cout << "  'r'      : Reset to default parameters" << std::endl;
    std::cout << "  'p'      : Toggle parameter windows" << std::endl;
    std::cout << "  ESC      : Exit program" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建调参器
    ColorProcessorTuner tuner("Color Tuner v1.0");
    tuner.createTuningWindow();
    
    // 打开摄像头或视频文件
    cv::VideoCapture cap;
    if (argc > 1) {
        // 使用视频文件
        cap.open(argv[1]);
        std::cout << "Opening video file: " << argv[1] << std::endl;
    } else {
        // 使用默认摄像头
        cap.open(0);
        std::cout << "Opening default camera" << std::endl;
    }
    
    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open video source!" << std::endl;
        return -1;
    }
    
    // 设置摄像头参数
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    
    // 定义帧获取函数
    bool loop_video = true;  // 控制是否循环播放

    auto frame_source = [&](cv::Mat& frame) -> bool {
        bool success = cap.read(frame);
        
        if (!success && loop_video) {
            // 视频播放完毕，重置到开头
            std::cout << "Video ended. Restarting..." << std::endl;
            
            // 重置视频捕获
            cap.set(cv::CAP_PROP_POS_FRAMES, 0);
            
            // 再次尝试读取
            success = cap.read(frame);
            
            if (!success) {
                std::cerr << "Error: Cannot restart video!" << std::endl;
            }
        }
        
        return success;
    };
    
    // 运行调参器
    tuner.run(frame_source);
    
    // 清理
    cap.release();
    cv::destroyAllWindows();
    
    std::cout << "Program finished." << std::endl;
    return 0;
}