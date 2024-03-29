cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 20)

add_library(
        shared_models
        ${CMAKE_CURRENT_LIST_DIR}/SignalingMessage.h
        ${CMAKE_CURRENT_LIST_DIR}/PeerMessage.h
        ${CMAKE_CURRENT_LIST_DIR}/InternalMessage.h
        ${CMAKE_CURRENT_LIST_DIR}/PeerType.h
        ${CMAKE_CURRENT_LIST_DIR}/Content/SetTransactionStatusContent.h
        ${CMAKE_CURRENT_LIST_DIR}/Content/BlocksResponseContent.h
        ${CMAKE_CURRENT_LIST_DIR}/Content/BlockHeadersResponseContent.h
        ${CMAKE_CURRENT_LIST_DIR}/PeerState.h
        ${CMAKE_CURRENT_LIST_DIR}/Content/PoolResponseContent.h
        ${CMAKE_CURRENT_LIST_DIR}/InternalNotification.h
        ${CMAKE_CURRENT_LIST_DIR}/DBInterface.h
        ${CMAKE_CURRENT_LIST_DIR}/ValidationState.h
        ${CMAKE_CURRENT_LIST_DIR}/Concepts.h
)
target_link_libraries(shared_models nlohmann_json stduuid leveldb concurrencpp)
set_target_properties(shared_models PROPERTIES LINKER_LANGUAGE CXX)