#include <opencv2/highgui.hpp>
#include <opencv2/core/async.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <string>
#include <iostream>


class Yolo{
private:
    cv::dnn::Net* net;

    void init(){
        
    }
public:
    Yolo(){
        cv::dnn::Net* input_net = &cv::dnn::readNetFromDarknet(std::string("yolov3.cfg"), std::string("yolov3.weights"));
        net = input_net;
        net->setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net->setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }

    Yolo(cv::dnn::Net* input_net): net(input_net){} 

    void train(){}
    void test(cv::Mat* frame){}
    void predict(cv::Mat* frame){

        float confThreshold = 0.5;
        float nmsThreshold = 0.4;
        int inpWidth = 416;
        int inpHeight = 416;
        cv::Mat* blob = &cv::dnn::blobFromImage(frame, 1/255.0, cv::Size(inpWidth, inpHeight), Scalar(0,0,0), true, false);

        // Sets input to network
        net.setInput(blob);

        // Runs forward pass to get output of output layers
        std::vector<cv::Mat> outputs;
        net.forward(outputs, getOutputsNames(net));

    }
};