#include "libgo/defer/defer.h"
#include "craft/public.h"
#include "craft/startRpcService.h"

namespace craft {
    bool checkLog(Raft *rf, const ::RequestVoteArgs *request);

    Status RpcServiceImpl::requestVoteRPC(::grpc::ServerContext *context,
                                          const ::RequestVoteArgs *request,
                                          ::RequestVoteReply *response) {

        m_rf_->co_mtx_.lock();
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
                //Guarantee to only vote for one person during a term
                if (m_rf_->m_votedFor_ == -1) {
                    m_rf_->m_current_term_ = peerTerm;
                    m_rf_->changeToState(STATE::FOLLOWER);
                    m_rf_->m_votedFor_ = request->candidateid();
                    response->set_votegranted(true);
                    m_rf_->m_electionTimer->reset(getElectionTimeOut(m_rf_->m_leaderEelectionTimeOut_));
                    m_rf_->persist();
                    spdlog::debug("[{}]:{} to [{}]:{} vote success", m_rf_->m_me_, m_rf_->m_clusterAddress_[m_rf_->m_me_],
                                  request->candidateid(),m_rf_->m_clusterAddress_[request->candidateid()]);
                } else {
                    break;
                }
            } else { // peerTerm > m_rf_->m_current_term_
                m_rf_->m_current_term_ = peerTerm;
                if (!checkLog(m_rf_, request)) {
                    m_rf_->changeToState(STATE::FOLLOWER);
                    m_rf_->m_votedFor_ = request->candidateid();
                    response->set_votegranted(true);
                    m_rf_->m_electionTimer->reset(getElectionTimeOut(m_rf_->m_leaderEelectionTimeOut_));
                    m_rf_->persist();
                    spdlog::debug("[{}]:{} to [{}]:{} vote success", m_rf_->m_me_, m_rf_->m_clusterAddress_[m_rf_->m_me_],
                                  request->candidateid(),m_rf_->m_clusterAddress_[request->candidateid()]);
                } else {
                    spdlog::error("[{}] to [{}] vote faild,Log check failed", m_rf_->m_me_, m_rf_->m_clusterAddress_[m_rf_->m_me_],
                                  request->candidateid(),m_rf_->m_clusterAddress_[request->candidateid()]);
                }
            }

        } while (false);
        m_rf_->co_mtx_.unlock();
        return Status::OK;
    }

    bool checkLog(Raft *rf, const ::RequestVoteArgs *request) {
        int lastLogIndex = rf->getLastLogIndex();
        int lastLogTerm = rf->getLastLogTerm();
        return (lastLogTerm > request->lastlogterm() ||
                (lastLogTerm == request->lastlogterm() && lastLogIndex > request->lastlogindex()));

    }


};  // namespace craft