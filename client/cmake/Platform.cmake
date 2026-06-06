# cmake/Platform.cmake — 平台差异隔离
# 用法：在 core/CMakeLists.txt 末尾 include(../../cmake/Platform.cmake)

if(WIN32)
    target_compile_definitions(yunrong_core PRIVATE PLATFORM_WINDOWS)
    target_link_libraries(yunrong_core PRIVATE ws2_32 crypt32)
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(yunrong_core PRIVATE PLATFORM_LINUX)
    target_link_libraries(yunrong_core PRIVATE pthread dl)
endif()
