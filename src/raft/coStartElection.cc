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

    void Raft::co_startElection() {
        go [this] {
            m_state_ = STATE::FOLLOWER;
            m_electionTimer->start(getElectionTimeOut(ELECTION_TIMEOUT));
            for (; !m_iskilled_;) {
                RETURN_TYPE type_;
                spdlog::debug("wait time out\n");
                (m_electionTimer->m_chan_) >> type_;
                if (type_ == RETURN_TYPE::TIME_OUT) {
                    spdlog::debug("time out\n");
                    startElection(this);
                } else {
                    spdlog::debug("not fund typed\n");
                }
            }
        };
    }

    void startElection(Raft *rf) {
        spdlog::debug("start leader election");
        rf->co_mtx_.lock();
        rf->changeToState(STATE::CANDIDATE);
        rf->co_mtx_.unlock();
        spdlog::debug("after startElection state ={}", rf->stringState(rf->m_state_));
        int allCount = rf->m_peers_->numPeers(), grantedCount = 1, resCount = 1;
        spdlog::debug("allcount = {}\n", allCount);
        std::shared_ptr<co_chan<bool>> grantedChan(new co_chan<bool>(allCount - 1));
        for (int i = 0; i < allCount; i++) {
            if (i == rf->m_me_) {
                continue;
            }
            go [rf, i, grantedChan] {
                std::shared_ptr<RequestVoteArgs> args(new RequestVoteArgs);
                args->set_candidateid(rf->m_me_);
                args->set_term(rf->m_current_term_);
                spdlog::debug("before sendRequestVote,me({}),args.term = {}", rf->m_me_, rf->m_current_term_);
                std::shared_ptr<RequestVoteReply> reply(new RequestVoteReply);
                sendRequestVote(rf, i, args, reply);
                bool is_voted = reply->votegranted();
                *grantedChan << is_voted ; //  default false ,if rpc  success true;
                spdlog::debug("after push chanvote ");
                if(is_voted){
                    rf->m_electionTimer->reset(
                            getElectionTimeOut(ELECTION_TIMEOUT));
                }
                rf->co_mtx_.lock();
                if (reply->term() > rf->m_current_term_) {
                    rf->m_current_term_ = reply->term();
                    rf->changeToState(STATE::FOLLOWER);
                    rf->m_votedFor_ = -1;
                    // todo persist()
                }
                rf->co_mtx_.unlock();
            };
        }

        spdlog::debug("wait tongji vote");
        bool flag;
        while (resCount != allCount) {
            *grantedChan >> flag;
            resCount++;
            if (flag) {
                grantedCount++;
            }
        }
        spdlog::debug("[{}],current_term = {},grantedCount = {}", rf->m_me_, rf->m_current_term_, grantedCount);
        rf->co_mtx_.lock();
        if (rf->m_state_ == STATE::CANDIDATE) {
            if (grantedCount > (allCount / 2)) {
                spdlog::info(
                        "before try change to leader,count:{}, currentTerm: {}, "
                        "argsTerm: {}",
                        grantedCount, rf->m_current_term_, rf->m_current_term_);
                spdlog::debug("end startElection state ={}", rf->stringState(rf->m_state_));
                rf->changeToState(STATE::LEADER);
            } else {
                rf->changeToState(STATE::FOLLOWER);
                spdlog::debug("grant faild,state ={}", rf->stringState(rf->m_state_));
            }
        }
        rf->co_mtx_.unlock();



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
        spdlog::debug("in sendRequestVote");
        for (int i = 0; i < 5 && !rf->m_iskilled_; i++) {
            ClientContext context;
            std::chrono::system_clock::time_point deadline =
                    std::chrono::system_clock::now() +
                    std::chrono::milliseconds(RPC_TIMEOUT);
            context.set_deadline(deadline);
            Status ok = stubs[serverId]->requestVoteRPC(&context, *request, response.get());
            if (!ok.ok()) {
                spdlog::debug("[{}],call voteRPC error\n", serverId);
                continue;
            } else {
                spdlog::debug("to {} success call rpc\n", serverId);
                break;
            }
        }
    }

};  // namespace craft