#include "libgo/coroutine.h"
#include "public.h"
#include "raft.h"

namespace craft {

void Raft::co_appendAentries() {
    spdlog::debug("co_appendAentries()");
    static std::vector<std::unique_ptr<RaftRPC::Stub>> &stubs =
        m_peers_->getPeerStubs();
    go[this] {
        for (;;) {
            co_sleep(HEART_BEAT_TIMEOUT);
            STATE state = m_state_;
          
            if (state == STATE::LEADER) {
                spdlog::debug("in co_appendAentries state:[{}],my term is [{}]", stringState(state),m_current_term_);
                go[this] {
                    for (int i = 0; i < this->m_peers_->numPeers(); i++) {
                        if (i == this->m_me_) {
                            continue;
                        }
                        ClientContext context;
                        std::chrono::system_clock::time_point deadline =
                            std::chrono::system_clock::now() +
                            std::chrono::milliseconds(RPC_TIMEOUT);
                        context.set_deadline(deadline);
                        AppendEntriesArgs args;
                        auto entry   = args.add_entries();
                        args.set_term(101);
                        entry->set_command("test log");
                        auto entry2   = args.add_entries();
                        entry2->set_command("test log2");

                        AppendEntriesReply response;
                        Status ok =
                            stubs[i]->appendEntries(&context, args, &response);
                        if (ok.ok()) {
                            spdlog::debug("heat beat ok");
                            m_electionTimer->reset(
                                getElectionTimeOut(ELECTION_TIMEOUT));
                            if(response.term() > m_current_term_){
                                changeToState(STATE::FOLLOWER);
                            }
                        } else {
                            spdlog::error("heat beat err");
                        }
                    }
                };
            } else {
                RETURN_TYPE a;
                *m_StateChangedCh_ >> a;
                spdlog::debug("recvive state changed");
                spdlog::debug("in co_appendAentries state:[{}],my term is [{}]", stringState(state),m_current_term_);
            }
        }
    };
}
};  // namespace craft