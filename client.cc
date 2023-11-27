#include <grpc++/grpc++.h>

#include <iostream>
#include <memory>
#include <string>

#include "peers.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

int main(int argc, char** argv) {
    //    std::thread t([] { co_sched.Start(0, 1024); });
    //                   t.detach();
    for (const auto& peer : peersAddr) {
        Command args;
        ::ResultPackge reply;
        args.set_content("add some data");

        auto channel =
                grpc::CreateChannel(peer, grpc::InsecureChannelCredentials());
        auto stub = RaftRPC::NewStub(channel);
        ClientContext context;
        std::chrono::system_clock::time_point deadline_ =
                std::chrono::system_clock::now() +
                std::chrono::milliseconds(RPC_TIMEOUT);
        context.set_deadline(deadline_);
        Status ok = stub->submitCommand(&context, args, &reply);
        if (ok.ok()) {
            spdlog::info("target term = [{}],index = [{}],isleader = [{}]",
                         reply.term(), reply.index(), reply.isleader());
        } else {
            spdlog::error("submit command error");
        }
    }

    return 0;
}
// RequestVoteArgs args;
// RequestVoteReply reply;
// static auto channel = grpc::CreateChannel(
//         "127.0.0.1:7000", grpc::InsecureChannelCredentials());
// static auto stub = RaftRPC::NewStub(channel);
// printf("start;\n");
// ClientContext context;
// while(true) {
//
// Status ok = stub->requestVoteRPC(&context, args, &reply);
//
// if (ok.ok()) {
// printf("ok;\n");
// break;
//
// } else {
// printf("%s\n", ok.error_message().c_str());
// continue;
// }
// }
// grpc C++ 这是我的客户端代码，为什么我客户端如果先y