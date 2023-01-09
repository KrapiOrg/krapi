set(CMAKE_CXX_STANDARD 20)

add_executable(
        signaling
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/SignalingServer.cpp
        ${CMAKE_CURRENT_LIST_DIR}/SignalingServer.h
)

target_link_libraries(
        signaling
        PRIVATE
        nlohmann_json
        datachannel
        spdlog
        stduuid
)
