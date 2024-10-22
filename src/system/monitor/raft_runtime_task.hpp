#pragma once
#include <memory>

#include "monitor_task.hpp"
struct RaftRunTimeTask : public MonitorTask {
    RaftRunTimeTask(nlohmann::json data) : data_(std::move(data)) {
        is_tmp = true;
    }
    std::string describe() const noexcept override { return "raft_custom"; }
    virtual nlohmann::json get_host_data_() override { return {}; }
    virtual nlohmann::json get_pid_data_() override { return data_; }

   private:
    nlohmann::json data_;
};