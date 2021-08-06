#include <opencv2/highgui.hpp>
#include <iostream>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <vector>

class Threadded_Queue{
private:
    std::queue<cv::Mat*> input_queue;
    int queue_size;
    std::mutex quetex;
    std::mutex camtex;
    cv::VideoCapture* capture;
    std::vector<std::thread*> threads;

    static void* pc(void* th) {((Threadded_Queue *)th)->probe_cam(); return NULL;}
    static void* sv(void* th) {((Threadded_Queue *)th)->show_video(); return NULL;}

    virtual void probe_cam(){
        if (!capture->isOpened()){
            std::cout << "Camera cannot be opened." << '\n';
            return;
        }
        
        cv::Mat frame;
        while(true)
        {
            camtex.lock();
            *capture >> frame;
            camtex.unlock();
            //cap.read(frame);
            quetex.lock();
            input_queue.push(&frame);
            queue_size++;
            quetex.unlock();
        }
    }

    virtual void show_video(){
        while(true){
            quetex.lock();
            bool is_queue_empty = input_queue.empty();
            quetex.unlock();
            if(!is_queue_empty){
                quetex.lock();
                cv::Mat* frame = input_queue.front();
                quetex.unlock();

                cv::imshow("edges", *frame);
                std::cout << "There are " << queue_size << " frames in the queue." << '\n';
                if(cv::waitKey(1) >= 0){
                    capture->release();
                    return;
                }
                
                quetex.lock();
                input_queue.pop();
                quetex.unlock();
                //delete frame;
                queue_size--;
            }
        }
    }

    void init(int probe_threads, int show_threads){
        int num_threads = probe_threads + show_threads;

        typedef void * (*THREADFUNCPTR)(void *);


        for (int i = 0; i < probe_threads; i++){
            std::thread probe_cam_thread(pc, this);
            threads.push_back(&probe_cam_thread);
            //pthread_create(threads.back(), NULL, pc, this);
        }
        for (int i = 0; i < show_threads; i++){
            std::thread show_video_thread(sv, this);
            threads.push_back(&show_video_thread);
            //pthread_create(threads.back(), NULL, sv, this);
        }
        
        std::cout << &(*threads.begin()) << '\n';

        for (auto i = threads.begin(); i != threads.end(); ++i){
            // pthread_join(**i, NULL);
            (*i)->join();
        }
    }

public:

    Threadded_Queue(): queue_size(0){}

    Threadded_Queue(cv::VideoCapture* cap, int probe_threads = 1, int show_threads = 1): Threadded_Queue() {
        capture = cap;
        init(probe_threads, show_threads);
    }

    Threadded_Queue(int id, int probe_threads = 1, int show_threads = 1): Threadded_Queue() {
        cv::VideoCapture cap(id);
        capture = &cap;
        init(probe_threads, show_threads);
    }


};