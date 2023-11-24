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
    Timer() : m_running_(false), m_chan_(co_chan<RETURN_TYPE>(1)) {}

    void start(uint64_t millisec) {
        std::lock_guard<std::mutex> lock(m_mutex_);
        if (!m_running_) {
            m_timeout_millisec_ = millisec;
            m_running_ = true;
            go [this]() {
                std::unique_lock<std::mutex> lock(m_mutex_);
                while (m_running_) {
                    if (m_cv_.wait_for(lock, std::chrono::milliseconds(m_timeout_millisec_)) 
                        == std::cv_status::timeout) {
                        m_chan_ << RETURN_TYPE::TIME_OUT;
                    }
                }
            };
        }
    }

    void reset(uint64_t millisec) {
        std::lock_guard<std::mutex> lock(m_mutex_);
        m_timeout_millisec_ = millisec;
        if (!m_running_) {
            m_running_ = true;
            go [this]() {
                std::unique_lock<std::mutex> lock(m_mutex_);
                while (m_running_) {
                    if (m_cv_.wait_for(lock, std::chrono::milliseconds(m_timeout_millisec_)) 
                        == std::cv_status::timeout) {
                        m_chan_ << RETURN_TYPE::TIME_OUT;
                    }
                }
            };
        } else {
            m_cv_.notify_one();
        }
    }

    void stop() {
        std::lock_guard<std::mutex> lock(m_mutex_);
        m_running_ = false;
        m_cv_.notify_one();
    }

private:
    std::atomic<bool> m_running_;
    uint64_t m_timeout_millisec_{};
    std::mutex m_mutex_;
    std::condition_variable m_cv_;

public:
    co_chan<RETURN_TYPE> m_chan_;
};