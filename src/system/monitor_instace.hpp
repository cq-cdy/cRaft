#pragma once
#include <chrono>

#include "./collection_instance.hpp"
#include "./monitor/context_switches_task.hpp"
#include "./monitor/cpu_task.hpp"
#include "./monitor/disk_io_task.hpp"
#include "./monitor/memory_task.hpp"
#include "./monitor/network_task.hpp"
#include "./monitor/threads_fd_task.hpp"

template <class DATA_T>
class MonitorInstance {
   public:
    MonitorInstance(
        std::string path, int max_size = 1 << 16,
        int max_io_thread_count_ = std::thread::hardware_concurrency() << 1,
        std::string data_file_name = "system_runtime.data") {
        collection_instance_ptr_ = new CollectionInstance<DATA_T>(
            path, max_size, max_io_thread_count_, data_file_name);
    }
    inline static std::string timestamp() noexcept {
        auto now = std::chrono::high_resolution_clock::now();

        auto timestamp_microseconds =
            std::chrono::duration_cast<std::chrono::microseconds>(
                now.time_since_epoch())
                .count();
        return std::to_string(timestamp_microseconds);
    }
    void flush_json(nlohmann::json data) { this->push(std::move(data)); }
    nlohmann::json get_system_base_state_json() {
        nlohmann::json data{};
        static std::vector<MonitorTask*> tasks{
            &disk_task_,      &cpu_task_,     &mem_task_,
            &thread_fd_task_, &network_task_, &ctx_swtich_task_};
        for (auto task_ptr_ : tasks) {
            if (task_ptr_ == nullptr) {
                continue;
            }
            auto role = task_ptr_->describe();
            auto __data = task_ptr_->get_pid_data();
            if (__data.size() > 1) {
                data[role]["process"] = __data;
            }
            __data = task_ptr_->get_host_data();
            if (__data.size() > 1) {
                data[role]["host"] = __data;
            }
        }
        return data;
    }
    void flush() {
        if (collection_instance_ptr_) {
            collection_instance_ptr_->flush();
        }
    }
    void record_ctx_swtich() { this->system_push(&(this->ctx_swtich_task_)); }
    void record_network() { this->system_push(&(this->network_task_)); }
    void record_thread_fd() { this->system_push(&(this->thread_fd_task_)); }
    void record_cpu() { this->system_push(&(this->cpu_task_)); }
    void record_disk() { this->system_push(&(this->disk_task_)); }
    void record_mem() { this->system_push(&(this->mem_task_)); }

    // for user special task
    template <class Handler>
        requires std::is_base_of<MonitorTask, Handler>::value
    void record_custom(Handler handler) {
        this->system_push(&handler);
    }

    template <class Handler = std::nullptr_t>
        requires std::is_convertible<Handler, MonitorTask>::value ||
                 std::is_same<Handler, std::nullptr_t>::value ||
                 std::is_same<Handler, MonitorTask>::value
    void record_batch(std::vector<Handler*> handlers) {
        nlohmann::json data{};
        data["type"] = "batch";
        std::vector<MonitorTask*> tasks{&disk_task_,    &cpu_task_,
                                        &mem_task_,     &thread_fd_task_,
                                        &network_task_, &ctx_swtich_task_};
        if constexpr (!std::is_same<Handler, std::nullptr_t>::value) {
            for (auto& handler : handlers) {
                tasks.push_back(dynamic_cast<MonitorTask*>(handler));
            }
        }

        for (auto task_ptr_ : tasks) {
            if (task_ptr_ == nullptr) {
                continue;
            }
            auto role = task_ptr_->describe();
            auto __data = task_ptr_->get_pid_data();
            if (__data.size() > 1) {
                data[role]["process"] = __data;
            }
            __data = task_ptr_->get_host_data();
            if (__data.size() > 1) {
                data[role]["host"] = __data;
            }
            if (task_ptr_->is_tmp) {
                delete task_ptr_;
            }
        }
        this->push(data);
    }
    ~MonitorInstance() { delete collection_instance_ptr_; }

   private:
    void system_push(MonitorTask* sys_task) {
        auto data = sys_task->get_pid_data();
        data["role"] = sys_task->describe();
        if (data.size() > 0) {
            data["type"] = "process";
            this->push(data);
        }
        data = sys_task->get_host_data();
        if (data.size() > 0) {
            data["type"] = "host";
            this->push(data);
        }
    }
    void push(nlohmann::json data) {
        if (data.size() > 1) {
            this->collection_instance_ptr_->push(std::move(data.dump(4)));
        }
    }

   private:
    DiskIoTask disk_task_;
    CPUTask cpu_task_;
    MemoryTask mem_task_;
    ThreadsAndFdTask thread_fd_task_;
    NetWorkTask network_task_;
    ContextSwitchTask ctx_swtich_task_;
    CollectionInstance<DATA_T>* collection_instance_ptr_;
};
