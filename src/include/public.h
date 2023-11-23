
#pragma once

#include <string>
#include <vector>
#include "libgo/coroutine.h"
#include <libgo/context/context.h>
#include <libgo/coroutine.h>
#include <libgo/defer/defer.h>
#include <libgo/netio/unix/epoll_reactor.h>
#include "random"
#include "spdlog/spdlog.h"

using uint = unsigned int;

const uint ELECTION_TIMEOUT = 1500;  // ms
const uint RPC_TIMEOUT = 300;       // ms
const uint HEART_BEAT_TIMEOUT = 200; //ms

const std::string last_log_index_persist_filepath = "../.data/persist/lastlogindex.data";
const std::string log_entry_term_persist_filepath = "../.data/persist/logentry.term.data";
const std::string log_entry_command_persist_filepath = "../.data/persist/logentry.command.data";
const std::string votefor_persist_filepath = "../.data/persist/votefor.data";

enum class STATE {
    FOLLOWER,
    CANDIDATE,
    LEADER,
};

enum class RETURN_TYPE {
    TIME_OUT,
    TASK_FIN,
    NON_TASK,
    STATE_CHANGED,
};
static std::vector<std::string> peersAddr = {
        "127.0.0.1:27700", "127.0.0.1:27101", "127.0.0.1:27102"//, "127.0.0.1:52003", "127.0.0.1:27704"
};
struct Command {
};
struct ApplyMsg {
    bool commandValid;
    Command *command;
    int commandIndex;
};

struct LogEntry_ {
    int term;
    Command *command;
};

static uint getElectionTimeOut(uint timeout) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(timeout, timeout * 2);
    return dis(gen);

}
