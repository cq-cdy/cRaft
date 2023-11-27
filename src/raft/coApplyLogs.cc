#include "raft.h"
#include "public.h"

namespace craft {

    void startApplyLogs(Raft *rf);

    void Raft::co_applyLogs() {

        go [this] {

            for(;!m_iskilled_;)
            {
                void * a;
                *m_notifyApplyCh_>> a;
                startApplyLogs(this);
            }


        };

    }

    void startApplyLogs(Raft *rf) {
        rf->co_mtx_.lock();
//        co_defer [rf] {
//            rf->m_applyTimer->reset(APPLY_INTERVAL);
//        };
        std::vector<ApplyMsg> msgs;

        if (rf->m_lastApplied_ < rf->m_snapShotIndex) {
            //todo install snapshot
        } else if (rf->m_commitIndex_ <= rf->m_lastApplied_) {
                spdlog::info("rf->m_commitIndex_ <= rf->m_lastApplied_");
        } else {
            msgs.resize(rf->m_commitIndex_ - rf->m_lastApplied_);
            for (int i = rf->m_lastApplied_ + 1; i <= rf->m_commitIndex_; i++) {
                msgs.push_back(ApplyMsg{true,
                                        rf->m_logs_[i].command(),
                                        i});
            }

        }
        rf->co_mtx_.unlock();
        for(const auto& msg:msgs){
            rf->co_mtx_.lock();
            *rf->m_applyCh_ << msg;
            rf->m_lastApplied_ = msg.commandIndex;
            rf->co_mtx_.unlock();
        }
    }
};