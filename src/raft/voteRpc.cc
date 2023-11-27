#include "libgo/defer/defer.h"
#include "public.h"
#include "startRpcService.h"

namespace craft {
    bool checkLog(Raft *rf, const ::RequestVoteArgs *request);

    Status RpcServiceImpl::requestVoteRPC(::grpc::ServerContext *context,
                                          const ::RequestVoteArgs *request,
                                          ::RequestVoteReply *response) {

        m_rf_->co_mtx_.lock();
        spdlog::debug("in voteRpc state:[{}],my term is [{}],peerterm is [{}]", m_rf_->stringState(m_rf_->m_state_),
                      m_rf_->m_current_term_, request->term());
        response->set_votegranted(false);
        response->set_term(m_rf_->m_current_term_);
        auto peerTerm = request->term();
        do {
            if (peerTerm < m_rf_->m_current_term_) {
                break;
            } else if (peerTerm == m_rf_->m_current_term_) {
                if (m_rf_->m_state_ == STATE::LEADER) {
                    break;
                }
                //保证一个任期内 只投给一个人
                if (m_rf_->m_votedFor_ == -1) {
                    m_rf_->m_current_term_ = peerTerm;
                    m_rf_->changeToState(STATE::FOLLOWER);
                    m_rf_->m_votedFor_ = request->candidateid();
                    response->set_votegranted(true);
                    m_rf_->m_electionTimer->reset(getElectionTimeOut(ELECTION_TIMEOUT));
                    m_rf_->persist();
                    spdlog::info("[{}] to [{}]投票成功-",m_rf_->m_me_,request->candidateid());
                } else {
                    break;
                }
            } else { // peerTerm > m_rf_->m_current_term_
                m_rf_->m_current_term_ = peerTerm;
                if (!checkLog(m_rf_, request) ) {
                    m_rf_->changeToState(STATE::FOLLOWER);
                    m_rf_->m_votedFor_ = request->candidateid();
                    response->set_votegranted(true);
                    m_rf_->m_electionTimer->reset(getElectionTimeOut(ELECTION_TIMEOUT));
                    m_rf_->persist();
                    spdlog::info("[{}] to [{}]投票成功--",m_rf_->m_me_,request->candidateid());
                }else{
                    spdlog::error("[{}] to [{}]投票失败，日志检查不通过",m_rf_->m_me_,request->candidateid());
                }
            }

        } while (false);
        m_rf_->co_mtx_.unlock();
        return Status::OK;
    }

    bool checkLog(Raft *rf, const ::RequestVoteArgs *request) {
        spdlog::debug("lastlogindex:[{}],lastlogterm:[{}]", rf->getLastLogIndex(), rf->getLastLogTerm());
        spdlog::debug("request lastlogindex:[{}],lastlogterm:[{}]", request->lastlogindex(), request->lastlogterm());
        int lastLogIndex = rf->getLastLogIndex();
        int lastLogTerm = rf->getLastLogTerm();
        return (lastLogTerm > request->lastlogterm() ||
                (lastLogTerm == request->lastlogterm() && lastLogIndex > request->lastlogindex()));

    }


};  // namespace craft