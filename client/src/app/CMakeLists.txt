# src/app/CMakeLists.txt — 可执行文件

add_executable(yunrong_client main.cpp logger.cpp config_mgr.cpp)

target_link_libraries(yunrong_client PRIVATE Qt6::Widgets nlohmann_json::nlohmann_json)
