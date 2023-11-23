#include "grpc++/grpc++.h"
#include "raft.h"
#include "rpc/raft.grpc.pb.h"
#include "startRpcService.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
namespace craft {

    void Raft::co_launchRpcSevices() {

        go [this]() {
            RpcServiceImpl rpcService(this);
            rpcService.publishRpcService();
        };

    }
}// namespace craft