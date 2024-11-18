#include <libgo/context/context.h>
#include <libgo/coroutine.h>

#include "craft/peers.h"
#include "craft/raft.h"
#include "system/json.hpp"
#include "system/monitor/raft_runtime_task.hpp"
namespace craft {

void sendRequestVote(Raft *rf, int serverId,
                     const std::shared_ptr<RequestVoteArgs> &request,
                     const std::shared_ptr<RequestVoteReply> &response);

void startElection(Raft *rf);

void Raft::co_startElection() {
    go[this] {
        m_electionTimer->reset(getElectionTimeOut(m_leaderEelectionTimeOut_));
        for (; !m_iskilled_;) {
            RETURN_TYPE type_;
            (m_electionTimer->m_chan_) >> type_;
            if (type_ == RETURN_TYPE::TIME_OUT) {
                // nlohmann::json js{};
                // js["role"] = "action";
                // js["action"] = "election_timeout";
                // js["timestamp"] = m_monitor_->timestamp();
                // js["me"] = this->m_me_;
                // js["host_term"] = this->m_current_term_;
                // js["lastlogterm"] = this->getLastLogTerm();
                // js["lastlogindex"] = this->getLastLogIndex();
                // this->m_monitor_->flush_json(js);
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
    if (rf->m_iskilled_) {
        spdlog::debug("raft is killed,return\n");
        rf->co_mtx_.unlock();
        return;
    }
    if (rf->m_state_ == STATE::LEADER) {
        spdlog::debug("already leader,return\n");
        rf->co_mtx_.unlock();
        return;
    }
    rf->changeToState(STATE::CANDIDATE);
    int allCount = rf->m_clusterAddress_.size(), grantedCount = 1, resCount = 1;
    spdlog::debug("allCount ={} ", allCount);
    std::shared_ptr<co_chan<bool>> grantedChan(new co_chan<bool>(allCount - 1));
    rf->persist();
    rf->co_mtx_.unlock();
    auto timestamp = rf->m_monitor_->timestamp();
    go[rf, timestamp]() {
        nlohmann::json js{};
        js["role"] = "state";
        js["timestamp"] = timestamp;
        js["system_state"] = rf->m_monitor_->get_system_base_state_json();
        js["raft_state"] = rf->base_json();
        rf->m_monitor_->flush_json(js);
    };
    for (int i = 0; i < allCount; i++) {
        if (i == rf->m_me_) {
            continue;
        }
        go[rf, i, grantedChan] {
            rf->co_mtx_.lock();
            std::shared_ptr<RequestVoteArgs> args(new RequestVoteArgs);
            auto timestamp = rf->m_monitor_->timestamp();
            args->set_candidateid(rf->m_me_);
            args->set_term(rf->m_current_term_);
            args->set_lastlogterm(rf->getLastLogTerm());
            args->set_lastlogindex(rf->getLastLogIndex());
            args->set_timestamp(rf->m_monitor_->timestamp());
            std::shared_ptr<RequestVoteReply> reply(new RequestVoteReply);
            auto start = std::chrono::system_clock::now();
            sendRequestVote(rf, i, args, reply);
            auto end = std::chrono::system_clock::now();
            auto duration =
                std::chrono::duration_cast<std::chrono::seconds>(end - start);
            bool is_voted = reply->votegranted();
            go[rf, is_voted, reply, i, duration, timestamp]() {
                nlohmann::json js{};
                js["role"] = "action";
                js["action"] = "requestVote";
                js["timestamp"] = timestamp;
                js["me"] = rf->m_me_;
                js["peer"] = i;
                js["is_ok"] = is_voted;
                js["duration"] = duration.count();
                js["host_term"] = rf->m_current_term_;
                js["peer_term"] = reply->term();
                rf->m_monitor_->flush_json(js);
            };
            *grantedChan << is_voted;  //  default false ,if rpc  success true;
            if (is_voted) {
                rf->m_electionTimer->reset(
                    getElectionTimeOut(rf->m_leaderEelectionTimeOut_));
            }
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
    spdlog::info("[{}],current_term = {},VoteCount:[{}/{}]", rf->m_me_,
                 rf->m_current_term_, grantedCount, allCount);
    rf->co_mtx_.lock();
    if (rf->m_state_ == STATE::CANDIDATE) {
        if (grantedCount > (allCount / 2)) {
            rf->changeToState(STATE::LEADER);
        } else {
            spdlog::info("grant faild,VoteCount:[{}/{}]", grantedCount,
                         allCount);
            rf->changeToState(STATE::FOLLOWER);
        }
    }
    rf->co_mtx_.unlock();
    rf->m_monitor_->record_batch<>({});
}

void sendRequestVote(Raft *rf, int serverId,
                     const std::shared_ptr<RequestVoteArgs> &request,
                     const std::shared_ptr<RequestVoteReply> &response) {
    static std::vector<std::unique_ptr<RaftRPC::Stub>> &stubs =
        rf->m_peers_->getPeerStubs();
    if (serverId < 0 || serverId > rf->m_clusterAddress_.size() ||
        serverId == rf->m_me_) {
        spdlog::error("serverId:{} invalid in sendRequestVote!", serverId);
        return;
    }
    for (int i = 0; i < 1 && !rf->m_iskilled_; i++) {
        ClientContext context;
        std::chrono::system_clock::time_point deadline =
            std::chrono::system_clock::now() +
            std::chrono::milliseconds(rf->m_rpcTimeOut_);
        context.set_deadline(deadline);
        Status ok =
            stubs[serverId]->requestVoteRPC(&context, *request, response.get());
        if (!ok.ok()) {
            spdlog::error("disconnect to id[{}]:{} to try getVote\n", serverId,
                          rf->m_clusterAddress_[serverId]);
            continue;
        } else {
            spdlog::debug("to {} success call voteRPC\n", serverId);
            break;
        }
    }
}

};  // namespace craft