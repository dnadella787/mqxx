add_library(mqxx_transport OBJECT)
target_sources(
    mqxx_transport
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/fake_session.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${PROJECT_SOURCE_DIR}
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/fake_session.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/session.hpp)
target_link_libraries(mqxx_transport PUBLIC mqxx_common)
mqxx_configure_target(mqxx_transport)
