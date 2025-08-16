cmake_minimum_required(VERSION 3.10)
project(TinyNetwork)

# 生成compile_commands.json供IDE使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 使用C++14
set(CMAKE_CXX_STANDARD 14)

# 添加源文件，创建动态库
add_library(tiny_network SHARED
    src/net/InetAddress.cpp
    src/net/Channel.cpp
    src/net/Poller.cpp
    src/net/EventLoop.cpp
    src/net/TcpConnection.cpp
    src/net/Acceptor.cpp
    src/net/TcpServer.cpp
    src/net/Buffer.cpp
    src/net/Socket.cpp
    src/net/EventLoopThread.cpp
    src/net/EventLoopThreadPool.cpp
    src/base/Timestamp.cpp
    src/base/CurrentThread.cpp
    src/base/Thread.cpp
    src/logger/LogStream.cpp
    src/logger/LogFile.cpp
    src/logger/FileUtil.cpp
    src/logger/AsyncLogging.cpp
    src/logger/Logger.cpp
    src/http/HttpRequest.cpp
    src/http/HttpResponse.cpp
    src/http/HttpContext.cpp
    src/http/HttpServer.cpp
)

# 设置头文件搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/src/base
    ${PROJECT_SOURCE_DIR}/src/net
    ${PROJECT_SOURCE_DIR}/src/logger
    ${PROJECT_SOURCE_DIR}/src/http
)

# 添加HTTP模块子目录
add_subdirectory(src/http)

# 添加子目录
add_subdirectory(tests)
add_subdirectory(examples)

# 设置库的链接配置
target_link_libraries(tiny_network pthread)

# 设置生成动态库的路径，放在根目录的lib文件夹下面
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置安装的默认路径到项目目录
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

# 将目录安装到 include/tiny_network 下（只安装头文件）
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/ DESTINATION include/tiny_network
        FILES_MATCHING PATTERN "*.h")