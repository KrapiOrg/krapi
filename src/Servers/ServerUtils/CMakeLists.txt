cmake_minimum_required(VERSION 3.23)
project(sutils)
set(CMAKE_CXX_STANDARD 20)
set(SUTILSPREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src/Servers/ServerUtils)
add_library(
        sutils
        ${SUTILSPREFIX}/ParsingUtils.h
)
target_link_libraries(sutils PRIVATE
        ixwebsocket
        nlohmann_json
        fmt
        spdlog
        cryptopp
        cxxopts
        )
