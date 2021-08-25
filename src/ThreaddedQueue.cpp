#include <opencv2/highgui.hpp>
#include <iostream>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <vector>
#include <chrono>
#include "Yolo.cpp"

class Threadded_Queue{
private:

    // Can only be a type of std::chrono::duration, ie. milliseconds, microseconds, nanoseconds
    template <typename T>
    class FPS{
    private:
        T _start;
        T _end;
        T _last;
        int num_frames;
        int _fps;
        int fps_buffer;
    public:
  

        void start(){_start = now();}
        void stop(){_end = now();}
        T now(){return std::chrono::duration_cast<T>(std::chrono::system_clock::now().time_since_epoch());}
        void update(){num_frames++;}
        T elapsed(){ return _end - _start;}
        int avg_fps(){return num_frames / elapsed().count();}
        int fps(){
            T n = now();
            
        }
        FPS(): num_frames(0), fps_buffer(5) {start(); stop();}
    };

    std::queue<cv::Mat*> input_queue;
    int queue_size;
    std::mutex quetex;
    std::mutex camtex;
    cv::VideoCapture* capture;
    std::vector<std::thread> threads;
    Yolo yolo;

    static void* pc(void* th, int thn) {((Threadded_Queue *)th)->probe_cam(thn); return NULL;}
    static void* sv(void* th, int thn) {((Threadded_Queue *)th)->show_video(thn); return NULL;}

    virtual void probe_cam(int thn){
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

    void predict_yolo(cv::Mat* frame){
        yolo.predict(*frame, true);
    }

    virtual void show_video(int thn){
        while(true){
            quetex.lock();
            bool is_queue_empty = input_queue.empty();
            quetex.unlock();
            if(!is_queue_empty){
                quetex.lock();
                cv::Mat* frame = input_queue.front();
                quetex.unlock();

                predict_yolo(frame);

                //cv::imshow("edges", *frame);
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

        // std::thread probe_cam_thread1(pc, this,0);
        // std::thread probe_cam_thread2(pc, this,1);
        // std::thread probe_cam_thread3(pc, this,2);
        // std::thread show_video_thread(sv, this,3);

        yolo = Yolo();

        for (int i = 0; i < probe_threads; i++){
            try{
                threads.push_back(std::thread(pc, this, i));
            } catch (int err){
                std::cout << "Error creating thread." << '\n';
            }
        }
        for (int i = 0; i < show_threads; i++){
            try{
                threads.push_back(std::thread(sv, this, i + probe_threads));
            } catch (int err){
                std::cout << "Error creating thread." << '\n';
            }        
        }

        for (auto& it : threads){
            // pthread_join(**i, NULL);
            std::cout << "Thread joined" << '\n';
            if (it.joinable()){
                it.join();
            }
        }
        // probe_cam_thread1.join();
        // probe_cam_thread2.join();
        // probe_cam_thread3.join();
        // show_video_thread.join();
    }

public:


    Threadded_Queue(cv::VideoCapture* cap, int probe_threads = 1, int show_threads = 1): queue_size(0) {
        capture = cap;
        init(probe_threads, show_threads);
    }

    Threadded_Queue(int id = 0, int probe_threads = 1, int show_threads = 1): queue_size(0) {
        cv::VideoCapture cap(id);
        capture = &cap;
        init(probe_threads, show_threads);
    }
};