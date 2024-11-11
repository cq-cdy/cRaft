//
// Created by cdy on 23-11-4.
//
#pragma once

#include <sys/wait.h>
#include <unistd.h>

#include <cstring>
#include <functional>
#include <string>

#include "../craft/raft.h"
static int current_start_count = 0;
static int child_pid = 0;

/*
pid：指标识要检查或等待的子进程的方式。这个参数的不同取值具有不同的含义：
waitpid((pid_t)(-1), 0, WNOHANG)
如果 pid > 0，那么waitpid就只会等待PID等于pid的子进程。
如果 pid 等于0，那么waitpid就会等待和当前进程在同一个进程组的任意子进程。
如果 pid 小于 -1，那么waitpid就会等待其组ID与pid的绝对值相同的任何子进程。
如果 pid 等于 -1，那么waitpid会等待任何子进程，这也就是你代码中的用法。

status：这是一个指向int变量的指针，用来保存子进程的状态信息。
这个值可以传递NULL，
若不为NULL，其主要用于判断子进程是正常退出还是被信号结束，
进程结束状态等信息。

options：这个参数用来改变 waitpid 的行为，比如你在代码中用的 WNOHANG。

WNOHANG：是因为正常的 waitpid
是一个阻断型函数，如果子进程还未结束，父进程会一直等待。如果我们希望父进程不要阻断，可以使用
WNOHANG 这个option，这样如果没有已经结束的子进程，waitpid函数会立即返回0。
WUNTRACED 和 WCONTINUED
是另外两个选项，当子进程被暂停(resumed)或继续(continued)时，也可以让 waitpid()
返回。
*/
static void handle_sigchld(int sig) {
    while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {
        printf("hui shou\n");
    }
}
static void handle_sigint(int sig) {
    printf("in handle_sigint kill %d \n", child_pid);

    if (kill(child_pid, SIGKILL) < 0) {
        perror("kill err");
    } else {
        printf("kill succ\n");
    }
    exit(0);
}
class HighAvai {
   private:
    HighAvai() = default;

   public:
    static HighAvai* getInstance(std::function<void()> f,
                                 int random_restart_seconds = 3) {
        static HighAvai h;
        h.server_func_ = f;
        // 在random_restart_seconds 到 2 * random_restart_seconds 之间随机
        h.random_restart_seconds_ = random_restart_seconds;
        return &h;
    }
    void setRestartCount(int count) { this->restart_count_ = count; }

    int start(int argc, char* argv[], void (*OtherfuncPtr)(void) = nullptr) {
        char* program_name = argv[0];
        if (argc > 1) {
            current_start_count = std::atoi(argv[1]);
            printf("restart_count = %d, current_start_count= %d\n",
                   restart_count_, current_start_count);
            if (current_start_count > restart_count_) {
                printf("no restart\n");
                exit(2);
            }
        }
        // 安装SIGCHLD信号处理函数
        struct sigaction sa;
        sa.sa_handler = &handle_sigchld;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
        if (sigaction(SIGCHLD, &sa, 0) == -1) {
            perror("sigaction failed");
            exit(1);
        }
        pid_t pid = fork();  // 创建子进程
        printf("argc = %d\n", argc);
        if (pid == -1) {
            printf("Failed to fork()");
            exit(-1);
        } else if (pid > 0) {  // fu
            child_pid = pid;
            printf("zi id : %d \n", child_pid);
            signal(SIGINT, handle_sigint);

            printf("current_start_count = %d --\n", current_start_count);

            int status;
            waitpid(child_pid, &status, 0);  // 等待子进程结束

            char str_restart_count[10];
            sprintf(str_restart_count, "%d", current_start_count + 1);
            char* new_argv[] = {program_name, str_restart_count, nullptr};
            sleep(random_restart_seconds_ + rand() % random_restart_seconds_);
            execvp(program_name, new_argv);
            exit(1);
        } else {
            server_func_();
           
            exit(0);
        }
    }

   private:
    std::function<void()> server_func_;
    int restart_count_ = 5;
    int random_restart_seconds_ = 5;
};
