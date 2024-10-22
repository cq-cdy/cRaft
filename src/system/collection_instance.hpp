#pragma once
#include <dirent.h>
#include <unistd.h>

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <list>
#include <thread>
namespace fs = std::filesystem;
template <class T>
struct CollectionInstance {
    CollectionInstance(
        std::string path, int max_size = 1 << 16,
        int max_io_thread_count_ = std::thread::hardware_concurrency() << 1,
        std::string data_file_name = "system_runtime.data")
        : path_(path),
          max_size_(max_size),
          max_io_thread_count_(max_io_thread_count_),
          data_file_name_(data_file_name) {
        //

        baseDir_ = path_;
        index_ = numFiles();
        checkOrCreate();
        is_runing_ = true;
    }

    ~CollectionInstance() {
        std::lock(this->io_mtx_, this->push_mtx_);
        std::lock_guard<std::mutex> lk1(io_mtx_, std::adopt_lock);
        std::lock_guard<std::mutex> lk2(push_mtx_, std::adopt_lock);
        is_runing_ = false;
        if (mem_data_ != nullptr) {
            delete mem_data_;
        }
        if (file_) {
            file_->flush();
            file_->close();
        }
    }

    void flush() {
        std::lock(this->io_mtx_, this->push_mtx_);
        std::lock_guard<std::mutex> lk1(io_mtx_, std::adopt_lock);
        std::lock_guard<std::mutex> lk2(push_mtx_, std::adopt_lock);
        if (file_) {
            if(mem_data_ == nullptr){
                return
            }
            for (const auto& data : *mem_data_) {
                *file_ << data << "\n";
            }
            file_->flush();
            file_->close();
        }
    }
    void push(T data) noexcept {
        std::unique_lock lock(this->push_mtx_);

        if (!is_runing_) {
            if (mem_data_ != nullptr) {
                delete mem_data_;
            }
            return;
        }
        auto N = mem_data_->size();
        mem_data_->push_back(data);
        printf("[%d / %d]\n", N, max_size_);
        if (N > max_size_) {
            std::list<T>* disk_io_data = mem_data_;
            mem_data_ = new std::list<T>();
            cv_.wait(lock,
                     [this]() -> bool {  // if to much io_trhead,then wait here;
                         return this->io_thread_count_.load() <
                                this->max_io_thread_count_;
                     });
            std::thread([this, disk_io_data]() {
                // printf("cur io count %d \n",
                // this->io_thread_count_.load());
                this->io_thread_count_.fetch_add(1);
                this->diskio(disk_io_data);
                // auto cur_count = io_thread_count_.load();
                this->cv_.notify_one();
                this->io_thread_count_.fetch_sub(1);
            }).detach();
        }

        // bool expected = false;

        // while (!is_adding_.compare_exchange_strong(expected, true));
        // auto N = mem_data_->size();
        // // to here is_adding_ must be true;
        // if (not is_runing_) {
        //     is_adding_.store(false);
        //     return;
        // }
        // (*mem_data_).push_back(data);
        // printf("[%d / %d]\n", N, max_size);
        // if (N >= max_size) {
        //     std::list<T>* disk_io_data = mem_data_;
        //     mem_data_ = new std::list<T>();
        //     if (is_runing_ == false) {
        //         delete mem_data_;
        //         delete disk_io_data;
        //         is_adding_.store(false);
        //         return;
        //     }
        //     std::thread([this, disk_io_data]() {
        //         this->diskio(disk_io_data);
        //     }).detach();
        // }
        // is_adding_.store(false);
    }

   private:
    void diskio(std::list<T>* diskio_data) {
        std::lock_guard lock(this->io_mtx_);
        if (!is_runing_) {
            if (diskio_data != nullptr) {
                delete diskio_data;
            }
            return;
        }
        if (diskio_data == nullptr) {
            return;
        }

        for (const auto& data : *diskio_data) {
            if (not is_runing_) {
                break;
            }

            *file_ << data << "\n";
        }
        if (diskio_data != nullptr) {
            delete diskio_data;
        }
        checkOrCreate();
    }
    void checkOrCreate() {
        auto fullpath = baseDir_ / (data_file_name_ + std::to_string(index_));
        if (!fs::exists(fullpath)) {
            file_ = new std::ofstream(fullpath, std::ios::out | std::ios::app);
            return;
        }
        auto size = fs::file_size(fullpath);
        printf("filesize : [%ld / %lld]\n", size, max_file_size_);
        if (size > max_file_size_) {
            if (file_ != nullptr) {
                file_->close();
                delete file_;
            }
            fullpath = baseDir_ / (data_file_name_ + std::to_string(++index_));
            file_ = new std::ofstream(fullpath, std::ios::out | std::ios::app);
        } else {
            if (file_ == nullptr) {
                file_ =
                    new std::ofstream(fullpath, std::ios::out | std::ios::app);
            }
        }
    }

    int numFiles() {
        int file_count = 0;

        // 遍历指定目录
        for (const auto& entry : fs::directory_iterator(path_)) {
            if (fs::is_regular_file(entry.status())) {  // 确认是一个文件
                file_count++;
            }
        }
        return file_count;
    }

   private:
    std::ofstream* file_{};
    std::list<T>* mem_data_ = new std::list<T>();

    std::mutex push_mtx_{};
    std::mutex io_mtx_{};
    bool is_runing_{};
    std::atomic<int> io_thread_count_{0};
    std::condition_variable cv_;

    int max_size_{};
    int max_io_thread_count_{};

    std::string data_file_name_{};
    std::string path_;
    fs::path baseDir_;

    unsigned long long max_file_size_ = 2L << 30;  // 2G for single file
    int index_{};
    // std::atomic<bool> is_adding_{false};
};