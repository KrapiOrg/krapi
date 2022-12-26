cmake_minimum_required(VERSION 3.23)

add_library(
        krapi_event
        STATIC
        ${CMAKE_CURRENT_LIST_DIR}/Event.h
        ${CMAKE_CURRENT_LIST_DIR}/EventQueue.h
)
target_link_libraries(
        krapi_event
        concurrencpp
        eventpp
        shared_models
        utils
)
