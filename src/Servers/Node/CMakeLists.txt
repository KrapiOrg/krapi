cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 20)

set(
        NODESOURCES
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/Blockchain.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/Blockchain.h
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/BlockchainManager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain/BlockchainManager.h
        ${CMAKE_CURRENT_LIST_DIR}/Miner.h
        ${CMAKE_CURRENT_LIST_DIR}/SpentTransactionsStore.h
        ${CMAKE_CURRENT_LIST_DIR}/Validator.h
        ${CMAKE_CURRENT_LIST_DIR}/TransactionPool/TransactionPool.cpp
        ${CMAKE_CURRENT_LIST_DIR}/TransactionPool/TransactionPool.h
        ${CMAKE_CURRENT_LIST_DIR}/TransactionPool/TransactionPoolManager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/TransactionPool/TransactionPoolManager.h
)
include_directories(
        ${CMAKE_CURRENT_LIST_DIR}/
        ${CMAKE_CURRENT_LIST_DIR}/Miner 
        ${CMAKE_CURRENT_LIST_DIR}/Blockchain
        ${CMAKE_CURRENT_LIST_DIR}/TransactionPool
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
        merklecpp
        leveldb
        concurrencpp
        readerwriterqueue
        krapi_event
        shared_models
        blockchain_models
        peer_manager
        utils
)
