#include "public.h"
#include "startRpcService.h"

namespace craft {

    Status RpcServiceImpl::submitCommand(::grpc::ServerContext *context,
                                         const ::Command *request,
                                         ::ResultPackge *response) {
        while (m_rf_->co_mtx_.try_lock()) {

            int index = -1, term = -1;
            bool isLeader = m_rf_->m_state_ == STATE::LEADER;
            response->set_isleader(isLeader);
            if (!isLeader) {
                response->set_term(index);
                response->set_index(term);
                break;
            } else {
                term = m_rf_->m_current_term_;
                index = m_rf_->getLastLogIndex();
            }

            //to raft
            LogEntry logEntry;
            logEntry.set_term(term);
            logEntry.set_command(request->content());
            m_rf_->m_logs_.push_back(logEntry);
            m_rf_->m_matchIndex_[m_rf_->m_me_] = index;
            m_rf_->m_nextIndex_[m_rf_->m_me_] = index +1   ;

            //to client
            response->set_term(term);
            response->set_index(index);

            spdlog::debug("success submit command:[{}]", request->content());
            break;
        }
        m_rf_->co_mtx_.unlock();
        return Status::OK;

    }

};  // namespace craft
