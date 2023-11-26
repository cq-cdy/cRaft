#pragma once

#include <libgo/context/context.h>
#include <libgo/coroutine.h>
#include <libgo/defer/defer.h>
#include <libgo/libgo.h>
#include "public.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

class Timer {
public:
    Timer() : m_running_(false), m_chan_(co_chan<RETURN_TYPE>(1)) {
        go[this]{
            while(true){
                std::unique_lock<std::mutex> lock(m_mutex_);
                if(!m_running_){
                    m_stop_cv_.wait(lock);
                    if(m_destory_){
                        break;
                    }
                }else{
                    if (m_cv_.wait_for(lock, std::chrono::milliseconds(m_timeout_millisec_))
                        == std::cv_status::timeout) {
                        //timeout event
                        m_chan_ << RETURN_TYPE::TIME_OUT;
                    }else{
                        //notify event
                    }
                    if(m_destory_){
                        break;
                    }
                }
                lock.unlock();
            }
        };
    }

public:
    void reset(uint64_t millisec){
        stop();
        reset_(millisec);
    }
    void stop() {
        std::lock_guard<std::mutex> lock(m_set_V_mtx_);
        m_running_ = false;
        m_cv_.notify_one();
    }
    void destory(){
        std::lock_guard<std::mutex> lock(m_set_V_mtx_);
        m_destory_ = true;
        m_running_ = false;
        m_cv_.notify_one();
        m_stop_cv_.notify_one();

    }
private:
    void reset_(uint64_t millisec) {
        std::lock_guard<std::mutex> lock(m_set_V_mtx_);
        m_running_ =true;
        m_stop_cv_.notify_one();
        m_timeout_millisec_ = millisec;
    }
private:
    std::atomic<bool> m_running_;
    uint64_t m_timeout_millisec_{};
    std::mutex m_mutex_;
    std::mutex m_set_V_mtx_;
    std::condition_variable m_cv_;
    std::condition_variable m_stop_cv_;
    bool m_destory_{false};

public:
    co_chan<RETURN_TYPE> m_chan_;
};