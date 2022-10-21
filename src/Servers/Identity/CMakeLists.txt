cmake_minimum_required(VERSION 3.23)
project(identity)
set(CMAKE_CXX_STANDARD 20)
set(IDENTITYPREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src/Servers/Identity)
set(IDENTITYSOURCES ${IDENTITYPREFIX}/main.cpp)
add_executable(identity "${IDENTITYSOURCES}")
target_link_libraries(identity PRIVATE
        shared_models
        nlohmann_json
        spdlog
        cxxopts
        ixwebsocket
        )
