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
        cryptopp
        datachannel
        eventpp
        stduuid
        range-v3
        concurrencpp
        signaling_client
        transaction_pool
        krapi_event
        shared_models
)