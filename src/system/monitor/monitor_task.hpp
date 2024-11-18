#pragma once
#include "monitor_pub.hpp"
struct MonitorTask {
    void set_pid(pid_t t_pid) noexcept { this->pid_ = t_pid; }
    pid_t pid_ = getpid();

    virtual std::string describe() const noexcept = 0;

    nlohmann::json get_host_data() {
        std::lock_guard __host_lock(this->file_seek_host_mtx_);
        return this->get_host_data_();
    }
    nlohmann::json get_pid_data() {
        std::lock_guard __pid_lock(this->file_seek_pid_mtx_);
        return this->get_pid_data_();
    }
    virtual ~MonitorTask() {
        file_seek_pid_mtx_.unlock();
        file_seek_host_mtx_.unlock();
    }
        bool is_tmp = false;

   private:
    std::mutex file_seek_host_mtx_{};
    std::mutex file_seek_pid_mtx_{};

   protected:
    virtual nlohmann::json get_host_data_() = 0;
    virtual nlohmann::json get_pid_data_() = 0;

};