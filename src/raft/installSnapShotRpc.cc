#include "craft/public.h"
#include "craft/startRpcService.h"

namespace craft {

    // return is can isntall snapshot file ?
    Status RpcServiceImpl::installSnapshot(::grpc::ServerContext *context,
                                           const ::InstallSnapshotArgs *request,
                                           ::InstallSnapshotReply *response) {
        m_rf_->co_mtx_.lock();

        response->set_term(m_rf_->m_current_term_);
        if (m_rf_->m_current_term_ > request->term()) {
            response->set_iscansendsnapfile(false);
            m_rf_->co_mtx_.unlock();
            return Status::OK;
        }
        if (request->term() > m_rf_->m_current_term_ ||
            m_rf_->m_state_ != STATE::FOLLOWER) {
            m_rf_->m_current_term_ = request->term();
            m_rf_->m_votedFor_ = -1;
            m_rf_->changeToState(STATE::FOLLOWER);
            m_rf_->m_electionTimer->reset(getElectionTimeOut(m_rf_->m_leaderEelectionTimeOut_));
            m_rf_->persist();
        }
        if (m_rf_->m_snapShotIndex >= request->lastincludeindex()) {
            response->set_iscansendsnapfile(false);
            m_rf_->co_mtx_.unlock();
            return Status::OK;
        }
        response->set_iscansendsnapfile(true);
        int lastIncludedIndex = request->lastincludeindex();
        int lastIncludedTerm = request->lastincludeterm();
        m_rf_->co_mtx_.unlock();
        //Delayed asynchronous installation to ensure
        // that the snapshot file is installed correctly
        // before changing the state machine metadata.
        go [this,lastIncludedIndex,lastIncludedTerm]{
            m_rf_->co_mtx_.lock();

            RETURN_TYPE X;
            spdlog::debug("now wait install snapshot file complete");
            *m_rf_->isCompleteSnapFileInstallCh_ >> X; // wait for install snapshot file complete
            if(X == RETURN_TYPE::INSTALL_SNAPSHOT_META){
                spdlog::info("now install snapshot metadata");
                int lastIndex = m_rf_->getLastLogIndex();
                if (lastIncludedIndex > lastIndex) {
                    m_rf_->m_logs_.resize(1);
                } else {
                    int installLen = lastIncludedIndex - m_rf_->m_snapShotIndex;
                    m_rf_->m_logs_.erase(m_rf_->m_logs_.begin(), m_rf_->m_logs_.begin() + installLen);
                    m_rf_->m_logs_[0].set_command("");
                }
                m_rf_->m_logs_[0].set_term(lastIncludedTerm);
                m_rf_->m_snapShotIndex = lastIncludedIndex;
                m_rf_->m_snapShotTerm = lastIncludedTerm;

                //Restore the state machine from the snapshot,
                // and the deserialization method is rewritten
                // and implemented by the upper-layer service.
                m_rf_->m_persister_->deserialization(m_rf_->m_persister_->snapshotFileName_.c_str());
            }else if  (X == RETURN_TYPE::NOT_INSTALL_SNAPSHOT_META){
                spdlog::error("Do not allow changes to the state machine");
            }else if (X == RETURN_TYPE::INSTALL_SNAPSHOT_FILE_FAILED){
                spdlog::error("Install snapshot file request timed out");
            }
            m_rf_->co_mtx_.unlock();
        };

        return Status::OK;

    }

}  // namespace craft