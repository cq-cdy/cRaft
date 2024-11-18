#pragma once
#include "monitor_task.hpp"

struct MemoryTask : public MonitorTask {
    MemoryTask() {
        status_file_ = std::ifstream("/proc/meminfo");
        if (!status_file_.is_open()) {
            std::cerr << "/proc/meminfo" << std::endl;
        }
        pid_status_file_ =
            std::ifstream("/proc/" + std::to_string(pid_) + "/status");
        if (!pid_status_file_.is_open()) {
            std::cerr << "Failed to open /proc/" << pid_ << "/status"
                      << std::endl;
        }
    }
    void reset() {}
    std::string describe() const noexcept override { return "memory"; }

    virtual nlohmann::json get_host_data_() override {
        this->status_file_.clear();
        status_file_.seekg(0, std::ios::beg);
        nlohmann::json ret{};
        std::string key;
        int value;
        std::string unit;  // 通常单位是 kB
        std::string line;
        while (std::getline(status_file_, line)) {
            std::istringstream iss(line);
            if (!(iss >> key >> value >> unit)) {
                continue;  // 如果这一行不匹配，则跳过
            }
            if (!key.empty()) {
                key.pop_back();
            }

            ret[key] = value;
        }
        return ret;
    }
    virtual nlohmann::json get_pid_data_() override {
        this->pid_status_file_.clear();
        this->pid_status_file_.seekg(0, std::ios::beg);
        nlohmann::json ret{};
        std::string key;
        int value;
        std::string unit;  // 通常单位是 kB
        std::string line;
        while (std::getline(pid_status_file_, line)) {
            std::istringstream iss(line);
            if (!(iss >> key >> value >> unit)) {
                continue;  // 如果这一行不匹配，则跳过
            }

            // 删除末尾的冒号
            if (!key.empty() && key.back() == ':') {
                key.pop_back();
            }
            ret[key] = value;
        }
        return ret;
    }

    ~MemoryTask() {
        status_file_.close();
        pid_status_file_.close();
    }

   private:
    std::ifstream status_file_;
    std::ifstream pid_status_file_;
};