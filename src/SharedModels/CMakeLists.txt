cmake_minimum_required(VERSION 3.23)
project(shared_models)
set(CMAKE_CXX_STANDARD 20)
set(MODELSPREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src/SharedModels)

add_library(
        shared_models
        ${MODELSPREFIX}/Message.h
        ${MODELSPREFIX}/Response.h
        ${MODELSPREFIX}/PeerMessage.h
        ${MODELSPREFIX}/PeerType.h
        ${MODELSPREFIX}/Content/SetTransactionStatusContent.h
        ${MODELSPREFIX}/Content/SyncBlockchainResponseContent.h
)
target_link_libraries(shared_models PRIVATE nlohmann_json)
set_target_properties(shared_models PROPERTIES LINKER_LANGUAGE CXX)