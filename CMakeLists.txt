cmake_minimum_required(VERSION 3.23)
project(krapi)

set(CMAKE_CXX_STANDARD 20)

# set up libraries
set(USE_TLS 1)
add_subdirectory(libs/websockets)
add_subdirectory(libs/json)
add_subdirectory(libs/fmt)
set(SPDLOG_FMT_EXTERNAL ON)
add_subdirectory(libs/spdlog)
add_subdirectory(libs/cryptopp)
add_subdirectory(libs/cxxopts)
add_subdirectory(test)

# General Includes
include_directories(src/Utils)
include_directories(src/Servers/ServerUtils)
include_directories(src/Models)

include(src/Utils/CMakeLists.txt)
include(src/Servers/ServerUtils/CMakeLists.txt)
include(src/Servers/Discovery/CMakeLists.txt)
include(src/Servers/TransactionPool/CMakeLists.txt)
include(src/Servers/Node/CMakeLists.txt)
include(src/Servers/ClientSimulator/CMakeLists.txt)
include(src/Models/CMakeLists.txt)