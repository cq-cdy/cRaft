#include "raft.h"
#include "public.h"

namespace craft{
    void Raft::co_applyLogs() {

        go [this] {
            spdlog::debug("co_applyLogs()");
        };

    }
};