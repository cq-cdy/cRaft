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
