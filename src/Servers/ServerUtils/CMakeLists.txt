cmake_minimum_required(VERSION 3.23)
project(sutils)
set(CMAKE_CXX_STANDARD 20)

add_library(
        sutils
        ${CMAKE_CURRENT_LIST_DIR}/ParsingUtils.h
)
target_link_libraries(
        sutils PRIVATE
        nlohmann_json
        spdlog
)
