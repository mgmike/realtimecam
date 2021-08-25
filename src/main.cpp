
#include <tbb/concurrent_queue.h>
#include <opencv2/highgui.hpp>
#include "ThreaddedQueue.cpp"
#include "DishcamConfig.h"
// g++ main.cpp -o program -std=c++11 -pthread -ltbb_debug `pkg-config --cflags --libs opencv`
//Make sure to include lpthread
using namespace std;
using namespace cv;


int main(){

    cout << CV_VERSION << '\n';

    cv::Mat img = cv::imread("/media/mike/Storage/Documents/dishcam/cpp_proj/src/bird.jpg");
    
    Yolo yolo;
    yolo.predict(img);


    // cv::VideoCapture capture(0);
    // capture.set(cv::CAP_PROP_POS_FRAMES, 60);
    // capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    // capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    // Threadded_Queue tq(&capture, 2, 1);

    return 0;
}