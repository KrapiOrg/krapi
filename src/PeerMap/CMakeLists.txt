project(peer_map)
add_library(
        peer_map
        ${CMAKE_CURRENT_LIST_DIR}/PeerMap.cpp
        ${CMAKE_CURRENT_LIST_DIR}/PeerMap.h
)
target_link_libraries(
        peer_map PRIVATE
        datachannel
        nlohmann_json
)