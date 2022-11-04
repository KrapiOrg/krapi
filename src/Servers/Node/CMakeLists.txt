cmake_minimum_required(VERSION 3.23)
project(Node)
set(CMAKE_CXX_STANDARD 20)
set(NODEPREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src/Servers/Node)
set(
        NODESOURCES
        ${NODEPREFIX}/main.cpp
        ${NODEPREFIX}/Managers/NodeManager.cpp
        ${NODEPREFIX}/Managers/NodeManager.h
        ${NODEPREFIX}/Managers/IdentityManager.cpp
        ${NODEPREFIX}/Managers/IdentityManager.h
        ${NODEPREFIX}/Managers/NetworkConnectionManager.cpp
        ${NODEPREFIX}/Managers/NetworkConnectionManager.h
        ${NODEPREFIX}/Models/NodeMessage.h
        ${NODEPREFIX}/Http/HttpClient.cpp
        ${NODEPREFIX}/Http/HttpClient.h
        ${NODEPREFIX}/Http/HttpServer.cpp
        ${NODEPREFIX}/Http/HttpServer.h
        ${NODEPREFIX}/Http/HttpMessage.h
        ${NODEPREFIX}/WebSocket/WebSocketServer.cpp
        ${NODEPREFIX}/WebSocket/WebSocketServer.h
        ${NODEPREFIX}/Utils/MessageQueue.h
        ${NODEPREFIX}/Blockchain/Blockchain.cpp
        ${NODEPREFIX}/Blockchain/Blockchain.h
        ${NODEPREFIX}/Blockchain/Block.h
        ${NODEPREFIX}/Blockchain/BlockHeader.h
        ${NODEPREFIX}/Blockchain/TransactionPool.cpp
        ${NODEPREFIX}/Blockchain/TransactionPool.h
)
add_executable(node "${NODESOURCES}")
target_include_directories(
        node
        PRIVATE
        src/SharedModels
        ${NODEPREFIX}/Http
        ${NODEPREFIX}/Managers
        ${NODEPREFIX}/Models
        ${NODEPREFIX}/Utils
        ${NODEPREFIX}/WebSocket
        ${NODEPREFIX}/Blockchain
)
target_link_libraries(node PRIVATE
        ixwebsocket
        nlohmann_json
        fmt
        spdlog
        cxxopts
        shared_models
        eventpp
        sutils
        magic_enum
        cryptopp
        )
