//
// Created by nuc on 23-5-7.
//

#ifndef OPENVINO_TEST_OPENVINOINFER_H
#define OPENVINO_TEST_OPENVINOINFER_H

#include <opencv2/opencv.hpp>
#include <openvino/openvino.hpp>
#include <algorithm>
#include <vector>

//用前先根据模型修改好要修改的地方

using namespace cv;
using namespace std;


struct Object
{
    cv::Rect_<float> rect; //
    float landmarks[8]; //4个关键点
    int label;
    float prob;
    int color;      //blue:1 , red:0
    double length;
    double width;
    double ratio;   //length s/ width
};

class OpenvinoInfer {
public:
    vector<Object> objects;
    const int IMAGE_HEIGHT = 640;
    const int IMAGE_WIDTH = 640;
    double ans;
    vector<double> ious;
    vector<Object> tmp_objects;
    std::shared_ptr<ov::Model> model;
    ov::Core core;
    ov::preprocess::PrePostProcessor *ppp;
    ov::CompiledModel compiled_model;
    ov::Shape input_shape;
    float conf_threshold = 0.25f;
    float nms_threshold = 0.45f;

    OpenvinoInfer(){}
    OpenvinoInfer(string model_path_xml, string model_path_bin, string device);
    void infer(Mat img,int detect_color);
    OpenvinoInfer(string model_path, string device);
    void set_thresholds(float conf, float nms) {
        conf_threshold = conf;
        nms_threshold = nms;
    }

    void infer(Mat img, float* detections){
        // Step 3. Read input image
        // resize image
        resize(img,img,Size(IMAGE_WIDTH,IMAGE_HEIGHT));
        if(img.channels() == 3) cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);


        // Step 5. Create tensor from image
        int rows = img.rows;
        int cols = img.cols;
        uchar* input_data = (uchar *)img.data; // 创建一个新的float数组

        ov::Tensor input_tensor = ov::Tensor(compiled_model.input().get_element_type(), compiled_model.input().get_shape(), input_data);


        // Step 6. Create an infer request for model inference
        ov::InferRequest infer_request = compiled_model.create_infer_request();
        infer_request.set_input_tensor(input_tensor);
        infer_request.infer();


        //Step 7. Retrieve inference results
        ov::Tensor output_tensor = infer_request.get_output_tensor();
        ov::Shape output_shape = output_tensor.get_shape();
        float* output_data = output_tensor.data<float>();
        std::copy(output_data, output_data + 9, detections);
    }
    double sigmoid(double x) {
        if(x>0)
            return 1.0 / (1.0 + exp(-x));
        else
            return exp(x) / (1.0 + exp(x));
    }

        double cal_iou(const cv::Rect& r1, const cv::Rect& r2)
    {
        float x_left = std::fmax(r1.x, r2.x);
        float y_top = std::fmax(r1.y, r2.y); 
        float x_right = std::fmin(r1.x + r1.width, r2.x + r2.width);
        float y_bottom = std::fmin(r1.y + r1.height, r2.y + r2.height);

        if (x_right < x_left || y_bottom < y_top) {
            return 0.0; 
        }

        double in_area = (x_right - x_left) * (y_bottom - y_top);
        double un_area = r1.area() + r2.area() - in_area; 

        return in_area / un_area;
    }

    double meaning(float x, int len){
        if(len == 0) ans = x;
        else{
            ans = (len * ans + x) / (len+1);
        }
        return ans;
    }

    ~OpenvinoInfer(){
        delete ppp;
    }
};

#endif //OPENVINO_TEST_OPENVINOINFER_H
