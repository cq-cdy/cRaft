#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

#include "util.hpp"
using namespace std;
void cpu_occupy() {
    unsigned int x = 0;
    for (int i = 0; i < UINT32_MAX; i++) {
        x++;
    }
  
}
// thread_local int y{0};
void run() {
    auto num_cores = std::thread::hardware_concurrency();
    std::cout << "Number of cores: " << num_cores << std::endl;
    std::vector<std::thread> threads;
    while (true) {
        for (int i = 0; i < num_cores; i++) {
            if (get_random_between(0, 10) < 3) {
                threads.push_back(std::thread([]() { cpu_occupy(); }));
            } else {
                threads.push_back(std::thread(
                    []() { sleep(get_random_between(2, 4)); }));
            }
        }
        for (auto& t : threads) {
            t.join();
        }
        threads.clear();
    }
}
int main() {
    run();
    return 0;
}