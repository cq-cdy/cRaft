
#include "craft/client.h"

int main(int argc, char **argv) {
    std::thread([] { co_sched.Start(0, 0); }).detach();
    spdlog::set_level(spdlog::level::debug);

    static int count = 0;
    std::string command =
            "{"
            "\"id\":\"auvm_sk_4312768ds1\""
            ",\"op\":\"delete\""
            ",\"from\":\"172.16.66.100:12345\""
            "}";
    //simple stress test
    for (int i = 0; i < 16; i++) {
        std::thread([&] {
            while (true) {
                CRaftClient client;
                for (int j = 0; j < 1000000; j++) {
                    ClientResult res = client.submitCommand(command);
                    printf("count = %d \n", count++);
                }
            }
        }).detach();
    }
    sleep(1000000);
    return 0;
}
