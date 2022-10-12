project(krapi_models)
set(CMAKE_CXX_STANDARD 20)
set(MODELSPREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src/Models)
add_library(
        krapi_models
        ${MODELSPREFIX}/Block.h
        ${MODELSPREFIX}/Block.cpp
        ${MODELSPREFIX}/Transaction.h
        ${MODELSPREFIX}/Transaction.cpp
        ${MODELSPREFIX}/BlockChain.h
        ${MODELSPREFIX}/BlockChain.cpp
)
target_link_libraries(krapi_models PRIVATE
        nlohmann_json
        fmt
        )