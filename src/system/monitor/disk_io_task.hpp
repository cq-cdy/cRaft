#include "monitor_task.hpp"
struct DiskIoTask : public MonitorTask {
    DiskIoTask() {
        pid_status_file_ =
            std::ifstream("/proc/" + std::to_string(pid_) + "/io");
        if (!pid_status_file_.is_open()) {
            std::cerr << "Failed to open /proc/" << pid_ << "/io" << std::endl;
        }
    }
    std::string describe() const noexcept override { return "disk_io"; }

    virtual nlohmann::json get_host_data_() override { return {}; }
    virtual nlohmann::json get_pid_data_() override {
        this->pid_status_file_.clear();
        this->pid_status_file_.seekg(0, std::ios::beg);
        nlohmann::json ret{};
        std::string key;
        std::string value;  // 值可能很大，使用字符串处理
        std::string line;
        while (std::getline(pid_status_file_, line)) {
            std::istringstream iss(line);
            if (!(iss >> key >> value)) {
                continue;  // 如果这一行不匹配，则跳过
            }
            if (!key.empty() && key.back() == ':') {
                key.pop_back();  // 删除末尾的冒号
            }
            ret[key] = std::stoll(value);  // 将字符串转为长整型
        }
        return ret;
    }

    ~DiskIoTask() { pid_status_file_.close(); }

   private:
    std::ifstream pid_status_file_;
};