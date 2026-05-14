add_library(mqxx_moqt OBJECT)
target_sources(
    mqxx_moqt
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/full_track_name.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/namespace_registry.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${PROJECT_SOURCE_DIR}
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/full_track_name.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/namespace_registry.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/relay_seams.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/session_roles.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/session_types.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/static_track_descriptor.hpp)
target_link_libraries(mqxx_moqt PUBLIC mqxx_common)
mqxx_configure_target(mqxx_moqt)
