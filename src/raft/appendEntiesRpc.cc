#include "libgo/defer/defer.h"
#include "public.h"
#include "startRpcService.h"

namespace craft {


    Status RpcServiceImpl::appendEntries(::grpc::ServerContext *context,
                                         const ::AppendEntriesArgs *request,
                                         ::AppendEntriesReply *response) {
        m_rf_->co_mtx_.lock();
        response->set_term(m_rf_->m_current_term_);
        response->set_success(false);
        do{
            if (request->term() > m_rf_->m_current_term_) {
                m_rf_->m_current_term_  = request->term();
                m_rf_->changeToState(STATE::FOLLOWER);
                m_rf_->m_electionTimer->reset(getElectionTimeOut(ELECTION_TIMEOUT));
            }else{
                if(m_rf_->m_state_ !=STATE::FOLLOWER){
                    spdlog::error(" not follower ,but reveiced heart beat, my:[{}]",m_rf_->stringState(m_rf_->m_state_));
                    m_rf_->changeToState(STATE::FOLLOWER);
                }else{
                    response->set_success(false);
                    m_rf_->m_electionTimer->reset(getElectionTimeOut(ELECTION_TIMEOUT));
                    spdlog::debug("RECEIVE heart beat my state is [{}],my term[{}]", m_rf_->stringState(m_rf_->m_state_),m_rf_->m_current_term_);
                }
            }
        }while(false);
        m_rf_->co_mtx_.unlock();

        return Status::OK;
    }

}