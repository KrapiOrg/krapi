cmake_minimum_required(VERSION 3.23)
project(discovery)
set(CMAKE_CXX_STANDARD 20)
set(DISCOVERYPREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src/Servers/Discovery)
set(DISCOVERYSOURCES ${DISCOVERYPREFIX}/main.cpp)
add_executable(discovery "${DISCOVERYSOURCES}")
target_link_libraries(discovery PRIVATE
        shared_models
        nlohmann_json
        spdlog
        cxxopts
        httplib
        )
