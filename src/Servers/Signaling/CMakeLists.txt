project(signaling)
set(CMAKE_CXX_STANDARD 20)

set(SIGNALINGDIR ${CMAKE_CURRENT_SOURCE_DIR}/src/Servers/Signaling)

add_executable(signaling ${SIGNALINGDIR}/main.cpp)

target_link_libraries(signaling PRIVATE nlohmann_json ixwebsocket spdlog)
