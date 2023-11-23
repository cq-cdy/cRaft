#include "libgo/defer/defer.h"
#include "public.h"
#include "startRpcService.h"

namespace craft {


    Status RpcServiceImpl::appendEntries(::grpc::ServerContext *context,
                                         const ::AppendEntriesArgs *request,
                                         ::AppendEntriesReply *response) {
        m_rf_->co_mtx_.lock();
        co_defer [this] {
            m_rf_->co_mtx_.unlock();
        };
        if (request->term() > m_rf_->m_current_term_) {
            m_rf_->changeToState(STATE::FOLLOWER);
        }
        spdlog::info("received term[{}],command_size[{}]",request->term(),request->entries_size());
        spdlog::info("logs:{},{}",request->entries(0).command(),request->entries(1).command());
        //spdlog::debug("RECEIVE heart beat my state is [{}]", m_rf_->stringState(m_rf_->m_state_));
        m_rf_->m_electionTimer->reset(getElectionTimeOut(ELECTION_TIMEOUT));
        return Status::OK;
    }

}