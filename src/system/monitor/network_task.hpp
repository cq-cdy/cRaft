#pragma once
#include "monitor_task.hpp"

struct NetWorkTask : public MonitorTask {
    NetWorkTask() {
        pid_status_file_ =
            std::ifstream("/proc/" + std::to_string(pid_) + "/net/dev");
        if (!pid_status_file_.is_open()) {
            std::cerr << "Failed to open /proc/" << pid_ << "/net/dev"
                      << std::endl;
        }
    }
    std::string describe() const noexcept override { return "network"; }

    virtual nlohmann::json get_host_data_() override { return {}; }
    virtual nlohmann::json get_pid_data_() override {
        this->pid_status_file_.clear();
        this->pid_status_file_.seekg(0, std::ios::beg);
        nlohmann::json ret{};
        std::string line;
        while (getline(pid_status_file_, line)) {
            if (line.find("Inter") != std::string::npos ||
                line.find("face") != std::string::npos) {
                continue;
            }

            std::istringstream iss(line);
            std::string interface;
            unsigned long rx_bytes, tx_bytes, dummy;

            iss >> interface >> rx_bytes >> dummy >> dummy >> dummy >> dummy >>
                dummy >> dummy >> dummy >> tx_bytes;

            // std::cout << "Interface: " << interface << ", RX Bytes: "
            //           << rx_bytes << ", TX Bytes: " << tx_bytes << std::endl;
            ret[interface]["RX Bytes"] = rx_bytes;
            ret[interface]["TX Bytes"] = tx_bytes;
        }
        return ret;
    }

    ~NetWorkTask() { pid_status_file_.close(); }

   private:
    std::ifstream pid_status_file_;
};