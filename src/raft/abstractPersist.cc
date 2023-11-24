#include <utility>

#include "persist/abstractPersist.h"
#include "filesystem"

namespace craft {
    AbstractPersist::AbstractPersist(std::string absPersistPath) {
        this->absPersistPath_ = std::move(absPersistPath);
        std::vector<std::string> all_persist_files = {"commitIndex.data", "currentTerm.data", "lastlogindex.data",
                                                      "lastSnapshotIndex.data", "lastSnapshotTerm.data",
                                                      "logentry.command.data", "logentry.term.data", "votefor.data"};

        for (auto &file: all_persist_files) {
            std::filesystem::path dir = absPersistPath_;
            std::filesystem::path full_path = dir /"persist"/ file;
            if (! std::filesystem::exists(full_path.parent_path())) {
                // 如果父目录不存在，创建它
                if ( std::filesystem::create_directories(full_path.parent_path())) {
                    spdlog::info("success create path [{}]",full_path.parent_path().string());
                } else {
                    spdlog::error("error create path [{}]",full_path.parent_path().string());                }
            }
            if (!std::filesystem::exists(full_path)) {
                std::ofstream ofs(full_path);
                if (ofs) {
                    spdlog::info("success create file [{}]", full_path.string());
                } else {
                    spdlog::error("error create file [{}]", full_path.string());
                    continue;
                }
            } else {
                spdlog::info("success find file [{}]", full_path.string());
            }
            file = full_path;
        }
        auto commitIndex = readLines(all_persist_files[0]);
        auto currentTerm = readLines(all_persist_files[1]);
        auto lastlogindex = readLines(all_persist_files[2]);
        auto lastSnapshotIndex = readLines(all_persist_files[3]);
        auto lastSnapshotTerm = readLines(all_persist_files[4]);
        auto logentry_command = readLines(all_persist_files[5]);
        auto logentry_term = readLines(all_persist_files[6]);
        auto votefor = readLines(all_persist_files[7]);

        if (commitIndex.size() != 1 && commitIndex.size() != 0) {
            spdlog::error("commitIndex.data content error");
        }
        if (currentTerm.size() != 1 && currentTerm.size() != 0) {
            spdlog::error("currentTerm.data content error");
        }
        if (lastlogindex.size() != 1 && lastlogindex.size() != 0) {
            spdlog::error("lastlogindex.data content error");
        }
        if (lastSnapshotIndex.size() != 1 && lastSnapshotIndex.size() != 0) {
            spdlog::error("lastSnapshotIndex.data content error");
        }
        if (lastSnapshotTerm.size() != 1 && lastSnapshotTerm.size() != 0) {
            spdlog::error("lastSnapshotTerm.data content error");
        }
        if (votefor.size() != 1 && votefor.size() != 0) {
            spdlog::error("votefor.data content error");
        }
        if (logentry_command.size() != logentry_term.size()) {
            spdlog::warn("logentry_term and logentry_command length not match");
            int min_ = std::min({logentry_command.size(), logentry_term.size()});
            logentry_term.resize(min_);
            logentry_command.resize(min_);
        }
        // ------------------
        if (!commitIndex.empty()) {
            commitIndex_ = std::atoi(commitIndex[0].c_str());
        }
        if (!currentTerm.empty()) {
            currentTerm_ = std::atoi(currentTerm[0].c_str());
        }
        if (!lastlogindex.empty()) {
            lastlogindex_ = std::atoi(lastlogindex[0].c_str());
        }
        if (!votefor.empty()) {
            votedFor_ = std::atoi(votefor[0].c_str());
        }
        if (!lastSnapshotIndex.empty()) {
            lastSnapshotIndex_ = std::atoi(lastSnapshotIndex[0].c_str());
        }
        if (!lastSnapshotTerm.empty()) {
            lastSnapshotTerm_ = std::atoi(lastSnapshotTerm[0].c_str());
        }
        if((!logentry_term.empty())&&(!logentry_command.empty())){
            for (int i = 0; i < logentry_term.size(); i++) {
                int term = std::atoi(logentry_term[i].c_str());
                logEntry_.emplace_back(term, logentry_command[i]);
            }
        }

//        using namespace std;
//        auto lastlogindex = readLines(filesystem::absolute(filesystem::path(last_log_index_persist_filepath)));
//        auto logentry_term = readLines(filesystem::absolute(filesystem::path(log_entry_term_persist_filepath)));
//        auto logentry_command = readLines(filesystem::absolute(filesystem::path(log_entry_command_persist_filepath)));
//        auto votefor = readLines(filesystem::absolute(filesystem::path(votefor_persist_filepath)));
//        if (lastlogindex.size() != 1) {
//            spdlog::error("lastloginde.data error");
//        }
//        if (votefor.size() != 1) {
//            spdlog::error("votefor.data error");
//        }
//        if (logentry_term.size() != logentry_command.size()) {
//            spdlog::warn("logentry_term and logentry_command length not match");
//            int min_ = std::min({logentry_command.size(), logentry_term.size()});
//            logentry_term.resize(min_);
//            logentry_command.resize(min_);
//        }
//        lastlogindex_ = std::atoi(lastlogindex[0].c_str());
//        votedFor_ = std::atoi(votefor[0].c_str());
//        for (int i = 0; i < logentry_term.size(); i++) {
//            int term = std::atoi(logentry_term[i].c_str());
//            logEntry_.emplace_back(term, logentry_command[i]);
//        }

    }

    int AbstractPersist::getLastLogIndex() const { return lastlogindex_; }

    int AbstractPersist::getVotedFor() const { return votedFor_; }

    int AbstractPersist::getCommitIndex() const {
        return commitIndex_;
    }

    int AbstractPersist::getCurrentTerm() const {
        return currentTerm_;
    }

    int AbstractPersist::getLastSnapshotTerm() const {
        return lastSnapshotTerm_;
    }

    int AbstractPersist::getLastSnapshotIndex() const {
        return lastSnapshotIndex_;
    }

    std::vector<std::pair<int, std::string>> AbstractPersist::getLogEntries() {
        return logEntry_;
    }

    std::vector<std::string> AbstractPersist::readLines(
            const std::string &filename) {
        std::vector<std::string> lines;
        std::ifstream file(filename);

        if (file.is_open()) {
            std::string line;
            while (getline(file, line)) {
                lines.push_back(line);
            }
            file.close();
        } else {
            spdlog::error("无法打开文件:{}", filename);
        }

        return lines;
    }


    AbstractPersist::~AbstractPersist() = default;
};  // namespace craft