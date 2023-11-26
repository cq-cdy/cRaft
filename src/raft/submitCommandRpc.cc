#include "libgo/defer/defer.h"
#include "public.h"
#include "startRpcService.h"

namespace craft {

    Status RpcServiceImpl::submitCommand(::grpc::ServerContext *context,
                                         const ::Command *request,
                                         ::ResultPackge *response) {
        m_rf_->co_mtx_.lock();
        int index = -1, term = -1;
        bool isLeader = m_rf_->m_state_ == STATE::LEADER;
        response->set_isleader(isLeader);
        if (!isLeader) {
            response->set_term(index);
            response->set_index(term);
            return Status::OK;
        } else {
            term = m_rf_->m_current_term_;
            index = m_rf_->m_lastApplied_;
        }

        LogEntry logEntry;
        logEntry.set_command(request->content());
        response->set_term(term);
        response->set_index(index);

        spdlog::debug("success submit command [{}]", request->content());
        m_rf_->co_mtx_.unlock();
        return Status::OK;

    }

};  // namespace craft