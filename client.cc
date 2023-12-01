
#include "craft/client.h"

int main(int argc, char **argv) {
    std::thread([] { co_sched.Start(0, 1024); }).detach();
    spdlog::set_level(spdlog::level::debug);

    static int count = 0;
    for (int i = 0; i < 10; i++) {
        std::thread([&] {
            while (true) {
                CRaftClient client;
                ClientResult res = client.submitCommand("modify a data");
                printf("count = %d \n", count++);
            }
        }).detach();
    }
    sleep(1000000);
    return 0;
}
