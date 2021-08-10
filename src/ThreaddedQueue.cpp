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
    std::vector<std::thread> threads;

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

    virtual void show_video(int thn){
        while(true){
            quetex.lock();
            bool is_queue_empty = input_queue.empty();
            quetex.unlock();
            if(!is_queue_empty){
                quetex.lock();
                cv::Mat* frame = input_queue.front();
                quetex.unlock();

                cv::imshow("edges", *frame);
                std::cout << "There are " << queue_size << " frames in the queue, from thread " << thn << '\n';
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

        std::thread probe_cam_thread1(pc, this,0);
        std::thread probe_cam_thread2(pc, this,1);
        std::thread probe_cam_thread3(pc, this,2);
        std::thread show_video_thread(sv, this,3);

        //std::thread show_cam_thread(show, &capture, camtex, quetex, input_queue, queue_size);


        // for (int i = 0; i < probe_threads; i++){
        //     try{
        //         threads.push_back(std::thread(pc, this, i));
        //     } catch (int err){
        //         std::cout << "Error thrown." << '\n';
        //     }
        // }
        // for (int i = 0; i < show_threads; i++){
        //     try{
        //         threads.push_back(std::thread(sv, this, i));
        //     } catch (int err){
        //         std::cout << "Error thrown." << '\n';
        //     }        
        // }

        // for (std::vector<std::thread>::iterator it = threads.begin(); it != threads.end(); ++it){
        //     // pthread_join(**i, NULL);
        //     std::cout << "Thread joined" << '\n';
        //     if (it->joinable()){
        //         it->join();
        //     }
        // }
        probe_cam_thread1.join();
        probe_cam_thread2.join();
        probe_cam_thread3.join();
        show_video_thread.join();
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