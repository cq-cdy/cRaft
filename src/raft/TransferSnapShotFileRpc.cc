#include "craft/public.h"
#include "craft/startRpcService.h"
#include "filesystem"

namespace craft {
    Status
    RpcServiceImpl::TransferSnapShotFile(::grpc::ServerContext *context,
                                         ::grpc::ServerReader<::TransferSnapShotFileArgs> *reader,
                                         ::TransferSnapShotFileReply *response) {
        response->set_isinstallsnapfile(false);
        std::filesystem::path snapshotFilePath =
                std::filesystem::path(m_rf_->m_persister_->absPersistPath_) / m_rf_->m_persister_->snapshotFileName_;
        check(snapshotFilePath);

        TransferSnapShotFileArgs arg;

        std::ofstream outfile(snapshotFilePath, std::ios::out|std::ios::trunc);
        if(!outfile){
            spdlog::error("open snapshot file []failed");
            return Status::OK;
        }
        spdlog::info("receiving snapshot file");
        while (reader->Read(&arg)) {
            const char* data = arg.data().c_str();
            outfile.write(data, arg.data().length());
        }
        response->set_isinstallsnapfile(true);
        *m_rf_->isCompleteSnapFileInstallCh_ <<RETURN_TYPE::INSTALL_SNAPSHOT_META;
        spdlog::info("received snapshot file OK to [{}]",arg.data());
        outfile.close();
        return Status::OK;
    }
}