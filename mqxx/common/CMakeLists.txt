add_library(mqxx_common INTERFACE)
target_compile_features(mqxx_common INTERFACE cxx_std_26)
target_include_directories(mqxx_common INTERFACE ${PROJECT_SOURCE_DIR})
