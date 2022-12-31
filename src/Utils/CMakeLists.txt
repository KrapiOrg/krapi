set(CMAKE_CXX_STANDARD 20)
set(UTILSPREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src/Utils)

add_library(
        utils
        ${UTILSPREFIX}/Box.h
        ${UTILSPREFIX}/Overload.h
        ${UTILSPREFIX}/AsyncQueue.h
        ${UTILSPREFIX}/NotNull.h
        ${UTILSPREFIX}/Assert.h
)
target_link_libraries(utils PRIVATE nlohmann_json)
set_target_properties(utils PROPERTIES LINKER_LANGUAGE CXX)
