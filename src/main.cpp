#include "ThreaddedQueue.cpp"
#include <tbb/concurrent_queue.h>
#include <opencv2/highgui.hpp>
// g++ main.cpp -o program -std=c++11 -pthread -ltbb_debug `pkg-config --cflags --libs opencv`
//Make sure to include lpthread
using namespace std;


int main(){
    cv::VideoCapture capture(0);
    capture.set(cv::CAP_PROP_POS_FRAMES, 60);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    Threadded_Queue tq(&capture, 2, 1);
}