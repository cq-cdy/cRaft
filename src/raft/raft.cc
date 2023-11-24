#include "raft.h"

#include "public.h"

namespace craft {
Raft::Raft(int me, AbstractPersist *persister, co_chan<Command> *applyCh)
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

    for (int i = 0; i < m_peers_->numPeers(); i++) {
        m_appendEntriesTimers_.emplace_back(new Timer);
    }
    loadFromPersist();
    co_launchRpcSevices();
    // todo read from Persister
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
        if (this->m_state_ != STATE::FOLLOWER) {
            this->m_votedFor_ = -1;
        }
        this->m_state_ = toState;
    } else if (toState == STATE::CANDIDATE) {
        this->m_state_ = toState;
        this->m_current_term_++;
        this->m_votedFor_ = this->m_me_;
    } else if (toState == STATE::LEADER) {
        this->m_state_ = toState;
        this->m_electionTimer->stop();
        spdlog::debug("changeToState,change to [{}]", stringState(m_state_));
        *m_StateChangedCh_ << RETURN_TYPE::STATE_CHANGED;
        return;
        // TODO
    } else {
        this->m_state_ = toState;
        spdlog::error("unkown toState");
    }
    spdlog::debug("end of changeToState,change to [{}]", stringState(m_state_));
    this->m_electionTimer->reset(getElectionTimeOut(ELECTION_TIMEOUT));
    *m_StateChangedCh_ << RETURN_TYPE::STATE_CHANGED;
}

template <typename T>
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
    for (auto &ptr : m_appendEntriesTimers_) {
        deleter(ptr);
    }
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
    m_snopShotIndex = m_persister_->getLastSnapshotIndex();
    m_snopShotTerm = m_persister_->getLastSnapshotTerm();

    auto logEntries = m_persister_->getLogEntries();
    for (int i = 0; i < logEntries.size(); i++) {
        auto log = LogEntry().New();
        log->set_term(logEntries[i].first);
        log->set_command(logEntries[i].second);
        m_logs_.push_back(*log);
    }
    spdlog::info(" load from success.");
    spdlog::info("m_votedFor_ = [{}]", m_votedFor_);
    spdlog::info("m_current_term_ = [{}]", m_current_term_);
    spdlog::info("m_commitIndex_ = [{}]", m_commitIndex_);
    spdlog::info("m_snopShotIndex = [{}]", m_snopShotIndex);
    spdlog::info("m_snopShotTerm = [{}]", m_snopShotTerm);
    for (const auto &i : m_logs_) {
        spdlog::info("logs term = {},command = {}", i.term(), i.command());
    }
}

}  // namespace craft
