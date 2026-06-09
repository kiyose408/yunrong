cmake_minimum_required(VERSION 3.20)
project(YunRong VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ===== 选项 =====
option(BUILD_TESTS "Build unit and integration tests" ON)
option(BUILD_GUI   "Build Qt Widgets GUI" ON)

# ===== 查找 Qt =====
find_package(Qt6 REQUIRED COMPONENTS Core Network WebSockets Sql)
if(BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
endif()
if(BUILD_TESTS)
    find_package(Qt6 REQUIRED COMPONENTS Test)
endif()

# ===== 第三方依赖 (FetchContent) =====
include(FetchContent)

# 国内网络环境使用 Gitee 镜像
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://gitee.com/mirrors/nlohmann-json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://gitee.com/kiyose408/spdlog.git
    GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)

# ===== 子目录 =====
add_subdirectory(src/app)
# T013: add_subdirectory(src/core)
