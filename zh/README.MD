## cRaft:基于C++有栈协程的Raft分布式一致性共识算法服务框架

> 该项目开源，可随意更改和使用，当使用或更改该项目发布于其他地方时请表明出处(https://github.com/cq-cdy/cRaft) ,详情见`LICEENSE`文件。

#### [[English document here]](https://github.com/cq-cdy/cRaft)
----
* 基于**C++有栈协程**轻量高效的设计
* 支持**分布式集群**部署的稳定运行、实现有效的容错机制
* 能够**多并发**同步日志的**压力测试**下,领导者**宕机、上线**等情况保证集群的稳定性
* 使用**依赖注入**和**模板方法**等设计模式提供与上层服务具有解耦的接口
* 安装和使用快捷、方便，提供自定义快照同步服务
-----
Raft原论文：[In Search of an Understandable Consensus Algorithm
(Extended Version)](https://raft.github.io/raft.pdf)
-------------------

### 环境
> ubuntu 20.04

>gcc/++ :11.4.0，C++ 20

> CMake-3.26.4

> C++协程库libgo [ [libgo-3.1-stable下载、安装] ](https://github.com/yyzybb537/libgo/releases/tag/v3.1-stable)

> grpc-1.45.2  [[下载地址]](https://github.com/grpc/grpc)

> protobuf-3.19.4.0(安装gprc自动安装)

> spdlog 日志库 [[下载地址]](https://github.com/gabime/spdlog)

## 性能

评价指标:集群提交速率CSR(Cluster submission rate):每秒内客户端发起日志提交，集群领导者将日志复制到全部或者大多数节点后再反馈给客户端的次数。
> 由于没有充足的硬件资源，所以整体测试不是很严谨，大致测试了一下响应速率。

> 下图是此次的两种配置项的测试环境

![](../img/cRaft-test.png)

> 在两种环境下的三台和五台cRaft节点大致的测试结果：

![](../img/CSR.png)

## 安装使用
1.配置好上述环境后 git clone
```
git clone https://github.com/cq-cdy/cRaft
```
2.编译
```
cd cRaft
mkdir build && cd build
cmake .. && make
```
3.此时会生成静态库 lib/libcraft.a
```
sudo cp libcraft.a /usr/local/lib
sudo cp -r craft/ /usr/local/include
sudo cp -r rpc/  /usr/local/include
```
安装完成，接下来构建自己的分布式服务

## 如何使用

#### 1.首先，在`~/craft/craft.conf` 创建配置文件
```makefile
id = 0
#some time out [ms]
ELECTION_TIMEOUT = 800
RPC_TIMEOUT = 100
HEART_BEAT_INTERVAL= 200

#add one line like this for each server
servers = 192.168.100.100:10156
servers = 192.168.100.101:10156
servers = 192.168.100.102:10156
servers = 192.168.100.103:10156
servers = 192.168.100.104:10156

#loglevel = DEBUG
loglevel = INFO

```
每当你想新增一台节点，就增加一条`servers`信息，其中`id`是本机所在的`servers`的序号，比如`servers` = 192.168.100.102的 `id`就是1，192.168.100.100的id就是0


#### 2.创建自己的自定义服务，如KV键值存储服务,KVServer.cc,导入头文件craft/raft.h
``` cpp
#include <thread>
#include "craft/raft.h"
#include "regex"
using namespace craft;
class KVServer : public craft::AbstractPersist {

public:
    KVServer(std::string path, std::string snapFileName)
            : AbstractPersist(std::move(path), std::move(snapFileName)) {}

    void deserialization(const char *filename) override {
        // from snapshot file load data to this object


        /*
         * some IO operation ...
         */

    }

    void serialization() override {
        // save data to snapshot file,such as this object to serialize to snapshot file
        /*
            * some IO operation ...
        */
    }

    // 一些属于KV服务的方法
    void addPair(std::pair<std::string,std::string> data){
        /*  some operation*/
    }
private:
    std::map<std::string,std::string> kv_datas_;
};

int main(int argc, char **argv) {

    // start libgo coroutine
    std::thread([] { co_sched.Start(0,0); }).detach();

    //set log level
    spdlog::set_level(spdlog::level::info);

    //set snapshot and persist path
    std::string abs_path = "/home/cdy/code/projects/cRaft/.data";
    
    // set snapshot file name
    std::string snapFileName = "KVServer.snap";
    KVServer kv(abs_path, snapFileName);

    co_chan<ApplyMsg> msgCh(10000);
    craft::Raft raft(&kv, &msgCh);
    raft.launch();

    ApplyMsg msg;
    msgCh >> msg;
    spdlog::info(" get Apply msg [{},{},{}]", msg.commandValid, msg.command.content, msg.commandIndex);
    raft.saveSnapShot(msg.commandIndex);
    sleep(INT32_MAX);
}
``` 
当然，除了从配置文件读取之外，也可以在运行时设置一些参数，运行时设置的配置项优先级高于配置文件是理所应当的：
```c++
    craft::Raft raft(&kv, &msgCh);
raft.setClusterAddress({"172.16.66.100:9667","172.16.66.101:9667","172.16.66.102:9667"});
raft.setRpcTimeOut(80);
raft.setHeatBeatTimeOut(150);
raft.setLeaderEelectionTimeOut(1500);
raft.setLogLevel(spdlog::level::debug);
//Make relevant settings before raft.launch()

raft.launch();
```
这是基本的分布式服务部署的方式，其中msgCh是客户端向集群提交的日志条目被同步到大部分节点上后，会从msgCh返回得到ApplyMsg,可以自由的保存任意时刻的快照。最后决定并没有在craft服务中提供阻塞，因此需要上层服务的阻塞 sleep(INT32_MAX);，同时关于libgo协程框架的使用，在craft之上，请务必将  std::thread([] { co_sched.Start(0,0); }).detach();放在第一行开启协程，并且**集群间的快照备份是支持自动的，采用的异步流式传输，因此能够很好的应对后续快照大文件的备份。**

#### 3.客户端的使用
创建 `client.cc`
``` c++
#include "craft/client.h"

int main(int argc, char **argv) {
    spdlog::set_level(spdlog::level::debug);
    std::string command =
            "{"
            "\"id\":\"adksra4312768dsdhk\""
            ",\"op\":\"delete\""
            ",\"from\":\"172.16.66.100:12345\""
            "}";
    CRaftClient client;
    ClientResult res = client.submitCommand(command);
    if (!res.is_timeout) {
        spdlog::info("success submit a log,term = [{}],index = [{}]", res.term, res.index);
    } else {
       spdlog::error("submit log faild - time out");
    }
    return 0;
}
```
客户端对gRPC请求的封装，客户端暂时并没有像服务端那样提供运行时更改相关参数的API，因此目前客户端只能从配置文件读取相关信息，通过ClientResult返回值拿到所添加的日志的term和index

#### 4.编译运行
创建CMakeLists.txt，并且将common.cmake文件复制到你的CMakeLists.txt所在目录，当然，自此之前你得保证上面所提到的环境库已经安装好了。
```cmake
cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
...
project(KVserver C CXX)
include(common.cmake)

...
foreach(_target KVServer client)
add_executable(${_target} "${_target}.cc")
target_link_libraries(${_target}

craft
absl::flags
absl::flags_parse
${_REFLECTION}
${_GRPC_GRPCPP}
${_PROTOBUF_LIBPROTOBUF}
libgo
dl
pthread)
endforeach()
```
然后可以在其他节点上执行这些步骤，就可以开启属于自己的分布式系统。

## 接下来
暂时并没有提供动态上下线节点的功能后续会考虑加入，日志暂时还没有落盘，短时间内二次启动可能集群通信会不通畅，这貌似是grpc的特性，我在调试的时候，每次都要重新更换端口集群通讯就完全没问题。我也尝试对grpc设置了如下的端口复用，但貌似并没有效果，等知道的大佬帮忙指出一下
``` c++
...
int a =1;
builder.AddListeningPort(server_address, grpc::InsecureServerCredentials(),&a);
builder.AddChannelArgument(GRPC_ARG_ALLOW_REUSEPORT, 1);
...
```
* craft参考了[MIT 6.824 lab 的golang代码](https://github.com/1345414527/MIT6.824-2022/tree/master/raft)
