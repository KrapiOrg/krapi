project(krapi_models)
set(CMAKE_CXX_STANDARD 20)
set(MODELSPREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src/Models)

include(${MODELSPREFIX}/Message/CMakeLists.txt)
include(${MODELSPREFIX}/Response/CMakeLists.txt)
add_library(krapi_models ${MDLSOURCES} ${RSPNSSOURCES})
target_link_libraries(krapi_models PRIVATE
        nlohmann_json
        fmt
        rspns_models
        msg_models
        )