
#include "craft/client.h"


int main(int argc, char **argv) {
    spdlog::set_level(spdlog::level::debug);
    CRaftClient client;
    ClientResult res = client.submitCommand("modify a data");
    if (!res.is_timeout) {
        spdlog::info("success submit a log,term = [{}],index = [{}]", res.term, res.index);
    } else {
       spdlog::error("submit log faild - time out");
    }
    return 0;
}
