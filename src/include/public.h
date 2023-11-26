
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

const uint ELECTION_TIMEOUT = 700;  // ms
const uint RPC_TIMEOUT = 100;       // ms
const uint HEART_BEAT_INTERVAL= 200; //ms


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
        "127.0.0.1:40116", "127.0.0.1:40115", "127.0.0.1:52102", "127.0.0.1:41403", "127.0.0.1:40114"
};
struct Command_ {
    std::string  content;
};
struct ResultPackge{
    int term{};
    int index{};
    bool isLeader{};
};
struct ApplyMsg {
    bool commandValid;
    Command_ *command;
    int commandIndex;
};

struct LogEntry_ {
    int term;
    Command_ command;
};

static uint getElectionTimeOut(uint timeout) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(timeout, timeout*2);
    return dis(gen);

}
