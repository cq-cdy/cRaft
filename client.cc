#include <grpc++/grpc++.h>

#include <iostream>
#include <memory>
#include <string>

#include "peers.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

int main(int argc, char **argv) {
    std::thread t([] { co_sched.Start(0, 1024); });
    t.detach();
    RequestVoteArgs args;
    RequestVoteReply reply;
    grpc::ChannelArguments channel_args;
    channel_args.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
    channel_args.SetInt(GRPC_ARG_HTTP2_MAX_PINGS_WITHOUT_DATA, 0);
    channel_args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, -1);
    channel_args.SetInt(GRPC_ARG_MAX_RECONNECT_BACKOFF_MS, 100);
    auto channel = grpc::CreateCustomChannel(
        "127.0.0.1:6000", grpc::InsecureChannelCredentials(), channel_args);
    auto s = channel->GetState(true);

    static int i = 0;
    auto stub = RaftRPC::NewStub(channel);
    printf("start;\n");
    go[&] {
        while (true) {
            // 设置单个RPC调用的超时时间
            ClientContext context;
            std::chrono::system_clock::time_point deadline_ =
                std::chrono::system_clock::now() +
                std::chrono::milliseconds(500);
            context.set_deadline(deadline_);
            Status ok = stub->requestVoteRPC(&context, args, &reply);
            grpc_connectivity_state state = channel->GetState(true);
            if (state == GRPC_CHANNEL_CONNECTING) {
                std::cout << "Channel state: " << state << std::endl;
            } else if (state == GRPC_CHANNEL_READY) {
                std::cout << "Channel READY: " << state << std::endl;
            }
            if (ok.ok()) {
                printf("%d ok;\n", i++);
            } else {
                channel.reset();
                channel = grpc::CreateCustomChannel(
                    "127.0.0.1:6000", grpc::InsecureChannelCredentials(),
                    channel_args);
                stub = RaftRPC::NewStub(channel);
                printf("%s\n", ok.error_message().c_str());
            }
            sleep(1);
        }
    };
    sleep(1000000);

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