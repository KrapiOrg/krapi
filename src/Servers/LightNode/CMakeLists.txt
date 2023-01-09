set(CMAKE_CXX_STANDARD 20)

add_executable(
        lightnode
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Wallet.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Wallet.h
)

target_link_libraries(
        lightnode PRIVATE
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