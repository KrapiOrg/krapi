add_library(
        transaction_pool
        ${CMAKE_CURRENT_LIST_DIR}/TransactionPool.cpp
        ${CMAKE_CURRENT_LIST_DIR}/TransactionPool.h
)
target_link_libraries(
        transaction_pool PRIVATE
        datachannel
        nlohmann_json
        eventpp
        cryptopp
        spdlog
        blockchain_models
)