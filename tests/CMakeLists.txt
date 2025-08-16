# 测试程序
# Headers are already included via the main CMakeLists.txt include_directories()
add_executable(test_inetaddress test_inetaddress.cpp)
target_link_libraries(test_inetaddress tiny_network)

# 添加noncopyable测试程序
add_executable(test_noncopyable test_noncopyable.cpp)

# 添加Timestamp测试程序
add_executable(test_timestamp test_timestamp.cpp)
target_link_libraries(test_timestamp tiny_network)

# 添加CurrentThread测试程序
add_executable(test_currentthread test_currentthread.cpp)
target_link_libraries(test_currentthread tiny_network)

# 添加Thread测试程序
add_executable(test_thread test_thread.cpp)
target_link_libraries(test_thread tiny_network pthread)

# 添加Channel测试程序
add_executable(test_channel test_channel.cpp)
target_link_libraries(test_channel tiny_network)

# 添加Poller和Channel配合测试程序
add_executable(test_poller_channel test_poller_channel.cpp)
target_link_libraries(test_poller_channel tiny_network)

# 添加EventLoop测试程序
add_executable(test_eventloop test_eventloop.cpp)
target_link_libraries(test_eventloop tiny_network pthread)

# 添加TcpConnection测试程序
add_executable(test_tcpconnection test_tcpconnection.cpp)
target_link_libraries(test_tcpconnection tiny_network)

# 添加LogStream测试程序
add_executable(test_logstream test_logstream.cpp)
target_link_libraries(test_logstream tiny_network)

# 添加Logger测试程序
add_executable(test_logger test_logger.cpp)
target_link_libraries(test_logger tiny_network pthread)

# 添加eventfd机制测试程序
add_executable(test_eventfd test_eventfd.cpp)
target_link_libraries(test_eventfd tiny_network pthread)