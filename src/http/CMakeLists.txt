# HTTP模块的CMakeLists.txt

# HTTP模块源文件
set(HTTP_SOURCES
    HttpRequest.cpp
    HttpResponse.cpp
    HttpContext.cpp
    HttpServer.cpp
)

# 添加HTTP测试可执行文件
add_executable(http_server_test ../../examples/http_server_test.cpp ${HTTP_SOURCES})

# 链接依赖库
target_link_libraries(http_server_test 
    tiny_network 
    pthread
)

# Headers are already included via the main CMakeLists.txt include_directories()