#include "monitor_task.hpp"

class CPUTask : public MonitorTask {
   public:
    CPUTask() {
        last_tp_ = std::chrono::_V2::system_clock::time_point();
        status_file_ = std::ifstream("/proc/stat");
        if (!status_file_.is_open()) {
            std::cerr << "/proc/stat" << std::endl;
            return;
        }
        pid_status_file_ = std::ifstream("/proc/self/stat");
        if (!pid_status_file_.is_open()) {
            std::cerr << "Failed to open /proc/" << pid_ << "/stat"
                      << std::endl;
            return;
        }
        readSystemCPUData(firstSample);
        firstProcSample = readProcessCPUData();
    }
    ~CPUTask() {
        status_file_.close();
        pid_status_file_.close();
    }
    std::string describe() const noexcept override { return "cpu"; }

    virtual nlohmann::json get_host_data_() override { return {}; }
    virtual nlohmann::json get_pid_data_() override {
        std::lock_guard lock(this->mtx_);
        auto now = std::chrono::high_resolution_clock::now();
        auto distance =
            duration_cast<std::chrono::duration<double>>(now - last_tp_)
                .count();
        if(distance < cal_cpu_interval){
            return last_json_;
        }
        this->pid_status_file_.clear();
        this->status_file_.clear();
        this->pid_status_file_.seekg(0, std::ios::beg);
        this->status_file_.seekg(0, std::ios::beg);
        std::vector<CPUData> secondSample;
        ProcessCPUData secondProcSample = readProcessCPUData();
        readSystemCPUData(secondSample);
        for (size_t i = 0; i < firstSample.size(); i++) {
            auto pair = calculateCPUUsage(firstSample[i], secondSample[i],
                                          firstProcSample, secondProcSample);
            // std::cout << "first:" << pair.first << " sec :" << pair.second
            //           << std::endl;
            last_json_[pair.first] = pair.second;
        }
        firstSample = secondSample;
        firstProcSample = secondProcSample;
        last_tp_ = now;
        
        return last_json_;
    }

   private:
    double cal_cpu_interval  = 0.8;
    nlohmann::json last_json_;
    std::chrono::_V2::system_clock::time_point last_tp_{};
    std::mutex mtx_;
    std::ifstream status_file_;
    std::ifstream pid_status_file_;
    struct CPUData {
        std::string name;
        long user, nice, system, idle, iowait, irq, softirq, steal, guest,
            guest_nice;
        long total;
    };
    struct ProcessCPUData {
        long utime, stime;
    };
    std::vector<CPUData> firstSample;
    ProcessCPUData firstProcSample;
    void readSystemCPUData(std::vector<CPUData>& entries) {
        std::string line;

        while (std::getline(status_file_, line)) {
            if (line.compare(0, 3, "cpu") == 0) {
                std::istringstream ss(line);
                CPUData data;
                ss >> data.name >> data.user >> data.nice >> data.system >>
                    data.idle >> data.iowait >> data.irq >> data.softirq >>
                    data.steal >> data.guest >> data.guest_nice;
                data.total = data.user + data.nice + data.system + data.idle +
                             data.iowait + data.irq + data.softirq +
                             data.steal + data.guest + data.guest_nice;
                entries.push_back(data);
            }
        }
    }
    ProcessCPUData readProcessCPUData() {
        std::string value;
        ProcessCPUData data = {0, 0};
        if (pid_status_file_.is_open()) {
            for (int i = 1; i <= 14; ++i) {
                pid_status_file_ >> value;
                if (i == 14) {  // utime
                    data.utime = std::stol(value);
                }
                if (i == 15) {  // stime
                    data.stime = std::stol(value);
                }
            }
        }
        return data;
    }
    std::pair<std::string, double> calculateCPUUsage(
        const CPUData& prev, const CPUData& curr,
        const ProcessCPUData& prevProc, const ProcessCPUData& currProc) {
        long prevTotal = prev.total;
        long currTotal = curr.total;
        long totald = currTotal - prevTotal;

        long prevProcTotal = prevProc.utime + prevProc.stime;
        long currProcTotal = currProc.utime + currProc.stime;
        long procd = currProcTotal - prevProcTotal;
        double cpuUsage;
        if (totald == 0) {
            cpuUsage = 0;
        }
        cpuUsage = 100.0 * procd / totald;
        return {curr.name, cpuUsage};
    }
};

// // 函数来计算整体 CPU 使用率
// nlohmann::json calculateOverallCPUUsage(const std::vector<CPUData>& prev,
//                                         const std::vector<CPUData>& curr)
//                                         {
//     long totalPrev = 0, totalCurr = 0, totalIdlePrev = 0, totalIdleCurr =
//     0; nlohmann::json ret; for (size_t i = 0; i < prev.size(); ++i) {
//         totalPrev += prev[i].total;
//         totalCurr += curr[i].total;
//         totalIdlePrev += prev[i].idle;
//         totalIdleCurr += curr[i].idle;
//     }

//     long totald = totalCurr - totalPrev;
//     long idled = totalIdleCurr - totalIdlePrev;

//     double overallCPUUsage = 100.0 * (totald - idled) / totald;
//     ret["host_cpu"] = overallCPUUsage;
//     // std::cout << "Overall CPU Usage: " << overallCPUUsage << "%"
//     //           << std::endl;
//     return ret;
// }