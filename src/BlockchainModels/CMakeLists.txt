project(blockchain_models)
set(CMAKE_CXX_STANDARD 20)

add_library(
        blockchain_models
        ${CMAKE_CURRENT_LIST_DIR}/Block.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Block.h
        ${CMAKE_CURRENT_LIST_DIR}/BlockHeader.cpp
        ${CMAKE_CURRENT_LIST_DIR}/BlockHeader.h
        ${CMAKE_CURRENT_LIST_DIR}/Transaction.h
        ${CMAKE_CURRENT_LIST_DIR}/Transaction.cpp
)
target_link_libraries(blockchain_models PRIVATE nlohmann_json cryptopp spdlog)