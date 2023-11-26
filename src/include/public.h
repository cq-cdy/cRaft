
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

const uint ELECTION_TIMEOUT = 1000;  // ms
const uint RPC_TIMEOUT = 150;       // ms
const uint HEART_BEAT_TIMEOUT = 300; //ms


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
        "127.0.0.1:29870", "127.0.0.1:28335", "127.0.0.1:27832", "127.0.0.1:21003", "127.0.0.1:21704"
};
struct Command_ {
};
struct ApplyMsg {
    bool commandValid;
    Command_ *command;
    int commandIndex;
};

struct LogEntry_ {
    int term;
    Command_ *command;
};

static uint getElectionTimeOut(uint timeout) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(timeout, timeout*2);
    return dis(gen);

}
