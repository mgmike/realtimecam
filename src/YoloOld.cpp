#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/async.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/core/cuda.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>


class Yolo{
private:
    cv::dnn::Net* net;
    double confThreshold = 0.5;
    double nmsThreshold = 0.4;
    int inpWidth = 416;
    int inpHeight = 416;
    std::vector<std::string> classes;

    std::vector<std::string> getOutputsNames(const cv::dnn::Net& net){
        static std::vector<std::string> names;
        if (names.empty()){
            //Get indices of output layers
            std::vector<int> outLayers = net.getUnconnectedOutLayers();

            // Get names of all layers
            std::vector<std::string> layersNames = net.getLayerNames();

            // Get names of output layers in names
            names.resize(outLayers.size());
            for(size_t i = 0; i < outLayers.size(); ++i)
                names[i] = layersNames[outLayers[i] - 1];
        }
        return names;
    }

    void postprocess(cv::Mat& frame, const std::vector<cv::Mat>& outputs){
        std::vector<int> classIds;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;

        for ( size_t i = 0; i < outputs.size(); ++i){
            // Scan through bounding boxes and only keep the high confidence boxes. 
            float* data = (float*)outputs[i].data;
            //navigate through rows, while also updating data to point to the next output element
            for (int j = 0; j < outputs[i].rows; ++j, data += outputs[i].cols){
                cv::Mat scores = outputs[i].row(j).colRange(5, outputs[i].cols);
                cv::Point classIdPoint;
                double confidence;
                // Get the value and location of the max score
                minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
                if(confidence > confThreshold){
                    int centerX = (int)(data[0] * frame.cols);
                    int centerY = (int)(data[1] * frame.rows);
                    int width = (int)(data[2] * frame.cols);
                    int height = (int)(data[3] * frame.rows);
                    int left = centerX - width / 2;
                    int top = centerY - height / 2;

                    classIds.push_back(classIdPoint.x);
                    confidences.push_back((float) confidence);
                    boxes.push_back(cv::Rect(left, top, width, height));
                }
            }
        }

        // Perform non maximum supression to eliminate redundant overlapping boxes with lower confidences
        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
        for (size_t i = 0; i < indices.size(); ++i){
            int idx = indices[i];
            cv::Rect box = boxes[idx];
            drawPred(classIds[idx], confidences[idx], box.x, box.y, box.x + box.width, box.y + box.height, frame);
        }
    }

    void drawPred(int classId, float conf, int left, int top, int right, int bottom, cv::Mat& frame){
        // Draw the bounding box rectangle
        cv::rectangle(frame, cv::Point(left,top), cv::Point(right, bottom), cv::Scalar(255, 178, 50), 3);

        // Get label for class name and confidence
        std::string label = cv::format("%.2f", conf);
        if(!classes.empty()){
            CV_Assert(classId < (int)classes.size());
            label = classes[classId] + ":" + label;
        }

        // Display the label at the top of the bounding box
        int baseLine;
        cv::Size labelSize = getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        top = cv::max(top, labelSize.height);
        cv::rectangle(frame, cv::Point(left, top - round(1.5*labelSize.height)), cv::Point(left + round(1.5*labelSize.width), top + baseLine), cv::Scalar(255, 255, 255), cv::FILLED);
        cv::putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0,0,0),1);

    }


public:
    Yolo(){
        std::string ycfg = "/media/mike/Storage/Documents/dishcam/cpp_proj/src/yolov3.cfg";
        std::string ywei = "/media/mike/Storage/Documents/dishcam/cpp_proj/src/yolov3.weights";
        cv::dnn::Net temp_net = cv::dnn::readNetFromDarknet(ycfg, ywei);
        net = &temp_net;
        //cv::cuda::printCudaDeviceInfo(0);
        net->setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        net->setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);

        // Load class names
        std::string classesFile = "coco.names";
        std::ifstream ifs(classesFile.c_str());
        std::string line;
        while (std::getline(ifs, line)) classes.push_back(line);
    }

    Yolo(cv::dnn::Net* input_net): net(input_net){} 

    void train(){}
    void test(cv::Mat* frame){}

    std::vector<cv::Mat>& predict(cv::Mat& frame, bool verbose = false){
        //cv::Mat blob;
        //cv::dnn::blobFromImage(frame, blob, 1/255.0, cv::Size(inpWidth, inpHeight), cv::Scalar(0,0,0), true, false);
        cv::Mat blob = cv::dnn::blobFromImage(frame, 1/255.0, cv::Size(inpWidth, inpHeight), cv::Scalar(0,0,0), true, false);


        if (verbose){
            cv::imshow("Temp", frame);
            cv::waitKey(0);
        }

        // Sets input to network
        net->setInput(blob);

        // Runs forward pass to get output of output layers
        std::vector<cv::Mat> outputs;
        net->forward(outputs, getOutputsNames(*net));

        postprocess(frame, outputs);

        // Put efficiency information. The function getPerfProfile returns the overall time for inference(t) and the timings of each of the layers(in layersTimers)
        std::vector<double> layersTimes;
        double freq = cv::getTickFrequency() / 1000;
        double t = net->getPerfProfile(layersTimes) / freq;
        std::string label = cv::format("Inference time for a frame : %.2f ms", t);
        cv::putText(frame, label, cv::Point(0, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255));

        // Write the frame with the detection 
        cv::Mat detectedFrame;
        frame.convertTo(detectedFrame, CV_8U);
        if (verbose){
            cv::imshow("Prediction", frame);
            cv::waitKey(0);
        }
        return outputs;
    }
};