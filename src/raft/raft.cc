#include "raft.h"

#include "public.h"
#include "filesystem"

namespace craft {
    Raft::Raft(int me, AbstractPersist *persister, co_chan<ApplyMsg> *applyCh)
            : m_persister_(persister), m_me_(me) {
        m_peers_ = RpcClients::getInstance();
        m_nextIndex_.resize(m_peers_->numPeers());
        m_matchIndex_.resize(m_peers_->numPeers());
        m_logs_.emplace_back();

        /*channels*/
        m_applyCh_ = applyCh;
        m_notifyApplyCh_ = new co_chan<void *>(100000);
        m_StateChangedCh_ = new co_chan<RETURN_TYPE>(1);
        m_stopCh_ = new co_chan<void *>(1);
        /* timer */
        m_electionTimer = new Timer();
        m_applyTimer = new Timer();
        m_appendEntriesTimer = new Timer();

        // todo read from Persister

        loadFromPersist();
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
        auto fromState = m_state_;
        if (toState == STATE::FOLLOWER) {
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

        spdlog::info("[{}]:{} from {} change to {},term = [{}]", m_me_, peersAddr[m_me_], stringState(fromState),
                     stringState(toState), m_current_term_);
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

    }


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
        spdlog::info("load from persist success.");
        spdlog::info("m_votedFor_ = [{}]", m_votedFor_);
        spdlog::info("m_current_term_ = [{}]", m_current_term_);
        spdlog::info("m_commitIndex_ = [{}]", m_commitIndex_);
        spdlog::info("m_snopShotIndex = [{}]", m_snapShotIndex);
        spdlog::info("m_snopShotTerm = [{}]", m_snapShotTerm);
        for (const auto &i: m_logs_) {
            spdlog::debug("logs term = {},command = {}", i.term(), i.command());
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
                        spdlog::info("[{}]:{},success commit log index = [{}]", m_me_, peersAddr[m_me_], i);
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

    std::tuple<int, int, std::vector<LogEntry>> Raft::getAppendLogs(int peerId) {
        int nextIndex = m_nextIndex_[peerId];
        int lastLogIndex = getLastLogIndex();
        int lastLogTerm = getLastLogTerm();
        std::vector<LogEntry> logEntries;
        if (nextIndex <= m_snapShotIndex || nextIndex > lastLogIndex) {
            return {lastLogIndex, lastLogTerm, logEntries};
        }
        logEntries.resize(lastLogIndex - nextIndex + 1);
        std::copy(this->m_logs_.begin() + (nextIndex - m_snapShotIndex), this->m_logs_.end(), logEntries.begin());
        int prevLogIndex = nextIndex - 1;
        int prevLogTerm;
        if (prevLogIndex == m_snapShotIndex) {
            prevLogTerm = m_snapShotTerm;
        } else {
            prevLogTerm = this->m_logs_[prevLogIndex - m_snapShotIndex].term();
        }
        return {prevLogIndex, prevLogTerm, logEntries};
    }


    void check(const std::filesystem::path &dir) {
        if (!std::filesystem::exists(dir.parent_path())) {
            std::filesystem::create_directories(dir.parent_path());
            spdlog::info("success create path:[{}]", dir.parent_path().string());
        }
        if (!std::filesystem::exists(dir)) {
            std::ofstream ofs(dir);
            if (ofs) {
                spdlog::info("success create file:[{}]", dir.string());
            } else {
                spdlog::error("error create file:[{}]", dir.string());
                exit(2);
            }
        }
    }

    void writePersist(std::string fileName, int singleValue) {
        std::ofstream stream;
        stream.open(fileName, std::ios::out);

        if (!stream) {
            spdlog::error("can not open file [{}]", fileName);
        } else {
            stream << singleValue;
        }
        stream.close();
    }

    void writePersist(std::string logtermFile, std::string logcommandFile, const std::vector<LogEntry>& logs) {
        {
            // 清空文件
            std::ofstream logtermStream;
            std::ofstream logcommandStream;
            if (!logtermStream) {
                spdlog::error("can not open file [{}]", logtermFile);
                return;
            }
            if (!logcommandStream) {
                spdlog::error("can not open file [{}]", logcommandFile);
                return;
            }
            logtermStream.open(logtermFile, std::ios::out | std::ios::trunc);
            logcommandStream.open(logcommandFile, std::ios::out | std::ios::trunc);
            logtermStream.close();
            logcommandStream.close();
        }
        std::ofstream logtermStream;
        std::ofstream logcommandStream;
        logtermStream.open(logtermFile,std::ios::out);
        logcommandStream.open(logcommandFile,std::ios::out);;
        if(!logtermStream){
            spdlog::error("can not open file [{}]",logtermFile);
        }
        if(!logcommandStream){
            spdlog::error("can not open file [{}]",logcommandFile);
        }
        for(auto i = 1; i < logs.size(); i++){
            logtermStream << logs[i].term() << std::endl;
            logcommandStream << logs[i].command() << std::endl;
        }
        logtermStream.close();
        logcommandStream.close();
    }

   static std::vector<std::string> all_persist_files = {"commitIndex.data", "currentTerm.data", "lastlogindex.data",
                                                  "lastSnapshotIndex.data", "lastSnapshotTerm.data",
                                                  "logentry.command.data", "logentry.term.data", "votefor.data"};
    void Raft::persist() {
        /*
            {"commitIndex.data", "currentTerm.data", "lastlogindex.data",
           "lastSnapshotIndex.data", "lastSnapshotTerm.data",
          "logentry.command.data", "logentry.term.data", "votefor.data"};
         */
        go[this]{
            std::filesystem::path dir = std::filesystem::path(m_persister_->absPersistPath_)/"persist";
            for (const auto &file: all_persist_files) {
                check(dir / file);
            }
            writePersist(dir/"commitIndex.data", m_commitIndex_);
            writePersist(dir/"currentTerm.data", m_current_term_);
            writePersist(dir/"lastlogindex.data", getLastLogIndex());
            writePersist(dir/"lastSnapshotIndex.data", m_snapShotIndex);
            writePersist(dir/"lastSnapshotTerm.data", m_snapShotTerm);
            writePersist(dir/"votefor.data", m_votedFor_);
            writePersist(dir/"logentry.term.data", dir/"logentry.command.data", m_logs_);
        };


    }

}  // namespace craft
