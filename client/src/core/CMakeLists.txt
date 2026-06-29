# src/core/CMakeLists.txt — 核心库（无 Qt GUI 依赖）

add_library(yunrong_core STATIC)

target_include_directories(yunrong_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(yunrong_core PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
    nlohmann_json::nlohmann_json
)

target_sources(yunrong_core PRIVATE
    infra/logger.cpp
    infra/config_mgr.cpp
    net/http_client.cpp
    net/protocol.cpp
)

include(../../cmake/Platform.cmake)
