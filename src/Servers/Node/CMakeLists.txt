cmake_minimum_required(VERSION 3.23)
project(Node)
set(CMAKE_CXX_STANDARD 20)

set(
        NODESOURCES
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)
add_executable(node ${NODESOURCES})
target_include_directories(
        node
        PRIVATE
        src/SharedModels
)
target_link_libraries(
        node PRIVATE
        nlohmann_json
        fmt
        spdlog
        eventpp
        datachannel
        ixwebsocket
        shared_models
)
