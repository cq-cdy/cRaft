
#include "raft.h"
#include <thread>
#include <utility>
#include "regex"
#include "persist/abstractPersist.h"

std::string getExecutablePath() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    return std::string(result, (count > 0) ? count : 0);
}

class KVServer : public craft::AbstractPersist {
public:

    KVServer(std::string path):AbstractPersist(std::move(path)){
    }
    void deserialization(const char *filename) override {

    }

    void serialization() override {

    }
};

int main(int argc, char **argv) {
    std::string exec_path = getExecutablePath();
    std::cout << "执行文件路径: " << exec_path << std::endl;
    // 正则表达式提取序号
    std::regex pattern("/rafts/([0-9]+)/");
    std::smatch match;
    std::string abs_path;
    int me = 0;
    if (std::regex_search(exec_path, match, pattern) && match.size() > 1) {
        std::cout << "提取的序号是: " << match.str(1) << std::endl;
        me = std::atoi(match.str(1).c_str());
        char s[1024];
        sprintf(s,"/home/cdy/code/projects/rafts/%d/.data",me);
        abs_path = std::string (s);
    } else {
        std::cerr << "在路径中未找到序号。" << std::endl;
        abs_path = "/home/cdy/code/projects/cRaft/.data";
    }
    spdlog::set_level(spdlog::level::info); // 设置日志级别为 debug
    spdlog::info("get persist path [{}]",abs_path);
    std::thread t([] { co_sched.Start(8, 1024); });
    t.detach();
    KVServer kv(abs_path);
    co_chan<ApplyMsg> msgCh(100);
    craft::Raft raft(me, &kv, &msgCh);
    raft.launch();
    while(true){
        ApplyMsg msg;
        msgCh >> msg;
        spdlog::info("!!!!!! get Apply msg [{},{},{}]",msg.commandValid,msg.command.content,msg.commandIndex);
    }

//    }
    return 0;
}
