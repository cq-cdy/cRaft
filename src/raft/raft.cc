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

        //sleep(99999999);
    }

    void Raft::changeToState(STATE toState) {
        if (toState == STATE::FOLLOWER) {
            if (this->m_state_ != STATE::FOLLOWER) {
                this->m_votedFor_ = -1;
            }
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
        if(m_StateChangedCh_->empty()){
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

    ResultPackge Raft::submitCommand(Command_ command) {
        int term, index;
        while (co_mtx_.try_lock()) {
            if (m_state_ != STATE::LEADER) {
                co_mtx_.unlock();
                return ResultPackge{-1, -1, false};
            }
            spdlog::info("[{}] receive command:[{}]", m_me_, command.content);
            LogEntry log;
            log.set_term(m_current_term_);
            log.set_command(log.command());
            m_logs_.push_back(log);
            index = getLastLogIndex();
            term = m_current_term_;
            m_matchIndex_[m_me_] = index;
            m_nextIndex_[m_me_] = index + 1;
            m_appendEntriesTimer->reset(0);
        }
        go[this]{
            co_sleep(50);
            m_appendEntriesTimer->reset(HEART_BEAT_INTERVAL);
        };
        co_mtx_.unlock();

        return ResultPackge{index, term, true};
    }
}  // namespace craft
