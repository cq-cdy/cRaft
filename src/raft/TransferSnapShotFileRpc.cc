#include "craft/public.h"
#include "craft/startRpcService.h"
#include "filesystem"
#include "system/json.hpp"
namespace craft {
Status RpcServiceImpl::TransferSnapShotFile(
    ::grpc::ServerContext *context,
    ::grpc::ServerReader<::TransferSnapShotFileArgs> *reader,
    ::TransferSnapShotFileReply *response) {
    nlohmann::json js{};
    js["role"] = "action";
    js["action"] = "receive_snapshot_file";
    js["timestamp"] = m_rf_->m_monitor_->timestamp();
    js["me"] = m_rf_->m_me_;

    response->set_isinstallsnapfile(false);
    js["is_installsnapfile"] = response->isinstallsnapfile();
    std::filesystem::path snapshotFilePath =
        std::filesystem::path(m_rf_->m_persister_->absPersistPath_) /
        m_rf_->m_persister_->snapshotFileName_;
    check(snapshotFilePath);

    TransferSnapShotFileArgs arg;

    std::ofstream outfile(snapshotFilePath, std::ios::out | std::ios::trunc);
    uint64_t receiveSnapFileSize = 0;
    if (!outfile) {
        spdlog::error("open snapshot file []failed");
        return Status::OK;
    }
    spdlog::info("receiving snapshot file");
    while (reader->Read(&arg)) {
        const char *data = arg.data().c_str();
        outfile.write(data, arg.data().length());
        receiveSnapFileSize += arg.data().length();
    }
    response->set_isinstallsnapfile(true);
    js["receiveSnapFileSize"] = receiveSnapFileSize;
    js["is_installsnapfile"] = response->isinstallsnapfile();
    m_rf_->m_monitor_->flush_json(js);
    *m_rf_->isCompleteSnapFileInstallCh_ << RETURN_TYPE::INSTALL_SNAPSHOT_META;
    spdlog::info("received snapshot file OK to [{}]", arg.data());
    outfile.close();
    return Status::OK;
}
}  // namespace craft