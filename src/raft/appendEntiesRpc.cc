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
        do {
            if (request->term() > m_rf_->m_current_term_) {
                m_rf_->m_current_term_ = request->term();
                m_rf_->changeToState(STATE::FOLLOWER);
                m_rf_->m_electionTimer->reset(getElectionTimeOut(ELECTION_TIMEOUT));
            } else {
                if (m_rf_->m_state_ != STATE::FOLLOWER) {
                    spdlog::error(" not follower ,but reveiced heart beat, my:[{}]",
                                  m_rf_->stringState(m_rf_->m_state_));
                    m_rf_->changeToState(STATE::FOLLOWER);
                }
                m_rf_->m_electionTimer->reset(getElectionTimeOut(ELECTION_TIMEOUT));
                spdlog::debug("RECEIVE heart beat my state is [{}],my term[{}]",
                              m_rf_->stringState(m_rf_->m_state_), m_rf_->m_current_term_);

                //do main logic
                int lastLogIndex = m_rf_->getLastLogIndex();
                if (request->prevlogindex() < m_rf_->m_snapShotIndex) {
                    //1.要插入的前一个index小于快照index，几乎不会发生
                    response->set_success(false);
                    response->set_nextlogindex(m_rf_->m_snapShotIndex + 1);
                } else if (request->prevlogindex() > lastLogIndex) {
                    //2. 要插入的前一个index大于最后一个log的index，说明中间还有log
                    response->set_success(false);
                    response->set_nextlogindex(lastLogIndex + 1);
                } else if (request->prevlogindex() == m_rf_->m_snapShotIndex) {
                    //3. 要插入的前一个index刚好等于快照的index，说明可以全覆盖，但要判断是否是全覆盖
                    if (m_rf_->isOutOfArgsAppendEntries(request)) {
                        response->set_success(false);
                        response->set_nextlogindex(0);
                    } else {
                        response->set_success(true);
                        m_rf_->m_logs_.resize(1);
                        for (const auto &log: request->entries()) {
                            m_rf_->m_logs_.push_back(log);
                        }
                        response->set_nextlogindex(m_rf_->getLastLogIndex() + 1);
                    }
                } else if (request->prevlogterm() ==
                           m_rf_->m_logs_[m_rf_->getStoreIndexByLogIndex(request->prevlogindex())].term()) {
                    //4. 中间的情况：索引处的两个term相同
                    if (m_rf_->isOutOfArgsAppendEntries(request)) {
                        response->set_success(false);
                        response->set_nextlogindex(0);
                    } else {
                        response->set_success(true);
                        int storeIndex = m_rf_->getStoreIndexByLogIndex(request->prevlogindex());
                        m_rf_->m_logs_.resize(storeIndex + 1);
                        for (const auto &log: request->entries()) {
                            m_rf_->m_logs_.push_back(log);
                        }
                        response->set_nextlogindex(m_rf_->getLastLogIndex() + 1);
                    }
                } else {
                    //5. 中间的情况：索引处的两个term不相同,跳过一个term
                    int term = m_rf_->m_logs_[m_rf_->getStoreIndexByLogIndex(request->prevlogindex())].term();
                    int index = request->prevlogindex();
                    for (; (index > m_rf_->m_commitIndex_)
                           && (index > m_rf_->m_snapShotIndex)
                           && (m_rf_->m_logs_[m_rf_->getStoreIndexByLogIndex(index)].term() == term);) {
                        index--;
                    }
                    response->set_success(false);
                    response->set_nextlogindex(index + 1);
                }
                if (response->success()) {
                    spdlog::info("current commit:[{}],try to commit:[{}]", m_rf_->m_commitIndex_,
                                 request->leadercommit());
                    if (m_rf_->m_commitIndex_ < request->leadercommit()) {
                        m_rf_->m_commitIndex_ = request->leadercommit();
                        *m_rf_->m_notifyApplyCh_ << (void *) 1;
                        spdlog::info("commit log to:[{}]", m_rf_->m_commitIndex_);
                    }
                }
                m_rf_->persist();
                spdlog::info("[{}] role: [{}], get appendentries finish,args = [{}],reply = [{}]", m_rf_->m_me_,
                             m_rf_->stringState(m_rf_->m_state_),
                             request->DebugString(), response->DebugString());
                for (auto i = 0; i < m_rf_->m_logs_.size(); i++) {
                    spdlog::info("{} [{}] log[{}/{}] : {}", m_rf_->m_me_, m_rf_->stringState(m_rf_->m_state_), i,
                                 m_rf_->m_logs_.size(), m_rf_->m_logs_[i].DebugString());
                }
//                int lastLogIndex = m_rf_->getLastLogIndex();
//                if (request->prevlogindex() < m_rf_->m_snapShotIndex) {
//                    //1.要插入的前一个index小于快照index，几乎不会发生
//                    response->set_nextlogindex(m_rf_->m_snapShotIndex + 1);
//                } else if (request->prevlogindex() > lastLogIndex) {
//                    //2. 要插入的前一个index大于最后一个log的index，说明中间还有log
//                    response->set_nextlogindex(lastLogIndex + 1);
//                } else if (request->prevlogindex() == m_rf_->m_snapShotIndex) {
//                    //3. 要插入的前一个index刚好等于快照的index，说明可以全覆盖，但要判断是否是全覆盖
//                    if (m_rf_->isOutOfArgsAppendEntries(request)) {
//                        response->set_nextlogindex(0);
//                    } else {
//                        response->set_success(true);
//                        m_rf_->m_logs_.resize(1);
//                        for (const auto &log: request->entries()) {
//                            m_rf_->m_logs_.push_back(log);
//                        }
//                        response->set_nextlogindex(m_rf_->getLastLogIndex() + 1);
//                    }
//                } else if (request->prevlogterm() ==
//                           m_rf_->m_logs_[m_rf_->getStoreIndexByLogIndex(request->prevlogindex())].term()) {
//                    //4. 中间的情况：索引处的两个term相同
//                    if (m_rf_->isOutOfArgsAppendEntries(request)) {
//                        response->set_nextlogindex(0);
//                    } else {
//                        response->set_success(true);
//                        int storeIndex = m_rf_->getStoreIndexByLogIndex(request->prevlogindex());
//                        m_rf_->m_logs_.resize(storeIndex + 1);
//                        for (const auto &log: request->entries()) {
//                            m_rf_->m_logs_.push_back(log);
//                        }
//                        response->set_nextlogindex(m_rf_->getLastLogIndex() + 1);
//                    }
//                } else {
//                    //5. 中间的情况：索引处的两个term不相同,跳过一个term
//                    int term = m_rf_->m_logs_[m_rf_->getStoreIndexByLogIndex(request->prevlogindex())].term();
//                    int index = request->prevlogindex();
//                    for (; (index > m_rf_->m_commitIndex_)
//                           && (index > m_rf_->m_snapShotIndex)
//                           && (m_rf_->m_logs_[m_rf_->getStoreIndexByLogIndex(index)].term() == term);) {
//                            index--;
//                    }
//                    response->set_nextlogindex(index + 1);
//                }
//
//                if(response->success()){
//                    spdlog::info("current commit:[{}],try to commit:[{}]",m_rf_->m_commitIndex_,request->leadercommit());
//                    if(request->leadercommit() > m_rf_->m_commitIndex_){
//                        m_rf_->m_commitIndex_ = request->leadercommit();
//                        *m_rf_->m_notifyApplyCh_ << (void*)1;
//                        spdlog::info("commit log to:[{}]",m_rf_->m_commitIndex_);
//                    }
//                    m_rf_->persist();
//
//                }else{
//
//                }
//                for(int i = 0;i < m_rf_->m_logs_.size();i++){
//                    spdlog::info("[FOLLWOER]log:[{}],[{}|{}]",m_rf_->m_logs_[i].DebugString(),i,m_rf_->m_logs_.size());
//                }
            }
        } while (false);
        m_rf_->co_mtx_.unlock();

        return Status::OK;
    }


}





