#include "craft/public.h"
#include "craft/startRpcService.h"
#include "system/json.hpp"
namespace craft {

Status RpcServiceImpl::appendEntries(::grpc::ServerContext *context,
                                     const ::AppendEntriesArgs *request,
                                     ::AppendEntriesReply *response) {
    m_rf_->co_mtx_.lock();
    response->set_term(m_rf_->m_current_term_);
    response->set_success(false);
    nlohmann::json state_js{};
    nlohmann::json action_js{};

    //
    action_js["role"] = "action";
    action_js["action"] = "receive_append_entries";
    action_js["timestamp"] = request->timestamp();
    action_js["me"] = m_rf_->m_me_;
    action_js["peer"] = request->leaderid();
    action_js["host_term"] = m_rf_->m_current_term_;
    action_js["peer_term"] = request->term();
    action_js["prevlogindex"] = request->prevlogindex();
    action_js["prevlogterm"] = request->prevlogterm();
    action_js["leadercommit"] = request->leadercommit();
    m_rf_->m_monitor_->flush_json(state_js);
    int receiveLogBytes = 0;
    do {
        if (request->term() > m_rf_->m_current_term_) {
            m_rf_->m_current_term_ = request->term();
            m_rf_->changeToState(STATE::FOLLOWER);
            m_rf_->m_electionTimer->reset(
                getElectionTimeOut(m_rf_->m_leaderEelectionTimeOut_));
        } else {
            if (m_rf_->m_state_ != STATE::FOLLOWER) {
                spdlog::error("not FOLLWER ,but reveiced heart beat, id[{}]",
                              m_rf_->stringState(m_rf_->m_state_));
                m_rf_->changeToState(STATE::FOLLOWER);
            }
            m_rf_->m_electionTimer->reset(
                getElectionTimeOut(m_rf_->m_leaderEelectionTimeOut_));
            spdlog::debug("id[{}]:[{}] [{}] receive HeartBeat from id[{}]:{}",
                          m_rf_->m_me_, m_rf_->m_clusterAddress_[m_rf_->m_me_],
                          m_rf_->stringState(m_rf_->m_state_),
                          request->leaderid(),
                          m_rf_->m_clusterAddress_[request->leaderid()]);
            // do main logic
            int lastLogIndex = m_rf_->getLastLogIndex();
            if (request->prevlogindex() < m_rf_->m_snapShotIndex) {
                // 1
                response->set_success(false);
                response->set_nextlogindex(m_rf_->m_snapShotIndex + 1);
            } else if (request->prevlogindex() > lastLogIndex) {
                // 2
                response->set_success(false);
                response->set_nextlogindex(lastLogIndex + 1);
            } else if (request->prevlogindex() == m_rf_->m_snapShotIndex) {
                // 3
                if (m_rf_->isOutOfArgsAppendEntries(request)) {
                    response->set_success(false);
                    response->set_nextlogindex(0);
                } else {
                    response->set_success(true);
                    m_rf_->m_logs_.resize(1);
                    for (const auto &log : request->entries()) {
                        m_rf_->m_logs_.push_back(log);
                    }
                    response->set_nextlogindex(m_rf_->getLastLogIndex() + 1);
                }
            } else if (request->prevlogterm() ==
                       m_rf_
                           ->m_logs_[m_rf_->getStoreIndexByLogIndex(
                               request->prevlogindex())]
                           .term()) {
                // 4
                if (m_rf_->isOutOfArgsAppendEntries(request)) {
                    response->set_success(false);
                    response->set_nextlogindex(0);
                } else {
                    response->set_success(true);
                    int storeIndex =
                        m_rf_->getStoreIndexByLogIndex(request->prevlogindex());
                    m_rf_->m_logs_.resize(storeIndex + 1);
                    for (const auto &log : request->entries()) {
                        m_rf_->m_logs_.push_back(log);
                        receiveLogBytes+=log.command().size();
                    }
                    response->set_nextlogindex(m_rf_->getLastLogIndex() + 1);
                }
            } else {
                // 5
                int term = m_rf_
                               ->m_logs_[m_rf_->getStoreIndexByLogIndex(
                                   request->prevlogindex())]
                               .term();
                int index = request->prevlogindex();
                for (; (index > m_rf_->m_commitIndex_) &&
                       (index > m_rf_->m_snapShotIndex) &&
                       (m_rf_->m_logs_[m_rf_->getStoreIndexByLogIndex(index)]
                            .term() == term);) {
                    index--;
                }
                response->set_success(false);
                response->set_nextlogindex(index + 1);
            }
            if (response->success()) {
                spdlog::debug("current commit:[{}],try to commit:[{}]",
                              m_rf_->m_commitIndex_, request->leadercommit());
                if (m_rf_->m_commitIndex_ < request->leadercommit()) {
                    m_rf_->m_commitIndex_ = request->leadercommit();
                    *m_rf_->m_notifyApplyCh_ << (void *)1;
                    spdlog::info("id[{}]:{}[{}] commit log[{}]", m_rf_->m_me_,
                                 m_rf_->m_clusterAddress_[m_rf_->m_me_],
                                 m_rf_->stringState(m_rf_->m_state_),
                                 m_rf_->m_commitIndex_);
                }
            }
            m_rf_->persist();
        }
    } while (false);
    m_rf_->co_mtx_.unlock();
    action_js["success"] = response->success();
    action_js["nextlogindex"] = response->nextlogindex();
    action_js["receiveLogBytes"] = receiveLogBytes;

    state_js["role"] = "state";
    state_js["timestamp"] =m_rf_->m_monitor_->timestamp(); // here not use request->timestamp() best
    state_js["system_state"] = m_rf_->m_monitor_->get_system_base_state_json();
    state_js["raft_state"] = m_rf_->base_json();
    m_rf_->m_monitor_->flush_json(state_js);
    m_rf_->m_monitor_->flush_json(action_js);
    return Status::OK;
}

}  // namespace craft
