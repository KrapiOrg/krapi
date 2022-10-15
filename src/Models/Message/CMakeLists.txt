cmake_minimum_required(VERSION 3.23)
project(msg_models)
set(CMAKE_CXX_STANDARD 20)
set(MSGMDLSPRFX ${CMAKE_CURRENT_SOURCE_DIR}/src/Models/Message)
set(
        MDLSOURCES
        ${MSGMDLSPRFX}/AckMsg.h
        ${MSGMDLSPRFX}/CreateTxMsg.h
        ${MSGMDLSPRFX}/DiscoverTxPoolsMsg.h
        ${MSGMDLSPRFX}/JsonToMessageConverter.h
        ${MSGMDLSPRFX}/Message.h
        ${MSGMDLSPRFX}/MessageBase.h
        ${MSGMDLSPRFX}/MessageBase.cpp
)
add_library(msg_models ${MDLSOURCES})
target_link_libraries(msg_models fmt nlohmann_json)