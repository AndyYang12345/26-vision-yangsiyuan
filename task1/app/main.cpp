#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "detector/ArmorDetectionPipeline.hpp"
#include "types/Params.hpp"

int main(int argc, char** argv) {
    std::cout << "========================================" << std::endl;
    std::cout << "        Armor Detection System          " << std::endl;
    std::cout << "========================================" << std::endl;
    
    Params params{};
    params.binarize_method = 1;
    ArmorDetectionPipeline pipeline(params);
    
    // 打开摄像头
    cv::VideoCapture cap;
    if (argc > 1) {
        cap.open(argv[1]);  // 使用视频文件
        std::cout << "Opening video file: " << argv[1] << std::endl;
    } else {
        cap.open(0);  // 使用默认摄像头
        std::cout << "Opening camera #0" << std::endl;
    }
    
    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open video source!" << std::endl;
        return -1;
    }
    
    // 设置摄像头参数
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    cap.set(cv::CAP_PROP_FPS, 120);
    cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
    
    cv::Mat frame;
    int frame_count = 0;
    
    bool enemy_is_red = true;
    bool paused = false;
    while (true) {
        if (!paused) {
            cap >> frame;
            if (frame.empty()) {
                std::cerr << "Error: Cannot read frame!" << std::endl;
                break;
            }
        }
        
        if (!paused) {
            frame_count++;
        }
        
        // 处理帧
        if (!paused) {
            DetectionResult result = pipeline.process(frame, enemy_is_red);
            
            cv::Mat display = result.armors_vis.empty() ? frame : result.armors_vis.clone();
            cv::imshow("LightBars", result.lightbars_vis);
            cv::imshow("Armors", display);
        
            // 计算并显示FPS
            static double fps = 0;
            static auto last_time = std::chrono::steady_clock::now();
            auto current_time = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double>(current_time - last_time).count();
        
            if (elapsed > 0.5) {  // 每0.5秒更新一次FPS
                fps = frame_count / elapsed;
                frame_count = 0;
                last_time = current_time;
            }
        
            cv::putText(display,
                       "FPS: " + std::to_string(static_cast<int>(fps)),
                       cv::Point(10, 90),
                       cv::FONT_HERSHEY_SIMPLEX, 0.7,
                       cv::Scalar(0, 255, 255), 2);
            cv::imshow("Armors", display);
        }
        
        int key = cv::waitKey(30);
        if (key == 27) {  // ESC
            break;
        } else if (key == ' ') {
            paused = !paused;
        }
    }
    
    // 清理
    cap.release();
    cv::destroyAllWindows();
    
    std::cout << "\nProgram finished." << std::endl;
    return 0;
}
