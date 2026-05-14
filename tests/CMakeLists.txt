add_executable(mqxx_tests)
target_sources(
    mqxx_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/moqt_name_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/moqt_static_track_descriptor_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/moqt_namespace_registry_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/moqt_session_roles_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/transport_fake_session_tests.cpp)
target_link_libraries(mqxx_tests PRIVATE mqxx_core)
target_include_directories(mqxx_tests PRIVATE ${PROJECT_SOURCE_DIR}/tests)
mqxx_configure_target(mqxx_tests)

add_test(NAME mqxx_tests COMMAND mqxx_tests)
