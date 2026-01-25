#include <opencv2/opencv.hpp>

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    cv::Mat dummy(1, 1, CV_8UC1, cv::Scalar(0));
    return dummy.empty() ? 1 : 0;
}
