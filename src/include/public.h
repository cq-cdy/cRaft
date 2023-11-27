
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
const uint RPC_TIMEOUT = 100;       // ms
const uint HEART_BEAT_INTERVAL= 500; //ms
const uint APPLY_INTERVAL= 500; //ms


enum class STATE {
    FOLLOWER,
    CANDIDATE,
    LEADER,
};

enum class RETURN_TYPE {
    TIME_OUT,
    APPLYED,
    STATE_CHANGED,
};
static std::vector<std::string> peersAddr = {
        "127.0.0.1:8110", "127.0.0.1:8151", "127.0.0.1:8172", "127.0.0.1:19664", "127.0.0.1:16464"
};
struct Command_ {
    std::string  content;
};

struct ApplyMsg {
    bool commandValid{};
    Command_ command{};
    int commandIndex{};
};

//struct LogEntry_ {
//    int term;
//    Command_ command;
//};

static uint getElectionTimeOut(uint timeout) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(timeout, timeout*2);
    return dis(gen);

}
