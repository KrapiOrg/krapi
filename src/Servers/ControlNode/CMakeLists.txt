set(CMAKE_CXX_STANDARD 20)

add_executable(
        control_node
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

target_link_libraries(
        control_node PRIVATE
        spdlog
        datachannel
        nlohmann_json
        eventpp
        stduuid
        cryptopp
        concurrencpp
        krapi_event
        blockchain_models
        peer_manager
)