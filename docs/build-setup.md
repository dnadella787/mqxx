# Build Setup

## Tooling

- CMake minimum version: `3.30`
- Language level: `C++26`
- Package manager: `Conan 2`
- Default generator in the checked-in profile: `Ninja`

## Files that define the build

- `CMakeLists.txt`
  Owns project options, shared target configuration, and high-level target assembly.
- `cmake/CompilerWarnings.cmake`
  Central warning policy.
- `cmake/Sanitizers.cmake`
  Address/UB sanitizer wiring for supported builds.
- `conanfile.py`
  Maps Conan options onto the CMake options and generates the toolchain and presets.
- `profiles/macos-homebrew-llvm-ninja-debug`
  Example host profile for local macOS/Homebrew LLVM debug builds.
- `CMakeUserPresets.json`
  Generated at the repo root by Conan after `conan install`.

## CMake options

Defined in the root `CMakeLists.txt`:

- `MQXX_BUILD_TESTS`
- `MQXX_ENABLE_CLANG_TIDY`
- `MQXX_ENABLE_SANITIZERS`
- `MQXX_WARNINGS_AS_ERRORS`

Conan forwards these through `conanfile.py` into the generated toolchain cache variables.
Tests are enabled by default.

## Local build flow

Install dependencies and generate the preset/toolchain files:

```bash
conan install . --output-folder=out/conan/llvm-debug/build/Debug --build=missing \
  -pr:h=profiles/macos-homebrew-llvm-ninja-debug \
  -pr:b=default
```

Then configure and build:

```bash
cmake --preset conan-debug
cmake --build --preset conan-debug
```

Run tests with:

```bash
ctest --preset conan-debug --output-on-failure
```

The active generated preset currently lives at:

- `out/conan/llvm-debug/build/Debug/build/Debug/generators/CMakePresets.json`

The root `CMakeUserPresets.json` includes that generated file so CLion and plain CMake can see
the `conan-debug` preset from the repository root.

## CLion setup

Use the `conan-debug` CMake preset, not a standalone `cmake-build-*` profile.

Recommended flow:

1. Run `conan install ...` first.
2. Reload the project in CLion.
3. Open `Settings | Build, Execution, Deployment | CMake`.
4. Select the imported `conan-debug` profile.
5. Remove or disable old non-preset profiles that point at `cmake-build-*`.

If CLion does not show `conan-debug`, verify that `CMakeUserPresets.json` exists at the repo root
and that the included generated preset file still exists under `out/conan/.../generators/`.
If stale generated preset files cause a duplicate `conan-debug` entry, delete the generated
`CMakeUserPresets.json`, rerun `conan install`, and let Conan recreate it.

## Project header pattern

This repository follows the Moxygen-style pattern of keeping project headers under the dedicated
top-level `mqxx/` subtree instead of treating the repo root as the code surface.

That means:

- in-tree includes use `#include "mqxx/..."`
- code stays under `mqxx/` rather than being spread across top-level directories
- the current goal is a standalone project, not an externally packaged library

## Reintroducing source code

When you begin reimplementation:

1. Add headers and sources under the module directories in `mqxx/`.
2. Update the module-local `CMakeLists.txt` files to list those files.
3. If a module gains compiled sources, convert its target from `INTERFACE` to a compiled library
   type that fits the design.
4. If `mqxx_core` should again produce a real library artifact, convert it from `INTERFACE` to a
   compiled target once there are concrete sources or object libraries to aggregate.
5. Add test sources under `tests/` and extend `mqxx_tests` rather than creating unrelated test
   executables without a reason.
