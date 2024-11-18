#include "craft/startRpcService.h"
#include "grpc++/grpc++.h"
#include "craft/public.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace craft{
    RpcServiceImpl::RpcServiceImpl(Raft *rf) :m_rf_(rf){
        if(rf == nullptr){
            spdlog::critical("raft is null in rpcserice init");
            exit(2);
        }
        m_addr_ = rf->m_clusterAddress_[rf->m_me_];
    }
    void RpcServiceImpl::publishRpcService() {
        std::string server_address(m_rf_->m_clusterAddress_[m_rf_->m_me_]);
        ServerBuilder builder;
        builder.SetSyncServerOption(grpc::ServerBuilder::SyncServerOption::NUM_CQS, 4);
        int a =1;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials(),&a);
        builder.AddChannelArgument(GRPC_ARG_ALLOW_REUSEPORT, 1);
        builder.RegisterService(this);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        spdlog::info("Server[{}] listening on {}", m_rf_->m_me_, server_address);
        server->Wait();
    }
};