cmake_minimum_required(VERSION 3.23)
project(krapi)

set(CMAKE_CXX_STANDARD 20)

# set up libraries
set(JSON_USE_IMPLICIT_CONVERSIONS OFF)
add_subdirectory(libs/json)
add_subdirectory(libs/fmt)
set(SPDLOG_FMT_EXTERNAL ON)
add_subdirectory(libs/spdlog)
set(CRYPTOPP_USE_MASTER_BRANCH TRUE)
add_subdirectory(libs/cryptopp-cmake)
add_subdirectory(libs/cxxopts)
add_subdirectory(libs/eventpp)
add_subdirectory(libs/IXWebSocket)
set(NO_MEDIA ON)
set(NO_EXAMPLES ON)
set(NO_TESTS ON)
set(NO_WEBSOCKET ON)
add_subdirectory(libs/libdatachannel)
set(MERKLECPP_TRACE_ENABLED ON)
add_subdirectory(libs/merklecpp)
add_subdirectory(test)

# General Includes
include_directories(src/Utils)
include_directories(src/Servers/ServerUtils)
include_directories(src/SharedModels)

include(src/Utils/CMakeLists.txt)
include(src/Servers/Signaling/CMakeLists.txt)
include(src/Servers/Node/CMakeLists.txt)
include(src/SharedModels/CMakeLists.txt)