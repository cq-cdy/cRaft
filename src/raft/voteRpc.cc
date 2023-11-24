#include "libgo/defer/defer.h"
#include "public.h"
#include "startRpcService.h"

namespace craft {

    Status RpcServiceImpl::requestVoteRPC(::grpc::ServerContext *context,
                                          const ::RequestVoteArgs *request,
                                          ::RequestVoteReply *response) {

        m_rf_->co_mtx_.lock();
        co_defer [this] {
            m_rf_->co_mtx_.unlock();
        };
        spdlog::debug("in voteRpc state:[{}],my term is [{}],peerterm is [{}]",m_rf_->stringState(m_rf_->m_state_),m_rf_->m_current_term_,request->term());
        response->set_votegranted(false);
        response->set_term(m_rf_->m_current_term_);
        auto peerTerm = request->term();
        do {
            if (peerTerm < m_rf_->m_current_term_) {
                break;
            } else if (peerTerm == m_rf_->m_current_term_) {
                //保证一个任期内 只投给一个人
                if (m_rf_->m_votedFor_ == -1) {
                    m_rf_->m_current_term_ = peerTerm;
                    m_rf_->changeToState(STATE::FOLLOWER);
                    m_rf_->m_votedFor_ = request->candidateid();
                    response->set_votegranted(true);
                }else{
                    break;
                }
            } else { // peerTerm > m_rf_->m_current_term_
                m_rf_->m_current_term_ = peerTerm;
                m_rf_->changeToState(STATE::FOLLOWER);
                m_rf_->m_votedFor_ = request->candidateid();
                response->set_votegranted(true);
            }

        } while (false);
        if(m_rf_->m_state_==STATE::FOLLOWER){
            m_rf_->m_electionTimer->reset(getElectionTimeOut(ELECTION_TIMEOUT));
        }
        return Status::OK;
    }


};  // namespace craft