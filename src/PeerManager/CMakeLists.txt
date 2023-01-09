set(CMAKE_CXX_STANDARD 20)

add_library(
        peer_manager
        ${CMAKE_CURRENT_LIST_DIR}/PeerManager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/PeerManager.h
        ${CMAKE_CURRENT_LIST_DIR}/PeerConnection.cpp
        ${CMAKE_CURRENT_LIST_DIR}/PeerConnection.h
)

target_link_libraries(
        peer_manager
        PRIVATE
        nlohmann_json
        spdlog
        cryptopp
        datachannel
        eventpp
        stduuid
        concurrencpp
        signaling_client
        krapi_event
        shared_models
)