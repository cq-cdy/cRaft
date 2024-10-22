#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <istream>
#include <iterator>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

// std::unordered_map<std::string, int> get_process_memory_usage(int pid) {
//     std::unordered_map<std::string, int> memory_usage;
//     std::string key;
//     int value;
//     std::string unit; // 通常单位是 kB

//     std::string path = "/proc/" + std::to_string(pid) + "/status";
//     std::ifstream status_file(path);
//     if (!status_file.is_open()) {
//         std::cerr << "Failed to open /proc/" << pid << "/status" <<
//         std::endl; return {};
//     }

//     std::string line;
//     while (std::getline(status_file, line)) {
//         std::istringstream iss(line);
//         if (!(iss >> key >> value >> unit)) {
//             continue; // 如果这一行不匹配，则跳过
//         }

//         // 删除末尾的冒号
//         if (!key.empty() && key.back() == ':') {
//             key.pop_back();
//         }

//         memory_usage[key] = value;
//     }

//     status_file.close();
//     return memory_usage;
// }

// int main() {
//     int pid = getpid();
//     // std::cout << "Enter process ID: ";
//     // std::cin >> pid;

//     auto memory_usage = get_process_memory_usage(pid);

//     for (const auto& [key, value] : memory_usage) {
//         std::cout << key << ": " << value << " kB" << std::endl;
//     }

//     return 0;
// }
// // 整个 主机的：
// // std::unordered_map<std::string, int> get_system_memory_usage() {
// //     std::unordered_map<std::string, int> memory_usage;
// //     std::string key;
// //     int value;
// //     std::string unit;  // 通常单位是 kB

// //     std::ifstream meminfo_file("/proc/meminfo");
// //     if (!meminfo_file.is_open()) {
// //         std::cerr << "Failed to open /proc/meminfo" << std::endl;
// //         return {};
// //     }

// //     std::string line;
// //     while (std::getline(meminfo_file, line)) {
// //         std::istringstream iss(line);
// //         if (!(iss >> key >> value >> unit)) {
// //             continue; // 如果这一行不匹配，则跳过
// //         }

// //         // 删除末尾的冒号
// //         if (!key.empty()) {
// //             key.pop_back();
// //         }

// //         memory_usage[key] = value;
// //     }

// //     meminfo_file.close();
// //     return memory_usage;
// // }

// // int main() {
// //     auto memory_usage = get_system_memory_usage();

// //     for (const auto& [key, value] : memory_usage) {
// //         std::cout << key << ": " << value << " kB" << std::endl;
// //     }

// //     return 0;
// // }
#include <fstream>
#include <sstream>
#include <string>

#include "json.hpp"
class DiskIODataCollector {
   private:
    std::ifstream io_file_;

   public:
    DiskIODataCollector(const std::string& path) {
        io_file_.open(path);
        if (!io_file_.is_open()) {
            throw std::runtime_error("Failed to open file: " + path);
        }
    }

    ~DiskIODataCollector() {
        if (io_file_.is_open()) {
            io_file_.close();
        }
    }

    nlohmann::json get_io_data() {
        nlohmann::json ret{};
        std::string key;
        std::string value;  // 值可能很大，使用字符串处理
        std::string line;
        while (std::getline(io_file_, line)) {
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
};
#include <list>
#include <vector>

#include "context_switches_task.hpp"
#include "cpu_task.hpp"
#include "disk_io_task.hpp"
#include "memory_task.hpp"
#include "network_task.hpp"
#include "threads_fd_task.hpp"
int main() {
    // auto t = MemoryTask();
    // ThreadsAndFdTask tf;
    // tf.get_pid_data();
    // std::cout << tf.get_pid_data().dump(4) << std::endl;
    std::thread([] {
        int i = 0;
        while (1) {
            i++;
        }
    }).detach();
  

    CPUTask t;
    while (1) {
        std::cout << t.get_pid_data().dump(4);
    }
    return 0;
}
