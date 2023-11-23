#pragma once

#include <libgo/context/context.h>
#include <libgo/coroutine.h>
#include <libgo/defer/defer.h>
#include <libgo/libgo.h>
#include <stdio.h>
#include "public.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

class Timer {
public:
    Timer()
            : m_running_(false), m_timeout_(false), m_chan_(co_chan<RETURN_TYPE>(1)) {

    }

    void start(uint64_t millisec) {
        std::unique_lock<std::mutex> lock(m_mutex_);
        if (!m_running_) {
            m_timeout_millisec_ = millisec;
            m_running_ = true;
            m_timeout_ = false;
            go [&]() {
                std::unique_lock<std::mutex> lock(m_mutex_);
                while (m_running_) {
                    m_start_time_ = std::chrono::high_resolution_clock::now();
                    m_cv_.wait_for(
                            lock, std::chrono::milliseconds(m_timeout_millisec_));
                    auto now = std::chrono::high_resolution_clock::now();
                    auto elapsed =
                            std::chrono::duration_cast<std::chrono::milliseconds>(
                                    now - m_start_time_)
                                    .count();
                    if (elapsed >= m_timeout_millisec_ && m_running_) {
                        m_timeout_ = true;
                        m_chan_ << RETURN_TYPE::TIME_OUT;
                    }
                }
            };
        } else {
            m_timeout_millisec_ = millisec;
        }
    }

    bool isTimeout() const { return m_timeout_; }

    bool isRunning() const { return m_running_; }

    void reset(uint64_t millisec) {
        std::unique_lock<std::mutex> lock(m_mutex_);
        m_timeout_millisec_ = millisec;
        m_timeout_ = false;
        if (!m_running_) {
            start(millisec);
        } else {
            m_cv_.notify_one();
        }
    }

    void stop() {
        m_running_ = false;
        m_cv_.notify_one();

    }

private:
    std::atomic<bool> m_running_;
    std::atomic<bool> m_timeout_;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time_;
    uint64_t m_timeout_millisec_;
    std::mutex m_mutex_;
    std::condition_variable m_cv_;
public:
    co_chan<RETURN_TYPE> m_chan_  ;
};