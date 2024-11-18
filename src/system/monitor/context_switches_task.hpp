#include "monitor_task.hpp"
struct ContextSwitchTask : public MonitorTask {
    ContextSwitchTask() {
        pid_status_file_ =
            std::ifstream("/proc/" + std::to_string(pid_) + "/status");
        if (!pid_status_file_.is_open()) {
            std::cerr << "Failed to open /proc/" << pid_ << "/status"
                      << std::endl;
        }
    }
    std::string describe() const noexcept override {
        return "context_switches";
    }

    virtual nlohmann::json get_host_data_() override { return {}; }
    virtual nlohmann::json get_pid_data_() override {
        this->pid_status_file_.clear();
        this->pid_status_file_.seekg(0, std::ios::beg);
        nlohmann::json ret;
        std::string line;
        while (std::getline(pid_status_file_, line)) {
            std::istringstream iss(line);
            std::string key;
            std::getline(iss, key, ':');
            iss >> std::ws;  // 跳过冒号后的所有空格
            std::string value;
            std::getline(iss, value);  // 读取剩余部分作为value

            // std::cout << "Key: '" << key << "' Value: '" << value << "'"
            //           << std::endl;

            if (key == "voluntary_ctxt_switches") {
                ret["nvcsw"] = std::stoi(value);
            } else if (key == "nonvoluntary_ctxt_switches") {
                ret["nivcsw"] = std::stoi(value);
            } else if (key == "minflt") {
                ret["minflt"] = std::stoi(value);
            } else if (key == "majflt") {
                ret["majflt"] = std::stoi(value);
            }
        }
        return ret;
    }

    ~ContextSwitchTask() { pid_status_file_.close(); }

   private:
    std::ifstream pid_status_file_;
};