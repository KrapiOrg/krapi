cmake_minimum_required(VERSION 3.23)
project(rspns_models)
set(CMAKE_CXX_STANDARD 20)
set(RSPNSMDLSPRFX ${CMAKE_CURRENT_SOURCE_DIR}/src/Models/Response)
set(
        RSPNSSOURCES
        ${RSPNSMDLSPRFX}/Response.h
        ${RSPNSMDLSPRFX}/ResponseBase.h
        ${RSPNSMDLSPRFX}/ResponseBase.cpp
        ${RSPNSMDLSPRFX}/TxPoolDiscoveryRsp.h
        ${RSPNSMDLSPRFX}/ErrorRsp.h
        ${RSPNSMDLSPRFX}/JsonToResponseConverter.h
        ${RSPNSMDLSPRFX}/NodeDiscoveryRsp.h
)
add_library(rspns_models ${RSPNSSOURCES})
target_link_libraries(rspns_models fmt nlohmann_json)