cmake_minimum_required(VERSION 3.23)
project(node)
set(CMAKE_CXX_STANDARD 20)

set(
        NODESOURCES
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/Blockchain.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/Blockchain.h
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
        stduuid
        spdlog
        eventpp
        cryptopp
        datachannel
        ixwebsocket
        merklecpp
        leveldb
        concurrencpp
        miner
        shared_models
        blockchain_models
        transaction_pool
        node_manager
        utils
)
