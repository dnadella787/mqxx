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

## Deferred dependencies

These are not currently required by the scaffold itself, but the build layout expects them to be
the first places to plug back in when implementation resumes.

### GoogleTest

The repository layout reserves `tests/` for GoogleTest-based tests.
No test target is created yet because no test source files are checked in.
When tests are added back:

- declare `gtest` in `conanfile.py`
- restore the `find_package(GTest REQUIRED)` path in `tests/CMakeLists.txt`
- define the test executable and register it with `gtest_discover_tests`

### Transport and protocol dependencies

No transport stack, TLS stack, or protocol helper dependency is currently active in the source
tree. Add them only when the reimplementation needs them.
