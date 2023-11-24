#include "libgo/defer/defer.h"
#include "public.h"
#include "startRpcService.h"

namespace craft {

Status RpcServiceImpl::submitCommand(::grpc::ServerContext *context,
                                     const ::Command *request,
                                     ::ResultPackge *response) {
    m_rf_->co_mtx_.lock();
    co_defer[this] { m_rf_->co_mtx_.unlock(); };

    auto res = response->New();

}

};  // namespace craft