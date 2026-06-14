add_library(mqxx_moqt)
target_sources(
    mqxx_moqt
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/namespace_registry.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/track_identity.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${PROJECT_SOURCE_DIR}
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/namespace_registry.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/track_identity.hpp)
target_link_libraries(mqxx_moqt PUBLIC mqxx_common)
mqxx_configure_target(mqxx_moqt)
