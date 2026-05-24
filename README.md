# mqxx

This repository is intentionally reset to a buildable scaffold.
Implementation files and completed-work documentation were removed so new work can start from
scratch without inheriting old protocol or API decisions.

The repository currently preserves:

- Conan 2 package management
- CMake project structure
- compiler warnings and sanitizer wiring under `cmake/`
- module directories under `mqxx/`
- a `tests/` directory reserved for GoogleTest-based coverage

Read these first:

- [docs/build-setup.md](docs/build-setup.md)
- [docs/repository-layout.md](docs/repository-layout.md)

## Current build state

- `mqxx_core` is a scaffold target only
- module targets exist, but no public headers or source files are checked in
- `tests/CMakeLists.txt` currently skips test target creation because no test sources exist yet

## Quick start

Generate Conan files and CMake presets:

```bash
conan profile detect --force
conan install . --output-folder=out/conan/llvm-debug/build/Debug --build=missing \
  -pr:h=profiles/macos-homebrew-llvm-ninja-debug \
  -pr:b=default
```

Configure and build with the generated preset:

```bash
cmake --preset conan-debug
cmake --build --preset conan-debug
```

Enable tests later by setting `MQXX_BUILD_TESTS=ON` in CMake and `-o '&:build_tests=True'` in
Conan once test sources exist again.
