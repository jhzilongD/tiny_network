# TinyNetwork - 高性能C++网络库

基于muduo网络库设计理念，实现的轻量级高性能网络库，采用Reactor模式和One Loop Per Thread线程模型。

## ✨ 主要工作

- **实现主从Reactor架构**：主线程Accept新连接，基于epoll(LT)的IO线程池处理读写事件，通过eventfd实现线程间高效通信
- **设计One Loop Per Thread模型**：每个IO线程独占EventLoop，避免锁竞争；线程池复用减少创建开销
- **开发异步日志系统**：双缓冲区设计分离前后端，后端线程批量落盘，确保日志不影响业务性能
- **开发HTTP服务**：有限状态机解析HTTP协议；实现基本路由分发，支持HTML、JSON响应
- **设计连接上下文存储**：基于智能指针的key-value机制，支持TCP流式传输的状态管理和协议解析状态保持
- **保证代码质量**：遵循RAII原则管理资源，单元测试覆盖核心模块

## 🏗️ 架构设计

```
┌─────────────────────────────────────────┐
│     Application (HTTP/Echo Server)      │
├─────────────────────────────────────────┤
│            TcpServer                     │
│   ┌─────────────────────────────┐       │
│   │   Main Reactor (Acceptor)   │       │
│   └─────────────────────────────┘       │
│   ┌─────────────────────────────┐       │
│   │   Sub Reactors (IO Threads) │       │
│   │  ┌──────┐ ┌──────┐ ┌──────┐│       │
│   │  │Loop1 │ │Loop2 │ │Loop3 │        │
│   │  └──────┘ └──────┘ └──────┘│       │
│   └─────────────────────────────┘       │
├─────────────────────────────────────────┤
│     EventLoop (epoll + eventfd)         │
├─────────────────────────────────────────┤
│          Channel + Poller               │
├─────────────────────────────────────────┤
│         Linux TCP/IP Stack              │
└─────────────────────────────────────────┘
```

## 🚀 快速开始

### 环境要求
- Linux (kernel 2.6+)
- C++14
- CMake 3.10+
- GCC 5.0+

### 编译项目
```bash
git clone https://github.com/jhzilongD/tiny_network.git
cd tiny_network
mkdir build && cd build
cmake ..
make 
```

### 运行示例

#### Echo服务器（单线程版）
```bash
./echo_server
# 另开终端测试
telnet localhost 8080
> hello
< hello
```

#### Echo服务器（多线程版）
```bash
./echo_server_mt  # 使用4个IO线程
# 测试并发性能
for i in {1..100}; do echo "test$i" | nc localhost 8080 & done
```

#### HTTP服务器
```bash
./http_server_test
# 浏览器访问
# http://localhost:8080/
# http://localhost:8080/hello
# http://localhost:8080/api/json
```

## 📊 性能测试

```
待做
```
## 🔧 核心组件

### 网络核心 (src/net/)
| 组件 | 功能 | 特点 |
|------|------|------|
| EventLoop | 事件循环 | One Loop Per Thread，线程安全 |
| Channel | 事件分发 | 负责文件描述符的事件处理 |
| Poller | IO多路复用 | 封装epoll，使用LT模式 |
| TcpServer | TCP服务器 | 管理连接生命周期 |
| TcpConnection | TCP连接 | 处理读写事件，支持优雅关闭 |
| Buffer | 缓冲区 | 自动扩容，解决粘包问题 |
| EventLoopThreadPool | IO线程池 | Round-Robin负载均衡 |

### 应用层 (src/http/)
| 组件 | 功能 | 特点 |
|------|------|------|
| HttpServer | HTTP服务器 | 基于TcpServer构建 |
| HttpRequest | HTTP请求 | 解析HTTP请求报文 |
| HttpResponse | HTTP响应 | 构造HTTP响应报文 |
| HttpContext | HTTP上下文 | 有限状态机解析 |

### 基础设施 (src/base/ & src/logger/)
| 组件 | 功能 | 特点 |
|------|------|------|
| Thread | 线程封装 | 支持线程命名 |
| ThreadPool | 线程池 | 减少创建销毁开销 |
| AsyncLogging | 异步日志 | 双缓冲，批量落盘 |
| Timestamp | 时间戳 | 微秒级精度 |

## 💡 技术亮点

### 1. eventfd实现线程通信
```cpp
// 跨线程执行任务，无需加锁
loop->runInLoop([=]() {
    // 在IO线程中安全执行
    conn->send("message");
});
```

