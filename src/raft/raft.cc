#include "raft.h"

#include "public.h"

namespace craft {
    Raft::Raft(int me, AbstractPersist *persister, co_chan<Command_> *applyCh)
            : m_persister_(persister), m_me_(me) {
        m_peers_ = RpcClients::getInstance();
        m_nextIndex_.resize(m_peers_->numPeers());
        m_matchIndex_.resize(m_peers_->numPeers());

        /*channels*/
        m_applyCh_ = applyCh;
        m_notifyApplyCh_ = new co_chan<void *>(100);
        m_StateChangedCh_ = new co_chan<RETURN_TYPE>(1);
        m_stopCh_ = new co_chan<void *>(1);
        /* timer */
        m_electionTimer = new Timer();
        m_applyTimer = new Timer();
        m_appendEntriesTimer = new Timer();

//        for (int i = 0; i < m_peers_->numPeers(); i++) {
//            m_appendEntriesTimers_.emplace_back(new Timer);
//        }
        // todo read from Persister

        //loadFromPersist();
        co_launchRpcSevices();
    }

    void Raft::launch() {
        /* main:logic:start with coroutines */
        co_appendAentries();
        co_startElection();
        co_applyLogs();

        sleep(99999999);
    }

    void Raft::changeToState(STATE toState) {
        if (toState == STATE::FOLLOWER) {
//            if (this->m_state_ != STATE::FOLLOWER) {
//                this->m_votedFor_ = -1;
//            }
            m_appendEntriesTimer->stop();
        } else if (toState == STATE::CANDIDATE) {
            this->m_current_term_++;
            this->m_votedFor_ = this->m_me_;
            m_electionTimer->reset(getElectionTimeOut(ELECTION_TIMEOUT));
            m_appendEntriesTimer->stop();
        } else if (toState == STATE::LEADER) {
            int lastLogIndex = getLastLogIndex();
            for (int i = 0; i < m_peers_->numPeers(); i++) {
                m_nextIndex_[i] = lastLogIndex + 1;
                m_matchIndex_[i] = lastLogIndex;
            }
            m_electionTimer->stop();
            m_appendEntriesTimer->reset(HEART_BEAT_INTERVAL);
        } else {
            spdlog::critical("change to unkown toState");
        }
        m_state_ = toState;
        spdlog::debug("changeToState,change to [{}]", stringState(m_state_));
        if (m_StateChangedCh_->empty()) {
            *m_StateChangedCh_ << RETURN_TYPE::STATE_CHANGED;
        }
    }

    template<typename T>
    void deleter(T *&ptr) {
        if (ptr != nullptr) {
            delete ptr;
            ptr = nullptr;
        }
    }

    bool Raft::is_killed() const { return m_iskilled_; }

    std::string Raft::stringState(STATE state) {
        std::string a;
        do {
            if (state == STATE::LEADER) {
                a = "LEADER";
            } else if (state == STATE::CANDIDATE) {
                a = "CANDIDATE";
            } else if (state == STATE::FOLLOWER) {
                a = "FOLLOWER";
            } else {
                a = "UNKNOWN";
            }
        } while (false);
        return a;
    }

    Raft::~Raft() {
        deleter(m_applyCh_);
        deleter(m_stopCh_);
        deleter(m_notifyApplyCh_);
        deleter(m_persister_);
        deleter(m_electionTimer);
        deleter(m_appendEntriesTimer);
//        for (auto &ptr: m_appendEntriesTimers_) {
//            deleter(ptr);
//        }
    }

    void Raft::persist() {}

