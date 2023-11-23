#pragma  once

#include "public.h"
#include "nocopyable.h"
#include "libgo/coroutine.h"
#include "grpc++/grpc++.h"
#include  "peers.h"
#include "utils/timer.h"
#include "startRpcService.h"

#include "persist/abstractPersist.h"
namespace craft {

    class Raft final : public noncopyable {

    public:
        explicit Raft(int me, AbstractPersist *persister, co_chan<Command> *applyCh) ;

        void launch() ;

        ~Raft();

    private:
        void co_launchRpcSevices();

        void co_startElection();

        void co_applyLogs();

        void co_appendAentries();

    public:

        void persist();
        void loadFromPersist();

        bool is_killed() ;

        void changeToState(STATE toState) ;

        // use to debug
        std::string stringState(STATE state);



    public:

        const int m_me_{};
        int m_current_term_ = 0;
        int m_votedFor_ = -1;
        int m_commitIndex_ = 0;
        int m_lastApplied_ = 0;
        int m_snopShotIndex = 0;
        int m_snopShotTerm = 0;
        bool m_iskilled_ = false;

    public:

        std::vector<LogEntry> m_logs_;
        co_mutex co_mtx_;
        co_chan<Command> *m_applyCh_ = nullptr;
        co_chan<void *> *m_notifyApplyCh_ = nullptr;
        co_chan<RETURN_TYPE> *m_StateChangedCh_ = nullptr;
        co_chan<void *> *m_stopCh_ = nullptr;

        RpcClients *m_peers_ = nullptr;
        AbstractPersist *m_persister_ = nullptr;
        STATE m_state_ = STATE::FOLLOWER;
        std::vector<int> m_nextIndex_;
        std::vector<int> m_matchIndex_;
        Timer *m_electionTimer = nullptr;
        std::vector<Timer *> m_appendEntriesTimers_{nullptr};


    };
}