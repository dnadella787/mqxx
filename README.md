# mqxx

This repository is intentionally minimal, but it now has a working Conan/CMake test loop and the
first shared byte-view utility seam. Most implementation is still deferred so new protocol and
transport work can be built up deliberately.

The repository currently preserves:

- Conan 2 package management
- CMake project structure
- compiler warnings and sanitizer wiring under `cmake/`
- a dedicated `mqxx/` code subtree, following the Moxygen-style project layout pattern
- a `tests/` directory reserved for GoogleTest-based coverage

Read these first:

- [docs/build-setup.md](docs/build-setup.md)
- [docs/repository-layout.md](docs/repository-layout.md)

## Current build state

- `mqxx_core` is still a scaffold aggregation target only
- `mqxx/common/byte_buffer.hpp` is the first shared header in the explicit `mqxx/` subtree
- includes are kept in `mqxx/...` form inside the project, but the repo is currently standalone
  rather than packaged for external consumers
- `mqxx_tests` currently covers the common byte-view seam

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

Run tests with:

```bash
ctest --preset conan-debug --output-on-failure
```

Disable tests only when needed with `-DMQXX_BUILD_TESTS=OFF` in CMake or
`-o '&:build_tests=False'` in Conan.
