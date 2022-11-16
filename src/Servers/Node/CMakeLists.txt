cmake_minimum_required(VERSION 3.23)
project(Node)
set(CMAKE_CXX_STANDARD 20)

set(
        NODESOURCES
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/NodeManager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/NodeManager.h
        ${CMAKE_CURRENT_LIST_DIR}/PeerMap.cpp
        ${CMAKE_CURRENT_LIST_DIR}/PeerMap.h
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/Block.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/Block.h
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/BlockHeader.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/BlockHeader.h
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/Blockchain.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/Blockchain.h
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/TransactionPool.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/TransactionPool.h
        ${CMAKE_CURRENT_LIST_DIR}/PeerMessage.h
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/Miner.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/Miner.h
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/Transaction.h
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/Transaction.cpp
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
        cryptopp
        datachannel
        ixwebsocket
        merklecpp
        shared_models
)
