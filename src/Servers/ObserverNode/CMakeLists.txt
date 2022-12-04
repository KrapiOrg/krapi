project(observer_node)
set(CMAKE_CXX_STANDARD 20)

add_executable(
        observer_node
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

target_link_libraries(
        observer_node
        nlohmann_json
        spdlog
        expected
        ixwebsocket
        datachannel
        eventpp
        stduuid
        cryptopp
        node_manager
        shared_models
)