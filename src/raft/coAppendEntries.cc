#include "libgo/coroutine.h"
#include "public.h"
#include "raft.h"

namespace craft {

    bool sendToAppendEntries(Raft *rf, int serverId,
                             const std::shared_ptr<AppendEntriesArgs> &args,
                             const std::shared_ptr<AppendEntriesReply> &reply);

    void handleAppendSuccess(Raft *rf, int serverId, const std::shared_ptr<AppendEntriesArgs> &args,
                             const std::shared_ptr<AppendEntriesReply> &reply);

    void handleAppendFaild(Raft *rf, int serverId, const std::shared_ptr<AppendEntriesArgs> &args,
                           const std::shared_ptr<AppendEntriesReply> &reply);

    void Raft::co_appendAentries() {
        go [this] {
            for (; !m_iskilled_;) {
                RETURN_TYPE X;
                m_appendEntriesTimer->m_chan_ >> X;
                m_appendEntriesTimer->reset(HEART_BEAT_INTERVAL);
                if (m_state_ == STATE::LEADER) {
                    spdlog::debug("in co_appendAentries state:[{}],my term is [{}]",
                                  stringState(m_state_), m_current_term_);
                    int allCount = m_peers_->numPeers();
                    std::shared_ptr<co_chan<bool>> successChan(
                            new co_chan<bool>(allCount - 1));
                    for (int i = 0; i < this->m_peers_->numPeers(); i++) {
                        if (i == this->m_me_) {
                            continue;
                        }
                        go [this, i, successChan] {
                            co_mtx_.lock();
                            if (m_state_ != STATE::LEADER) {
                                m_appendEntriesTimer->reset(HEART_BEAT_INTERVAL);
                                co_mtx_.unlock();
                                return;
                            }
                            std::shared_ptr<AppendEntriesArgs> args(
                                    new AppendEntriesArgs);
                            args->set_term(m_current_term_);
                            args->set_leaderid(m_me_);
                            args->set_leadercommit(m_commitIndex_);
                            auto argsPack = getAppendLogs(i);
                            args->set_prevlogindex(std::get<0>(argsPack));
                            args->set_prevlogterm(std::get<1>(argsPack));
                            for (const auto &m: std::get<2>(argsPack)) {
                                auto a = args->add_entries();
                                a->set_term(m.term());
                                a->set_command(m.command());
                            }

                            std::shared_ptr<AppendEntriesReply> reply(
                                    new AppendEntriesReply);
                            bool isCallOk = sendToAppendEntries(this, i, args, reply);
                            if(!isCallOk){
                                co_mtx_.unlock();
                                return ;
                            }
                            if (reply->term() > m_current_term_) {
                                m_current_term_ = reply->term();
                                changeToState(STATE::FOLLOWER);
                                m_electionTimer->reset(getElectionTimeOut(ELECTION_TIMEOUT));
                                persist();
                                return;
                            }
                            if (m_state_ == STATE::LEADER) {
                                if (reply->success()) {
                                    spdlog::debug("reply->success");
                                    handleAppendSuccess(this, i, args, reply);
                                } else {
                                    spdlog::debug("reply->faild");
                                    handleAppendFaild(this, i, args, reply);
                                }
                            }
//                            for(int t = 0;t < m_logs_.size();t++){
//                                spdlog::info("[leader]logs:[{}],[{}|{}]",m_logs_[t].DebugString(),t,m_logs_.size());
//                            }
                            co_mtx_.unlock();
                        };
                    }
//                    bool flag;
//                    while (resCount != allCount) {
//                        *successChan >> flag;
//                        resCount++;
//                        if (flag) {
//                            successCount++;
//                        }
//                    }
//                    co_mtx_.lock();
//                    if (m_state_ != STATE::LEADER) {
//                        co_mtx_.unlock();
//                        continue;
//                    }
//                    co_mtx_.unlock();
//                    spdlog::debug("send hb end,all = {} ,success = [{}]", allCount,
//                                  successCount);

                } else {
                    RETURN_TYPE a;
                    *m_StateChangedCh_ >> a;
                    spdlog::debug(
                            "recvive state changed state:[{}],my term is [{}]",
                            stringState(m_state_), m_current_term_);
                }
            }
        };
    }

    bool sendToAppendEntries(Raft *rf, int serverId,
                             const std::shared_ptr<AppendEntriesArgs> &args,
                             const std::shared_ptr<AppendEntriesReply> &reply) {
        static std::vector<std::unique_ptr<RaftRPC::Stub>> &stubs =
                rf->m_peers_->getPeerStubs();
        bool isCallok = false;
        ClientContext context;
        std::chrono::system_clock::time_point deadline =
                std::chrono::system_clock::now() +
                std::chrono::milliseconds(RPC_TIMEOUT);
        context.set_deadline(deadline);
       // spdlog::info("in sendToAppendEntries,args:[{}]", args->DebugString());
        Status ok = stubs[serverId]->appendEntries(&context, *args, reply.get());
       // spdlog::info("in sendToAppendEntries,reply:[{}]", reply->DebugString());
        if (ok.ok()) {
            isCallok = true;
            //spdlog::debug("heat beat ok");
        } else {
            spdlog::error("disconnect to FOLLOWER [{}]:{}", serverId,peersAddr[serverId]);
        }
        return isCallok;
    }

    void handleAppendSuccess(Raft *rf, int serverId, const std::shared_ptr<AppendEntriesArgs> &args,
                             const std::shared_ptr<AppendEntriesReply> &reply) {
        if (reply->nextlogindex() > rf->m_nextIndex_[serverId]) {
            rf->m_nextIndex_[serverId] = reply->nextlogindex();
            rf->m_matchIndex_[serverId] = reply->nextlogindex() - 1;
        }
        if (args->entries_size() > 0 && args->entries(args->entries_size() - 1).term() == rf->m_current_term_) {
            rf->tryCommitLog();
        }
        rf->persist();
    }

    void handleAppendFaild(Raft *rf, int serverId, const std::shared_ptr<AppendEntriesArgs> &args,
                           const std::shared_ptr<AppendEntriesReply> &reply) {
        if (reply->nextlogindex() != 0) {
            if (reply->nextlogindex() > rf->m_snapShotIndex) {
                rf->m_nextIndex_[serverId] = reply->nextlogindex();
                rf->m_appendEntriesTimer->reset(0); //立马重发
                rf->m_appendEntriesTimer->reset(HEART_BEAT_INTERVAL);
            } else {
                //todo 快照
            }
        }
    }

};  // namespace craft