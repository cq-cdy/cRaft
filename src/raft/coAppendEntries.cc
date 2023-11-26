#include "libgo/coroutine.h"
#include "public.h"
#include "raft.h"

namespace craft {

    void sendToAppendEntries(Raft *rf, int serverId,
                             const std::shared_ptr<AppendEntriesArgs> &args,
                             const std::shared_ptr<AppendEntriesReply> &reply);

    void Raft::co_appendAentries() {
        go [this] {
            for (; !m_iskilled_;) {
                co_sleep(HEART_BEAT_TIMEOUT);
                STATE state = m_state_;
                if (state == STATE::LEADER) {
                    spdlog::debug("in co_appendAentries state:[{}],my term is [{}]",
                                  stringState(state), m_current_term_);
                    int allCount = m_peers_->numPeers(), successCount = 1,
                            resCount = 1;
                    std::shared_ptr<co_chan<bool>> successChan(
                            new co_chan<bool>(allCount - 1));
                    for (int i = 0; i < this->m_peers_->numPeers(); i++) {
                        if (i == this->m_me_) {
                            continue;
                        }
                        std::shared_ptr<AppendEntriesArgs> args(
                                new AppendEntriesArgs);
                        args->set_term(m_current_term_);
                        std::shared_ptr<AppendEntriesReply> reply(
                                new AppendEntriesReply);
                        go [this, i, args, reply, successChan] {
                            sendToAppendEntries(this, i, args, reply);
                            *successChan << reply->success();
                            if(reply->success()){
                                //m_electionTimer->reset(getElectionTimeOut(ELECTION_TIMEOUT));
                            }
                            co_mtx_.lock();
                            if (reply->term() > m_current_term_ ) {
                                m_current_term_ = reply->term();
                                changeToState(STATE::FOLLOWER);
                                m_electionTimer->reset(getElectionTimeOut(ELECTION_TIMEOUT));
                            }
                            co_mtx_.unlock();

//                            if (m_state_ == STATE::LEADER) {
//                                if (reply->term() > m_current_term_ &&
//                                    m_state_ != STATE::FOLLOWER) {
//                                    m_current_term_ = reply->term();
//                                    changeToState(STATE::FOLLOWER);
//                                }
//                            }
                        };
                    }

                    bool flag;
                    while (resCount != allCount) {
                        *successChan >> flag;
                        resCount++;
                        if (flag) {
                            successCount++;
                        }
                    }
                    if (m_state_ != STATE::LEADER) {
                        co_mtx_.unlock();
                        continue;
                    }
                    spdlog::debug("send hb end,all = {} ,success = [{}]", allCount,
                                  successCount);

                } else {
                    RETURN_TYPE a;
                    *m_StateChangedCh_ >> a;
                    spdlog::debug(
                            "recvive state changed state:[{}],my term is [{}]",
                            stringState(state), m_current_term_);
                }
            }
        };
    }

    void sendToAppendEntries(Raft *rf, int serverId,
                             const std::shared_ptr<AppendEntriesArgs> &args,
                             const std::shared_ptr<AppendEntriesReply> &reply) {
        static std::vector<std::unique_ptr<RaftRPC::Stub>> &stubs =
                rf->m_peers_->getPeerStubs();
        ClientContext context;
        std::chrono::system_clock::time_point deadline =
                std::chrono::system_clock::now() +
                std::chrono::milliseconds(RPC_TIMEOUT);
        context.set_deadline(deadline);
        Status ok = stubs[serverId]->appendEntries(&context, *args, reply.get());
        if (ok.ok()) {
            spdlog::debug("heat beat ok");
        } else {
            spdlog::error("heat beat err");
        }
    }
};  // namespace craft