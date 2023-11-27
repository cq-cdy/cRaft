#include <libgo/context/context.h>
#include <libgo/coroutine.h>
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
            m_electionTimer->reset(getElectionTimeOut(ELECTION_TIMEOUT));
            for (; !m_iskilled_;) {
                RETURN_TYPE type_;
                (m_electionTimer->m_chan_) >> type_;
                if (type_ == RETURN_TYPE::TIME_OUT) {
                    startElection(this);
                } else {
                    spdlog::debug("not found type\n");
                }
            }
        };
    }

    void startElection(Raft *rf) {
        spdlog::debug("start leader election");
        rf->co_mtx_.lock();
        if(rf->m_iskilled_){
            spdlog::debug("raft is killed,return\n");
            return;
        }
        if(rf->m_state_ == STATE::LEADER){
            spdlog::debug("already leader,return\n");
            return;
        }
        rf->changeToState(STATE::CANDIDATE);
        int allCount = rf->m_peers_->numPeers(), grantedCount = 1, resCount = 1;
        spdlog::debug("allcount = {}\n", allCount);
        std::shared_ptr<co_chan<bool>> grantedChan(new co_chan<bool>(allCount - 1));
        std::shared_ptr<RequestVoteArgs> args(new RequestVoteArgs);
        args->set_candidateid(rf->m_me_);
        args->set_term(rf->m_current_term_);
        args->set_lastlogterm(rf->getLastLogTerm());
        args->set_lastlogindex(rf->getLastLogIndex());
        rf->persist();
        rf->co_mtx_.unlock();
        for (int i = 0; i < allCount; i++) {
            if (i == rf->m_me_) {
                continue;
            }
            go [rf, i, grantedChan,args] {

                std::shared_ptr<RequestVoteReply> reply(new RequestVoteReply);
                sendRequestVote(rf, i, args, reply);
                bool is_voted = reply->votegranted();
                *grantedChan << is_voted ; //  default false ,if rpc  success true;
                if(is_voted){
                    rf->m_electionTimer->reset(
                            getElectionTimeOut(ELECTION_TIMEOUT));
                }
                rf->co_mtx_.lock();
                if (reply->term() > rf->m_current_term_) {
                    rf->m_current_term_ = reply->term();
                    rf->changeToState(STATE::FOLLOWER);
                    rf->m_votedFor_ = -1;
                    rf->persist();
                }
                rf->co_mtx_.unlock();
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
        spdlog::debug("[{}],current_term = {},grantedCount = {}", rf->m_me_, rf->m_current_term_, grantedCount);
        if (rf->m_state_ == STATE::CANDIDATE) {
            rf->co_mtx_.lock();

            if (grantedCount > (allCount / 2)) {
                spdlog::info(
                        "before try change to leader,count:{}, currentTerm: {}, "
                        "argsTerm: {}",
                        grantedCount, rf->m_current_term_, rf->m_current_term_);
                rf->changeToState(STATE::LEADER);
            } else {
                rf->changeToState(STATE::FOLLOWER);
                spdlog::info("grant faild,state ={}", rf->stringState(rf->m_state_));
            }
            rf->co_mtx_.unlock();

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
        for (int i = 0; i < 1 && !rf->m_iskilled_; i++) {
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
                spdlog::debug("to {} success call voteRPC\n", serverId);
                break;
            }
        }
    }

};  // namespace craft