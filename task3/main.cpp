#include "OpenvinoInfer.h"

#include <opencv2/opencv.hpp>
#include <iostream>

static cv::Scalar color_for_obj(const Object& obj) {
    if (obj.color == 1) { // blue
        return cv::Scalar(255, 0, 0);
    }
    if (obj.color == 0) { // red
        return cv::Scalar(0, 0, 255);
    }
    return cv::Scalar(0, 255, 0);
}

static bool ends_with(const std::string& s, const std::string& suffix) {
    if (s.size() < suffix.size()) {
        return false;
    }
    return s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0]
                  << " <video_path|0> [model_path.(onnx|xml)] [model_bin] [device] [detect_color] [conf] [nms]\n"
                  << "  device: CPU/GPU/NPU... (default CPU)\n"
                  << "  detect_color: -1 all, 0 blue, 1 red (default -1)\n"
                  << "  conf: confidence threshold (default 0.25)\n"
                  << "  nms: nms threshold (default 0.45)\n"
                  << "  default model: Model/0526.onnx\n";
        return 1;
    }

    const std::string video_path = argv[1];
    const std::string model_path = (argc >= 3) ? argv[2] : "Model/0526.onnx";
    const bool is_onnx = ends_with(model_path, ".onnx") || ends_with(model_path, ".ONNX");

    std::string model_bin;
    int next_arg = 3;
    if (!is_onnx) {
        if (argc < 4) {
            std::cerr << "Error: XML model requires a .bin path.\n";
            return 1;
        }
        model_bin = argv[3];
        next_arg = 4;
    }

    const std::string device = (argc >= next_arg + 1) ? argv[next_arg] : "CPU";
    const int detect_color = (argc >= next_arg + 2) ? std::atoi(argv[next_arg + 1]) : -1;
    const float conf = (argc >= next_arg + 3) ? std::atof(argv[next_arg + 2]) : 0.25f;
    const float nms = (argc >= next_arg + 4) ? std::atof(argv[next_arg + 3]) : 0.45f;

    OpenvinoInfer infer = is_onnx ? OpenvinoInfer(model_path, device)
                                  : OpenvinoInfer(model_path, model_bin, device);
    infer.set_thresholds(conf, nms);

    cv::VideoCapture cap;
    if (video_path == "0") {
        cap.open(0);
    } else {
        // Prefer FFmpeg for file input; fall back to default backend.
        cap.open(video_path, cv::CAP_FFMPEG);
        if (!cap.isOpened()) {
            cap.open(video_path);
        }
    }

    if (!cap.isOpened()) {
        std::cerr << "Failed to open video/camera: " << video_path << std::endl;
        return 1;
    }

    const int input_w = 640;
    const int input_h = 640;
    const double fps = cap.get(cv::CAP_PROP_FPS);
    const int delay_ms = (fps > 0.0) ? static_cast<int>(1000.0 / fps) : 33;

    cv::Mat frame;
    while (cap.read(frame)) {
        if (frame.empty()) {
            break;
        }

        cv::Mat display;
        if (frame.cols != input_w || frame.rows != input_h) {
            cv::resize(frame, display, cv::Size(input_w, input_h));
        } else {
            display = frame;
        }

        infer.infer(display, detect_color);

        for (const auto& obj : infer.tmp_objects) {
            const cv::Scalar c = color_for_obj(obj);
            cv::rectangle(display, obj.rect, c, 2);

            // Draw 4 keypoints
            for (int i = 0; i < 4; ++i) {
                cv::circle(display, cv::Point2f(obj.landmarks[i * 2], obj.landmarks[i * 2 + 1]), 3, c, -1);
            }

            char text[64];
            std::snprintf(text, sizeof(text), "id=%d c=%d p=%.2f", obj.label, obj.color, obj.prob);
            cv::putText(display, text, cv::Point(obj.rect.x, obj.rect.y - 5),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, c, 1);
        }

        cv::imshow("OpenVINO Demo", display);
        if (cv::waitKey(std::max(1, delay_ms)) == 27) { // ESC
            break;
        }
    }

    return 0;
}
