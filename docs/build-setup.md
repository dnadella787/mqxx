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
  Includes the Conan-generated preset file after `conan install`.

## CMake options

Defined in the root `CMakeLists.txt`:

- `MQXX_BUILD_TESTS`
- `MQXX_ENABLE_CLANG_TIDY`
- `MQXX_ENABLE_SANITIZERS`
- `MQXX_WARNINGS_AS_ERRORS`

Conan forwards these through `conanfile.py` into the generated toolchain cache variables.
Tests are disabled by default in the scaffold because no test sources are checked in.

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

The generated preset currently lives at:

- `out/conan/llvm-debug/build/Debug/generators/CMakePresets.json`

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

## Reintroducing source code

When you begin reimplementation:

1. Add headers and sources under the module directories in `mqxx/`.
2. Update the module-local `CMakeLists.txt` files to list those files.
3. If a module gains compiled sources, convert its target from `INTERFACE` to a compiled library
   type that fits the design.
4. If `mqxx_core` should again produce a real library artifact, convert it from `INTERFACE` to a
   compiled target once there are concrete sources or object libraries to aggregate.
5. Add test sources under `tests/` and replace the temporary skip message in `tests/CMakeLists.txt`
   with the real GoogleTest target definition.
