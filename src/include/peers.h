#pragma  once
#include "grpc++/grpc++.h"
#include "memory"
#include "include/public.h"
#include "rpc/raft.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class RpcClients {
   public:
    static RpcClients* getInstance() {
        static RpcClients peerClients;
        return &peerClients;
    }

    std::vector<std::unique_ptr<RaftRPC::Stub>>& getPeerStubs(){
        return m_peerStubs;
    }
    std::vector<std::shared_ptr<::grpc::Channel>>& getPeerChannels(){
        return m_peerChannels;
    }

    int numPeers() const{
        return m_num_peers_;
    }

   private:
    RpcClients() {

        m_num_peers_ = peersAddr.size();
        for (int i = 0; i < peersAddr.size(); i++) {
            auto channel = grpc::CreateChannel(
                m_peerAddr[i], grpc::InsecureChannelCredentials());
            m_peerStubs.emplace_back(RaftRPC::NewStub(channel));
            m_peerChannels.emplace_back(grpc::CreateChannel(peersAddr[i], grpc::InsecureChannelCredentials()));
        }
    }

   private:
    int m_num_peers_{};
     std::vector<std::unique_ptr<RaftRPC::Stub>> m_peerStubs;
     std::vector<std::shared_ptr<::grpc::Channel>> m_peerChannels;
     std::vector<std::string> m_peerAddr = peersAddr;
    ClientContext m_clientContext;
};