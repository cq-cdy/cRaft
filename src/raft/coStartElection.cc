#include <libgo/context/context.h>
#include <libgo/coroutine.h>
#include <libgo/defer/defer.h>

#include "peers.h"
#include "public.h"
#include "raft.h"

namespace craft {

    void sendRequestVote(Raft *rf, int serverId,
                         const std::shared_ptr<RequestVoteArgs> &request,
                         const std::shared_ptr<RequestVoteReply> &response);

    void startElection(Raft *rf);

    static bool last_eclction_isEnd = true;

    void Raft::co_startElection() {
        go [this] {
            m_electionTimer->start(getElectionTimeOut(ELECTION_TIMEOUT));
            for (; true;) {
                RETURN_TYPE type_;
                (m_electionTimer->m_chan_) >> type_;
                if (type_ == RETURN_TYPE::TIME_OUT) {
                        printf("time out\n");
                        startElection(this);
                } else {
                    printf("not fund typed\n");
                }
            }
        };
    }

    void startElection(Raft *rf) {
        spdlog::debug("start leader election");
        rf->co_mtx_.lock();
        rf->m_electionTimer->reset(getElectionTimeOut(ELECTION_TIMEOUT));
        // if (rf->m_state_ == STATE::LEADER) {
        //     spdlog::debug(" leader return ");
        //     rf->co_mtx_.unlock();
        //     return;
        // }
        rf->changeToState(STATE::CANDIDATE);
        spdlog::debug("after startElection state ={}", rf->stringState(rf->m_state_));
        rf->co_mtx_.unlock();

        std::shared_ptr<RequestVoteArgs> args(new RequestVoteArgs);
        args->set_candidateid(rf->m_me_);
        args->set_term(rf->m_current_term_);
        spdlog::debug("after set,me({}),args.term = {}",rf->m_me_,rf->m_current_term_);
        int allCount = rf->m_peers_->numPeers(), grantedCount = 1, resCount = 1;
        spdlog::debug("allcount = {}\n", allCount);
        std::shared_ptr<co_chan<bool>> grantedChan(new co_chan<bool>(allCount - 1));
        for (int i = 0; i < allCount; i++) {
            if (i == rf->m_me_) {
                continue;
            }
            go [rf, i, args, grantedChan] {
                std::shared_ptr<RequestVoteReply> reply(new RequestVoteReply);
                sendRequestVote(rf, i, args, reply);
                *grantedChan << reply->votegranted();  //  default false ,if rpc  success true;
                if (reply->term() > rf->m_current_term_) {
                    rf->co_mtx_.lock();
                    rf->m_current_term_ = reply->term();
                    rf->changeToState(STATE::FOLLOWER);
                    rf->m_votedFor_ = -1;
                    rf->m_electionTimer->reset(
                            getElectionTimeOut(ELECTION_TIMEOUT));
                    // todo persist()
                    rf->co_mtx_.unlock();
                }
            };
        }
        bool flag;
        while (resCount != allCount) {
            *grantedChan >> flag;
            resCount++;
            if (flag) {
                grantedCount++;
            }
        }
        rf->co_mtx_.lock();
        co_defer [rf]{
            rf->co_mtx_.unlock();
        };
        if(rf->m_state_  != STATE::CANDIDATE){
            return;
        }
        spdlog::debug("[{}],current_term = {},grantedCount = {}",rf->m_me_,rf->m_current_term_,grantedCount);
        if (grantedCount > (allCount / 2)) {
            spdlog::info(
                    "before try change to leader,count:{}, currentTerm: {}, "
                    "argsTerm: {}",
                    grantedCount, rf->m_current_term_, args->term());
            if (rf->m_state_ == STATE::CANDIDATE &&
                rf->m_current_term_ == args->term()) {
                rf->changeToState(STATE::LEADER);
            }
            // todo
            if (rf->m_state_ == STATE::LEADER) {
                spdlog::debug("{} current state: LEADER", rf->m_me_);
            }
            spdlog::debug("end startElection state ={}", rf->stringState(rf->m_state_));
        } else {
            rf->changeToState(STATE::FOLLOWER);
            spdlog::debug("grant faild,state ={}",rf->stringState(rf->m_state_));
        }
    }

    void sendRequestVote(Raft *rf, int serverId,
                         const std::shared_ptr<RequestVoteArgs> &request,
                         const std::shared_ptr<RequestVoteReply> &response) {
        static std::vector<std::unique_ptr<RaftRPC::Stub>> &stubs =
                rf->m_peers_->getPeerStubs();
        if (serverId < 0 || serverId > peersAddr.size() || serverId == rf->m_me_) {
            spdlog::error("serverId:{} invalid in sendRequestVote!", serverId);
            return;
        }
       
        for (int i = 0; i < 5 && !rf->is_killed(); i++) {
            ClientContext context;
            std::chrono::system_clock::time_point deadline =
                    std::chrono::system_clock::now() +
                    std::chrono::milliseconds(RPC_TIMEOUT);
            context.set_deadline(deadline);
            Status ok =stubs[serverId]->requestVoteRPC(&context, *request, response.get());
            if (!ok.ok()) {
                continue;
            } else {
                spdlog::debug("to {} success call rpc\n", serverId);
                break;
            }
        }
    }

};  // namespace craft