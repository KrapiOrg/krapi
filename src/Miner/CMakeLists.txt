project(miner)
set(CMAKE_CXX_STANDARD 20)

add_library(
        miner
        ${CMAKE_CURRENT_LIST_DIR}/Miner.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Miner.h
)
target_link_libraries(
        miner PRIVATE
        merklecpp
        spdlog
        blockchain_models
        cryptopp
        nlohmann_json
        eventpp
)