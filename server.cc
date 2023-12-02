#include <thread>
#include "craft/raft.h"
#include "regex"

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
        // save data to snapshot file,such as this object to serialize to snapshot file
        /*
            * some IO operation ...
        */
    }

    void addPair(std::pair<std::string,std::string> data){
        /*  some operation*/
    }
private:
    std::map<std::string,std::string> kv_datas_;
};

int main(int argc, char **argv) {

    // start libgo coroutine
    std::thread([] { co_sched.Start(0,1024); }).detach();

    //set log level
    spdlog::set_level(spdlog::level::debug);

    //set snapshot and persist path
    std::string abs_path = "/home/cdy/code/projects/cRaft/.data";

    // set snapshot file name
    std::string snapFileName = "KVServer.snap";
    KVServer kv(abs_path, snapFileName);

    co_chan<ApplyMsg> msgCh(10000);
    craft::Raft raft(&kv, &msgCh);

    raft.launch();
    while(true){
        ApplyMsg msg;
        msgCh >> msg;
        spdlog::info(" get Apply msg [{},{},{}]", msg.commandValid, msg.command.content, msg.commandIndex);
        //raft.saveSnapShot(msg.commandIndex);
    }

    sleep(INT32_MAX);
}
