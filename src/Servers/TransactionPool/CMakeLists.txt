cmake_minimum_required(VERSION 3.23)
project(txpool)
set(CMAKE_CXX_STANDARD 20)
set(TXPOOLPREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src/Servers/TransactionPool)
set(TXPOOLSOURCES ${TXPOOLPREFIX}/main.cpp)
add_executable(txpool "${TXPOOLSOURCES}")
target_link_libraries(txpool PRIVATE
        shared_models
        ixwebsocket
        nlohmann_json
        fmt
        spdlog
        cryptopp
        cxxopts
        )