//Status RpcServiceImpl::appendEntries(::grpc::ServerContext *context,
//                                         const ::AppendEntriesArgs *request,
//                                         ::AppendEntriesReply *response) {
//        m_rf_->co_mtx_.lock();
//        spdlog::debug("RECEIVE heart beat my state is [{}],my term[{}]",
//                      m_rf_->stringState(m_rf_->m_state_), m_rf_->m_current_term_);
//        response->set_term(m_rf_->m_current_term_);
//        if (request->term() < m_rf_->m_current_term_) {
//            response->set_success(false);
//            m_rf_->co_mtx_.unlock();
//            return Status::OK;
//        }
//
//        m_rf_->m_current_term_ = request->term();
//        m_rf_->changeToState(STATE::FOLLOWER);
//        m_rf_->m_electionTimer->reset(getElectionTimeOut(ELECTION_TIMEOUT));
//
//        if (m_rf_->m_state_ != STATE::FOLLOWER) {
//            spdlog::error(" not follower ,but reveiced heart beat, my:[{}]",
//                          m_rf_->stringState(m_rf_->m_state_));
//            m_rf_->changeToState(STATE::FOLLOWER);
//        }
//        int lastLogIndex = m_rf_->getLastLogIndex();
//        if (request->prevlogindex() < m_rf_->m_snapShotIndex) {
//            //1.要插入的前一个index小于快照index，几乎不会发生
//            response->set_success(false);
//            response->set_nextlogindex(m_rf_->m_snapShotIndex + 1);
//        } else if (request->prevlogindex() > lastLogIndex) {
//            //2. 要插入的前一个index大于最后一个log的index，说明中间还有log
//            response->set_success(false);
//            response->set_nextlogindex(lastLogIndex + 1);
//        } else if (request->prevlogindex() == m_rf_->m_snapShotIndex) {
//            //3. 要插入的前一个index刚好等于快照的index，说明可以全覆盖，但要判断是否是全覆盖
//            if (m_rf_->isOutOfArgsAppendEntries(request)) {
//                response->set_success(false);
//                response->set_nextlogindex(0);
//            } else {
//                response->set_success(true);
//                m_rf_->m_logs_.resize(1);
//                for (const auto &log: request->entries()) {
//                    m_rf_->m_logs_.push_back(log);
//                }
//                response->set_nextlogindex(m_rf_->getLastLogIndex() + 1);
//            }
//        } else if (request->prevlogterm() ==
//                   m_rf_->m_logs_[m_rf_->getStoreIndexByLogIndex(request->prevlogindex())].term()) {
//            //4. 中间的情况：索引处的两个term相同
//            if (m_rf_->isOutOfArgsAppendEntries(request)) {
//                response->set_success(false);
//                response->set_nextlogindex(0);
//            } else {
//                response->set_success(true);
//                int storeIndex = m_rf_->getStoreIndexByLogIndex(request->prevlogindex());
//                m_rf_->m_logs_.resize(storeIndex + 1);
//                for (const auto &log: request->entries()) {
//                    m_rf_->m_logs_.push_back(log);
//                }
//                response->set_nextlogindex(m_rf_->getLastLogIndex() + 1);
//            }
//        } else {
//            //5. 中间的情况：索引处的两个term不相同,跳过一个term
//            int term = m_rf_->m_logs_[m_rf_->getStoreIndexByLogIndex(request->prevlogindex())].term();
//            int index = request->prevlogindex();
//            for (; (index > m_rf_->m_commitIndex_)
//                   && (index > m_rf_->m_snapShotIndex)
//                   && (m_rf_->m_logs_[m_rf_->getStoreIndexByLogIndex(index)].term() == term);) {
//                index--;
//            }
//            response->set_success(false);
//            response->set_nextlogindex(index + 1);
//        }
//        if (response->success()) {
//            spdlog::info("current commit:[{}],try to commit:[{}]", m_rf_->m_commitIndex_, request->leadercommit());
//            if (m_rf_->m_commitIndex_ < request->leadercommit()) {
//                m_rf_->m_commitIndex_ = request->leadercommit();
//                *m_rf_->m_notifyApplyCh_ << (void *) 1;
//                spdlog::info("commit log to:[{}]", m_rf_->m_commitIndex_);
//            }
//        }
//        m_rf_->persist();
//        spdlog::info("[{}] role: [{}], get appendentries finish,args = [{}],reply = [{}]", m_rf_->m_me_, m_rf_->stringState(m_rf_->m_state_),
//                     request->DebugString(), response->DebugString());
//        for(auto i = 0;i<m_rf_->m_logs_.size();i++){
//            spdlog::info("{} [{}] log[{}/{}] : {}",m_rf_->m_me_,m_rf_->stringState(m_rf_->m_state_),i,m_rf_->m_logs_.size(),m_rf_->m_logs_[i].DebugString());
//        }
//        m_rf_->co_mtx_.unlock();
//        return Status::OK;
//    }