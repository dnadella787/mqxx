# Dependencies

This repository currently keeps dependency policy intentionally small.

## Active dependencies

### Conan 2

Conan is the package manager and toolchain coordinator for this repository.
It is responsible for:

- dependency resolution
- CMake toolchain generation
- CMake preset generation
- propagating repository build options into configured builds

### CMake

CMake is the build system entry point.
The root `CMakeLists.txt` owns project-wide options and target assembly.

## Active test dependencies

### GoogleTest

GoogleTest is active for the default developer loop.
The Conan recipe declares `gtest` when `build_tests` is enabled, and tests are enabled by default.

The current test shape is:

- `find_package(GTest REQUIRED)` in `tests/CMakeLists.txt`
- one `mqxx_tests` executable
- `gtest_discover_tests(mqxx_tests)` for CTest registration

Disable tests only when needed with:

- `-DMQXX_BUILD_TESTS=OFF`
- `-o '&:build_tests=False'`

### Transport and protocol dependencies

No transport stack, TLS stack, or protocol helper dependency is currently active in the source
tree. Add them only when the reimplementation needs them.
