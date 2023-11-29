#include <grpc++/grpc++.h>

#include <chrono>

#include "craft/peers.h"
#include "craft/utils/commonUtil.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using namespace std::chrono;
auto start = steady_clock::now();

std::vector<std::string> peersAddr;
int main(int argc, char **argv) {
    ConfigReader reader((std::string(getenv("HOME")) + "/craft/craft.conf"));
    auto configMap = reader.getMap();
   auto range = configMap.equal_range("servers");
    if (range.first != range.second) {
        std::vector<std::string> servers;
        for (auto it = range.first; it != range.second; it++) {
            peersAddr.push_back(it->second);
        }
    }
    int count = 0;
    static int numThreads = 4;
    // stress test
    for (int i = 0;; i %= peersAddr.size()) {
        auto channel = grpc::CreateChannel(peersAddr[i],
                                           grpc::InsecureChannelCredentials());
        auto stub = RaftRPC::NewStub(channel);
        do {
            Command args;
            ::ResultPackge reply;
            args.set_content("add some data");

            ClientContext context;
            std::chrono::system_clock::time_point deadline_ =
                std::chrono::system_clock::now() +
                std::chrono::milliseconds(200);
            context.set_deadline(deadline_);
            Status ok = stub->submitCommand(&context, args, &reply);
            if (ok.ok()) {
                spdlog::info(
                    "count [i]={},target term = [{}],index = [{}],isleader = "
                    "[{}]",
                    count++, reply.term(), reply.index(), reply.isleader());
                if (!reply.isleader()) {
                    break;
                }
            } else {
                spdlog::error("submit command error");
                break;
            }

        } while (true);
        i++;
    }
    return 0;
}
