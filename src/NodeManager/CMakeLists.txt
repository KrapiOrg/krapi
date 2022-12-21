project(node_manager)

set(CMAKE_CXX_STANDARD 20)

add_library(
        node_manager
        ${CMAKE_CURRENT_LIST_DIR}/NodeManager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/NodeManager.h
        ${CMAKE_CURRENT_LIST_DIR}/PeerConnection.cpp
        ${CMAKE_CURRENT_LIST_DIR}/PeerConnection.h
)

target_link_libraries(
        node_manager
        PRIVATE
        nlohmann_json
        spdlog
        ixwebsocket
        cryptopp
        datachannel
        eventpp
        stduuid
        tl::expected
        concurrencpp
        signaling_client
        transaction_pool
        shared_models
)