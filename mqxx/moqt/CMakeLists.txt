add_library(mqxx_moqt)
target_sources(
    mqxx_moqt
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/namespace_registry.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/track_identity.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/wire.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/session.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/client.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/server.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${PROJECT_SOURCE_DIR}
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/namespace_registry.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/track_identity.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/wire.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/session.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/client.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/server.hpp)
target_link_libraries(mqxx_moqt PUBLIC mqxx_common)
mqxx_configure_target(mqxx_moqt)