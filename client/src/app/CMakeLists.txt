# src/app/CMakeLists.txt — 可执行文件

add_executable(yunrong_client main.cpp logger.cpp)

target_link_libraries(yunrong_client PRIVATE Qt6::Widgets)
