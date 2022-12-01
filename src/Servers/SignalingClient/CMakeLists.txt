project(signaling_client)
cmake_minimum_required(VERSION 3.24)
include_directories(${CMAKE_CURRENT_LIST_DIR})
add_library(
        signaling_client
        ${CMAKE_CURRENT_LIST_DIR}/SignalingClient.cpp
        ${CMAKE_CURRENT_LIST_DIR}/SignalingClient.h
)
target_link_libraries(
        signaling_client
        PRIVATE
        ixwebsocket
        nlohmann_json
        spdlog
        stduuid
        shared_models
)