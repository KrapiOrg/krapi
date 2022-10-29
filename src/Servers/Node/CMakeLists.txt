cmake_minimum_required(VERSION 3.23)
project(Node)
set(CMAKE_CXX_STANDARD 20)
set(NODEPREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src/Servers/Node)
set(
        NODESOURCES
        ${NODEPREFIX}/main.cpp
        ${NODEPREFIX}/NodeManager.cpp
        ${NODEPREFIX}/NodeManager.h
        ${NODEPREFIX}/NetworkConnection.cpp
        ${NODEPREFIX}/NetworkConnection.h
        ${NODEPREFIX}/NodeMessage.h
        ${NODEPREFIX}/HttpClient.cpp
        ${NODEPREFIX}/HttpClient.h
        ${NODEPREFIX}/WebSocketServer.cpp
        ${NODEPREFIX}/WebSocketServer.h
        ${NODEPREFIX}/IdentityManager.cpp
        ${NODEPREFIX}/IdentityManager.h
        ${NODEPREFIX}/MessageQueue.h
        ${NODEPREFIX}/HttpServer.cpp
        ${NODEPREFIX}/HttpServer.h
        ${NODEPREFIX}/HttpMessage.h
        ${NODEPREFIX}/TransactionQueue.h
)

add_executable(node "${NODESOURCES}")
target_include_directories(node PRIVATE Utils)
target_include_directories(node PRIVATE src/Models)
target_link_libraries(node PRIVATE
        ixwebsocket
        httplib
        nlohmann_json
        fmt
        spdlog
        cxxopts
        shared_models
        eventpp
        sutils
        )
