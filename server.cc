
#include <unistd.h>

#include <cstdlib>
#include <thread>

#include "./src/craft/raft.h"
#include "atomic"
#include "craft/high_availability.h"
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

void coredump_handle(int n) {
    if (rft_p == nullptr) {
        return;
    }
    auto js = rft_p->base_json();
    rft_p->m_monitor_->record_batch<MonitorTask>({new CoreDumpTask(rft_p)});
    rft_p->m_monitor_->flush();
    printf(" coredump flushed !!!\n");
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

void run() {
    // start libgo coroutine
    std::thread([] { co_sched.Start(0, 0); }).detach();

    // set log level
    spdlog::set_level(spdlog::level::debug);

    // set snapshot and persist path
    const char *homePath = std::getenv("RAFT_HOME_PATH");
    if (homePath == nullptr) {
        spdlog::error(
            "RAFT_HOME_PATH is not set. please run 'source setenv.sh' first");
        exit(1);
    }
    std::string abs_path = std::string(homePath) + "/.data";

    // set snapshot file name
    std::string snapFileName = "KVServer.snap";
    KVServer kv(abs_path, snapFileName);

    co_chan<ApplyMsg> msgCh(100000);
    rft_p = new craft::Raft(&kv, &msgCh);
    // struct sigaction sa;
    // sa.sa_handler = &coredump_handle;
    // sigemptyset(&sa.sa_mask);
    // sa.sa_flags = SA_RESTART | SA_NOCLDSTOP | SA_RESETHAND|SA_NODEFER;
    // if (sigaction(SIGCHLD, &sa, 0) == -1) {
    //     perror("sigaction failed");
    //     exit(1);
    // }
    signal(SIGSEGV, coredump_handle);  // 捕获段错误
    signal(SIGFPE, coredump_handle);   // 捕获浮点异常
    signal(SIGINT, coredump_handle);   // 捕获浮点异常
    signal(SIGABRT, coredump_handle);
    rft_p->setLogLevel(spdlog::level::info);
    rft_p->launch();
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
int main(int argc, char **argv) {
     run();
//    HighAvai *high_avai = HighAvai::getInstance(run, 2);
//   high_avai->setRestartCount(5 /* defalut count = 5；*/);
//   high_avai->start(argc, argv);
}
