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

When transport work starts, `Boost.Corosio` is the intended async-I/O runtime for this repository.
Use it behind the `mqxx/transport/` boundary rather than introducing a direct Asio-based transport
layer. Current reasons for that choice:

- it is purpose-built for coroutine-based async I/O rather than callback-style layering
- it keeps executor affinity in the library model instead of making that a local convention
- it provides the transport runtime role this repo needs without pushing coroutine scheduling policy
  into MOQT model code

Current constraints to document when adopting it as an active dependency:

- upstream currently documents `Capy` as a required dependency alongside `corosio`
- the upstream repository currently does not publish versioned releases, so this repo should pin a
  specific commit or Conan package revision rather than track a moving branch
- transport code must still keep MOQT protocol types and relay policy transport-agnostic

So the project stance is:

- `corosio` is the planned transport runtime dependency
- it is not yet an active dependency in the source tree
- transport APIs in this repo should assume a `corosio`-backed runtime only inside the transport
  layer, never in the protocol layer
