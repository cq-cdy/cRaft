
#include "craft/client.h"

#include <random>

std::string generateRandomString(int n) {
    std::string result;
    static const char alphanum[] =
        "abcdefghijklmnopqrstuvwxyz"; // 可以在这里添加其他字符
    std::srand(std::time(nullptr)); // 使用当前时间初始化随机种子
    for (int i = 0; i < n; ++i) {
        result += alphanum[std::rand() % (sizeof(alphanum) - 1)];
    }
    return result;
}
static std::string randomBytes() {
    int random_int = std::rand() % 10;
    if (random_int < 3) {
        return generateRandomString((std::rand() % 5000) + 5000);
    } else {
        return generateRandomString((std::rand() % 5000) + 1);
    }
}
int main(int argc, char **argv) {
    spdlog::set_level(spdlog::level::debug);
    static int count = 0;
    // simple stress test
    for (int i = 0; i < 2; i++) {
        std::thread([&] {
            CRaftClient client;
            while (true) {
                auto command = randomBytes();
                ClientResult res = client.submitCommand(command);
                std::cout << res.is_timeout << std::endl;
                sleep(std::rand() % 4);
            }
        }).detach();
    }
    sleep(1000000);
    return 0;
}
