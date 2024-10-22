#include "monitor_task.hpp"
struct ThreadsAndFdTask : public MonitorTask {
    ThreadsAndFdTask() {}
    std::string describe() const noexcept override { return "threads_and_fd"; }

    virtual nlohmann::json get_host_data_() override { return {}; }
    virtual nlohmann::json get_pid_data_() override {
        int thread_count = 0;
        nlohmann::json ret{};
        auto path = "/proc/" + std::to_string(pid_) + "/task";
        // 计算线程数
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_directory()) {
                ++thread_count;
            }
        }
        ret["threads"] = thread_count;
        path = "/proc/" + std::to_string(pid_) + "/fd";
        int fd_count = 0;
        ret["fd_count"] = fd_count;
        return ret;
    }

    ~ThreadsAndFdTask() { pid_status_file_.close(); }

   private:
    std::ifstream pid_status_file_;
};