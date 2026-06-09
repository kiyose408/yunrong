# src/app/CMakeLists.txt — 可执行文件

add_executable(yunrong_client main.cpp logger.cpp config_mgr.cpp ws_client.cpp ws_client.h)

target_link_libraries(yunrong_client PRIVATE Qt6::Widgets Qt6::WebSockets nlohmann_json::nlohmann_json)