    void Raft::loadFromPersist() {
        spdlog::info("start load from persist.");
        if (m_persister_ == nullptr) {
            spdlog::error("Persist Object not found");
            return;
        }
        m_votedFor_ = m_persister_->getVotedFor();
        // m_lastApplied_ = m_persister_->getLastLogIndex();
        m_current_term_ = m_persister_->getCurrentTerm();
        m_commitIndex_ = m_persister_->getCommitIndex();
        m_snapShotIndex = m_persister_->getLastSnapshotIndex();
        m_snapShotTerm = m_persister_->getLastSnapshotTerm();

        auto logEntries = m_persister_->getLogEntries();
        for (auto &logEntrie: logEntries) {
            auto log = LogEntry().New();
            log->set_term(logEntrie.first);
            log->set_command(logEntrie.second);
            m_logs_.push_back(*log);
        }
        spdlog::info(" load from success.");
        spdlog::info("m_votedFor_ = [{}]", m_votedFor_);
        spdlog::info("m_current_term_ = [{}]", m_current_term_);
        spdlog::info("m_commitIndex_ = [{}]", m_commitIndex_);
        spdlog::info("m_snopShotIndex = [{}]", m_snapShotIndex);
        spdlog::info("m_snopShotTerm = [{}]", m_snapShotTerm);
        for (const auto &i: m_logs_) {
            spdlog::info("logs term = {},command = {}", i.term(), i.command());
        }
    }

    int Raft::getLastLogTerm() const {
        return m_logs_[m_logs_.size() - 1].term();
    }

    int Raft::getLastLogIndex() const {
        return m_snapShotIndex + m_logs_.size() - 1;
    }

    bool Raft::isOutOfArgsAppendEntries(const ::AppendEntriesArgs *args) const {
        int argsLastLogIndex = args->prevlogindex() + args->entries_size();
        int lastLogIndex = getLastLogIndex();
        int lastLogTerm = getLastLogTerm();

        if (lastLogTerm == args->term() && argsLastLogIndex < lastLogIndex) {
            return true;
        }
        return false;
    }

    int Raft::getStoreIndexByLogIndex(int logIndex) {
        int storeIndex = logIndex - m_snapShotIndex;
        if (storeIndex < 0) {
            spdlog::error("getStoreIndexByLogIndex error,logIndex = [{}],m_snapShotIndex = [{}]", logIndex,
                          m_snapShotIndex);
            return -1;
        }
        return storeIndex;
    }

    void Raft::tryCommitLog() {
        int lastLogIndex = getLastLogIndex();
        bool hasCommit = false;
        for (int i = m_commitIndex_ + 1; i <= lastLogIndex; i++) {
            int count = 0;
            for (int j = 0; j < m_peers_->numPeers(); j++) {
                if (m_matchIndex_[j] >= i) {
                    count++;
                    if (count > m_peers_->numPeers() / 2) {
                        m_commitIndex_ = i;
                        hasCommit = true;
                        spdlog::info("commit log index = [{}]", i);
                        break;
                    }
                }
            }
            if (m_commitIndex_ != i) {
                break;
            }
        }
        if (hasCommit) {
            *m_notifyApplyCh_ << (void *) 1;
        }
    }

    Raft::ArgsPack Raft::getAppendLogs(int peerId) {
        int nextIndex = m_nextIndex_[peerId];
        int lastLogIndex = getLastLogIndex();
        int lastLogTerm = getLastLogTerm();

        if (nextIndex <= m_snapShotIndex || nextIndex > lastLogIndex) {
//            spdlog::error("getAppendLogs error,nextIndex = [{}],m_snapShotIndex = [{}],lastLogIndex=[{}]", nextIndex,
//                          m_snapShotIndex,lastLogIndex);
            return ArgsPack{lastLogIndex, lastLogTerm, {}};
        }
        ArgsPack argsPack;
        //这里一定要进行深拷贝，不然会和Snapshot()发生数据上的冲突
        //logEntries = rf.logs[nextIndex-rf.lastSnapshotIndex:]
        //-
        int size = lastLogIndex - nextIndex + 1;
        std::vector<LogEntry> entries(size);
        int start = nextIndex - m_snapShotIndex;
        int end = std::min(static_cast<int>(m_logs_.size()), start + size);
        for (int i = start, j = 0; i < end; i++, j++) {
            entries[j] = m_logs_[i];
        }
        argsPack.logEntries = entries;
        //-

        argsPack.prevLogIndex = nextIndex - 1;
        if (argsPack.prevLogIndex == m_snapShotIndex) {
            argsPack.prevLogTerm = m_snapShotTerm;
        } else {
            argsPack.prevLogTerm = m_logs_[argsPack.prevLogIndex - m_snapShotIndex].term();
        }
        return argsPack;

    }
}  // namespace craft
