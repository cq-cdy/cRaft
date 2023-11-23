#pragma once
#include "rpc/raft.grpc.pb.h"
#include "grpc++/grpc++.h"
#include "raft.h"
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace craft {
    class Raft;
class RpcServiceImpl : public RaftRPC::Service {
   public:
    RpcServiceImpl() = delete;
    explicit RpcServiceImpl(Raft* raft);
    /* */
    Status requestVoteRPC(::grpc::ServerContext *context,
                          const ::RequestVoteArgs *request,
                          ::RequestVoteReply *response) override;

    Status appendEntries(::grpc::ServerContext *context,
                         const ::AppendEntriesArgs *request,
                         ::AppendEntriesReply *response) override;

    void publishRpcService();

    /*add some rpc functions*/
   private:
    std::string m_addr_;
    Raft* m_rf_{};
};


}  // namespace craft