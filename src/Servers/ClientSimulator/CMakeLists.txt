cmake_minimum_required(VERSION 3.23)
project(csim)
set(CMAKE_CXX_STANDARD 20)
set(CSIM ${CMAKE_CURRENT_SOURCE_DIR}/src/Servers/ClientSimulator)
set(
        CSIMSOURCES
        ${CSIM}/main.cpp
)
add_executable(csim "${CSIMSOURCES}")
target_include_directories(csim PRIVATE Utils)
target_include_directories(csim PRIVATE src/Models)
target_link_libraries(
        csim PRIVATE
        ixwebsocket
        nlohmann_json
        spdlog
        utils
)
