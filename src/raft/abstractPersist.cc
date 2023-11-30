#include "craft/persist/abstractPersist.h"
#include "filesystem"

namespace craft {
    AbstractPersist::AbstractPersist(std::string absPersistPath,std::string snapshotFileName_)
            : absPersistPath_(std::move(absPersistPath)), snapshotFileName_(std::move(snapshotFileName_)) {
        std::vector<std::string> all_persist_files = {"commitIndex.data", "currentTerm.data", "lastlogindex.data",
                                                      "lastSnapshotIndex.data", "lastSnapshotTerm.data",
                                                      "logentry.command.data", "logentry.term.data", "votefor.data"};
        for (auto &file: all_persist_files) {
            std::filesystem::path dir = absPersistPath_;
            std::filesystem::path full_path = dir /"persist"/ file;
            if (! std::filesystem::exists(full_path.parent_path())) {
                if ( std::filesystem::create_directories(full_path.parent_path())) {
                    spdlog::info("success create path:[{}]",full_path.parent_path().string());
                } else {
                    spdlog::error("error create path:[{}]",full_path.parent_path().string());                }
            }
            if (!std::filesystem::exists(full_path)) {
                std::ofstream ofs(full_path);
                if (ofs) {
                    spdlog::info("success create file:[{}]", full_path.string());
                } else {
                    spdlog::critical("error create file:[{}]", full_path.string());
                    exit(2);
                }
            } else {
                spdlog::info("success find file:[{}]", full_path.string());
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

        if (commitIndex.size() != 1 && !commitIndex.empty()) {
            spdlog::error("commitIndex.data content error");
        }
        if (currentTerm.size() != 1 && !currentTerm.empty()) {
            spdlog::error("currentTerm.data content error");
        }
        if (lastlogindex.size() != 1 && !lastlogindex.empty()) {
            spdlog::error("lastlogindex.data content error");
        }
        if (lastSnapshotIndex.size() != 1 && !lastSnapshotIndex.empty()) {
            spdlog::error("lastSnapshotIndex.data content error");
        }
        if (lastSnapshotTerm.size() != 1 && !lastSnapshotTerm.empty()) {
            spdlog::error("lastSnapshotTerm.data content error");
        }
        if (votefor.size() != 1 && !votefor.empty()) {
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
            commitIndex_ = std::stoi(commitIndex[0]);
        }
        if (!currentTerm.empty()) {
            currentTerm_ = std::stoi(currentTerm[0]);
        }
        if (!lastlogindex.empty()) {
            lastlogindex_ = std::stoi(lastlogindex[0]);
        }
        if (!votefor.empty()) {
            votedFor_ = std::stoi(votefor[0]);
        }
        if (!lastSnapshotIndex.empty()) {
            lastSnapshotIndex_ = std::stoi(lastSnapshotIndex[0]);
        }
        if (!lastSnapshotTerm.empty()) {
            lastSnapshotTerm_ = std::stoi(lastSnapshotTerm[0]);
        }
        if((!logentry_term.empty())&&(!logentry_command.empty())){
            for (int i = 0; i < logentry_term.size(); i++) {
                int term = std::stoi(logentry_term[i]);
                logEntry_.emplace_back(term, logentry_command[i]);
            }
        }

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
            spdlog::error("can not open file:{}", filename);
        }

        return lines;
    }


    AbstractPersist::~AbstractPersist() = default;
};  // namespace craft