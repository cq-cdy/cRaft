#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <thread>

#include "atomic"
#include "craft/raft.h"
#include "regex"
using namespace std::chrono;
static craft::Raft *rft_p = nullptr;
class CoreDumpTask : public MonitorTask {
   public:
    CoreDumpTask(craft::Raft *rft_p) : m_rft_p_(rft_p) { is_tmp = true; }
    std::string describe() const noexcept override { return "coredump"; }

    virtual nlohmann::json get_host_data_() override { return {}; }
    virtual nlohmann::json get_pid_data_() override {
        if (m_rft_p_ == nullptr) {
            return {};
        }
        nlohmann::json data = m_rft_p_->base_json();
        data["action"] = "coredump";
        return data;
    }

   private:
    craft::Raft *m_rft_p_{};
};
// 错误处理函数
void signalHandler(int signum) {
    printf("yes in core dump\n");
    if (rft_p == nullptr) {
        _Exit(signum);
    }
    rft_p->m_monitor_->flush();

    _Exit(signum);
}

class KVServer : public craft::AbstractPersist {
   public:
    KVServer(std::string path, std::string snapFileName)
        : AbstractPersist(std::move(path), std::move(snapFileName)) {}

    void deserialization(const char *filename) override {
        // from snapshot file load data to this object

        /*
         * some IO operation ...
         */
    }

    void serialization() override {
        // save data to snapshot file,such as this object to serialize to
        // snapshot file
        /*
         * some IO operation ...
         */
    }

    void addPair(std::pair<std::string, std::string> data) {
        /*  some operation*/
    }

   private:
    std::map<std::string, std::string> kv_datas_;
};

int main(int argc, char **argv) {
    // start libgo coroutine
    std::thread([] { co_sched.Start(0, 0); }).detach();

    // set log level
    spdlog::set_level(spdlog::level::info);

    // set snapshot and persist path
    std::string abs_path = "/home/cdy1/code/project/cRaft/.data";

    // set snapshot file name
    std::string snapFileName = "KVServer.snap";
    KVServer kv(abs_path, snapFileName);

    co_chan<ApplyMsg> msgCh(100000);
    craft::Raft raft(&kv, &msgCh);
    rft_p = &raft;
    signal(SIGSEGV, signalHandler);  // 捕获段错误
    signal(SIGFPE, signalHandler);   // 捕获浮点异常
    signal(SIGINT, signalHandler);   // 捕获浮点异常
    raft.setLogLevel(spdlog::level::debug);
    raft.launch();
    // auto start = high_resolution_clock::now();
    // std::atomic<long long int> i  =0 ;
    // for (int k = 0; k < 8; k++) {
    //     std::thread([&] {
    //         while (true) {
    //             i++;
    //             ApplyMsg msg;
    //             msgCh >> msg;
    //             auto end = high_resolution_clock::now();
    //             spdlog::info("i = [{}]",i++);
    //             // spdlog::info(" get Apply msg [{},{},{}]",
    //             msg.commandValid,
    //             // msg.command.content, msg.commandIndex);
    //             // raft.saveSnapShot(msg.commandIndex);
    //         }
    //     }).detach();
    // }

    sleep(INT32_MAX);
}
