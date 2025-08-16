# 示例程序
# Headers are already included via the main CMakeLists.txt include_directories()

# 保留原来的hello_server
add_executable(hello_server hello_server.cpp)
target_link_libraries(hello_server tiny_network)

# 添加Echo服务器
add_executable(echo_server echo_server.cpp)
target_link_libraries(echo_server tiny_network)

# 添加多线程Echo服务器
add_executable(echo_server_mt echo_server_mt.cpp)
target_link_libraries(echo_server_mt tiny_network pthread)

# 添加带日志的Echo服务器
add_executable(echo_server_with_log echo_server_with_log.cpp)
target_link_libraries(echo_server_with_log tiny_network pthread)