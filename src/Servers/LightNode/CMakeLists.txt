project(lightnode)
set(CMAKE_CXX_STANDARD 20)

add_executable(
        lightnode
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/LightNodeManager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/LightNodeManager.h
        ${CMAKE_CURRENT_LIST_DIR}/Wallet.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Wallet.h
)

target_link_libraries(
        lightnode PRIVATE
        spdlog
        datachannel
        nlohmann_json
        cryptopp
        eventpp
        ixwebsocket
        effolkronium_random
        stduuid
        blockchain_models
        transaction_pool
        node_manager
)