### 2. 连接上下文存储
```cpp
// 支持单连接多协议状态
conn->setContext("http", httpContext);
conn->setContext("session", userSession);

// 协议升级
conn->clearContext("http");
conn->setContext("websocket", wsContext);
```

### 3. 双缓冲异步日志
```cpp
// 前端写入不阻塞
LOG_INFO << "Connection established";

// 后端批量落盘
// 4MB缓冲区，每3秒或缓冲区满时写入
```

## 📚 使用示例

### 创建TCP服务器
```cpp
#include "TcpServer.h"
#include "EventLoop.h"

int main() {
    EventLoop loop;
    TcpServer server(&loop, "MyServer", 8080);
    
    // 设置线程数
    server.setThreadNum(4);
    
    // 设置连接回调
    server.setConnectionCallback([](const TcpConnectionPtr& conn) {
        if (conn->connected()) {
            LOG_INFO << "New connection from " << conn->peerAddress().toIpPort();
        } else {
            LOG_INFO << "Connection closed";
        }
    });
    
    // 设置消息回调
    server.setMessageCallback([](const TcpConnectionPtr& conn, Buffer* buf) {
        std::string msg = buf->retrieveAsString();
        conn->send(msg);  // echo
    });
    
    server.start();
    loop.loop();
    return 0;
}
```

### 创建HTTP服务器
```cpp
#include "HttpServer.h"
#include "EventLoop.h"

void onRequest(const HttpRequest& req, HttpResponse* resp) {
    if (req.path() == "/") {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->setBody("<h1>Hello TinyNetwork!</h1>");
    } else {
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setCloseConnection(true);
    }
}

int main() {
    EventLoop loop;
    HttpServer server(&loop, "HttpServer", 8080);
    server.setHttpCallback(onRequest);
    server.setThreadNum(4);
    server.start();
    loop.loop();
    return 0;
}
```

## 📁 项目结构

```
tiny_network/
├── CMakeLists.txt          # 构建配置
├── README.md               # 项目文档
├── src/                   # 源代码
│   ├── base/             # 基础组件
│   │   ├── Thread.h/cpp
│   │   ├── CurrentThread.h/cpp
│   │   ├── Timestamp.h/cpp
│   │   └── noncopyable.h
│   ├── net/              # 网络核心
│   │   ├── EventLoop.h/cpp
│   │   ├── Channel.h/cpp
│   │   ├── Poller.h/cpp
│   │   ├── TcpServer.h/cpp
│   │   ├── TcpConnection.h/cpp
│   │   ├── Buffer.h/cpp
│   │   └── ...
│   ├── http/             # HTTP模块
│   │   ├── HttpServer.h/cpp
│   │   ├── HttpRequest.h/cpp
│   │   ├── HttpResponse.h/cpp
│   │   └── HttpContext.h/cpp
│   └── logger/           # 日志系统
│       ├── AsyncLogging.h/cpp
│       ├── Logger.h/cpp
│       └── ...
├── tests/               # 单元测试
│   ├── CMakeLists.txt
│   └── test_*.cpp
├── examples/            # 示例程序  
│   ├── CMakeLists.txt
│   ├── hello_server.cpp
│   ├── echo_server*.cpp
│   └── http_server_test.cpp
└── build/              # 构建目录
```

## 🔮 后续计划

- [ ] 添加定时器功能（处理超时连接、定时任务）
- [ ] 支持WebSocket协议
- [ ] 集成SSL/TLS（OpenSSL）
- [ ] 实现连接池和内存池
- [ ] 支持UDP协议
- [ ] 性能优化（零拷贝、SO_REUSEPORT）

## 🤝 贡献

欢迎提交Issue和Pull Request！


## ⭐ Star History

如果这个项目对你有帮助，请给个 Star ⭐

## 📖 参考资料

- [muduo网络库](https://github.com/chenshuo/muduo) - 陈硕
- 《Linux多线程服务端编程：使用muduo C++网络库》
- 《UNIX网络编程》 - W. Richard Stevens
- [Linux epoll机制](https://man7.org/linux/man-pages/man7/epoll.7.html)

## 📝 License

MIT License - 详见 [LICENSE](LICENSE) 文件

---

**作者**: [dzl]  
**邮箱**: seudmax2000@gmail.com  
**创建时间**: 2025