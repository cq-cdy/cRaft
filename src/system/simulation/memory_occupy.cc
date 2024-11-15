#include <sys/sysinfo.h>

#include <iostream>
#include <new>
#include <vector>

#include "util.hpp"
using namespace std;

long long get_available_memory() {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        std::cerr << "Failed to get system information" << std::endl;
        return -1;
    }
    return static_cast<long long>(info.freeram) * info.mem_unit;
}

void run() {
    vector<char>* memory = new vector<char>();

    long long last_size = 0;
    const long long free_memory_threshold =
        200 * 1024 * 1024;  // 设置200MB为阈值

    while (true) {
        auto avail_mem = get_available_memory();
        if (avail_mem < free_memory_threshold) {
            cout << "Memory low! Freeing all allocated memory." << endl;
            delete memory;
            memory = new vector<char>();
            last_size = 0;
            random_sleep(3,10);
            continue;
        }

        double t =
            ((double)get_random_between(15, 25));  // 每次申请15-25%的可用内存
        double random_percent = t / 100;
        long long alloc_size = (avail_mem * random_percent) + last_size;
        if (alloc_size > avail_mem) {
            cout << "too much Freeing memory" << endl;
            delete memory;
            memory = new vector<char>();
            last_size = 0;
            random_sleep(3,10);
            continue;
        }
        // cout << "Allocating " << alloc_size << " bytes" << endl;

        if (get_random_between(0, 10) >= 3) {  // 60%的概率分配内存
            try {
                memory->resize(alloc_size);
                (*memory)[memory->size() - 1] = 1;
            } catch (const std::bad_alloc&) {
                cout << "Caught bad_alloc! Freeing memory" << endl;
                delete memory;
                memory = new vector<char>();
                last_size = 0;
                continue;
            }
        } else {  // 70%的概率收缩内存
            memory->resize(
                (unsigned int)(memory->size() /
                               get_random_between(20, 25)));  // 随机收缩比例
            last_size = 0;
        }
        last_size = memory->size();
        random_sleep(3, 10);
    }
    delete memory;
}

int main() {
    run();
    return 0;
}
