cmake_minimum_required(VERSION 3.23)
project(krapi)

set(CMAKE_CXX_STANDARD 20)

set(USE_TLS 1)
add_subdirectory(libs/websockets)
add_subdirectory(libs/json)
add_subdirectory(libs/fmt)
set(SPDLOG_FMT_EXTERNAL ON)
add_subdirectory(libs/spdlog)
add_subdirectory(libs/cryptopp)
add_subdirectory(libs/cxxopts)
add_subdirectory(test)

include_directories(src)
add_executable(krapi main.cpp src/Utils/ErrorOr.h)
target_link_libraries(krapi PRIVATE
        ixwebsocket
        nlohmann_json
        fmt
        spdlog
        cryptopp
        cxxopts
        )
