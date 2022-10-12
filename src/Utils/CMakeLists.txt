project(utils)
set(CMAKE_CXX_STANDARD 20)
set(UTILSPREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src/Utils)

add_library(utils
        ${UTILSPREFIX}/Box.h
        ${UTILSPREFIX}/ErrorOr.h
        )
set_target_properties(utils PROPERTIES LINKER_LANGUAGE CXX)
