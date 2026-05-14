# Dependency Plan

## Philosophy

The project should keep dependencies minimal, but "minimal" does not mean "avoid every library."
Networking code, testing tools, and transport integration often justify focused dependencies if
they remove a large amount of custom infrastructure or risk.

Dependencies are grouped here so the repo stays easy to reason about while it grows toward fuller
MOQT library and relay behavior.
The short repository-wide defaults are summarized in
[project-decisions.md](project-decisions.md).

## Required core dependencies

### CMake

Used for builds, IDE integration, `compile_commands.json`, and a portable project structure.

### Compiler with practical C++26 support

The project builds as C++26.
The implementation also makes practical use of modern features from C++20 and later such as
`std::span`, ranges algorithms, defaulted comparisons, designated initializers, and non-type
template parameters for conservative compile-time descriptors.

In practice, that means choosing a modern Clang or MSVC toolchain carefully and testing the exact
compiler/CMake combination instead of assuming all "C++26" environments behave the same.

### standalone Asio

This is the current decision for the eventual event-loop and async I/O integration layer.
The current repository does not link Asio yet, but the architecture is being shaped around
standalone Asio-style boundaries from the start.

## Test-only dependencies

### GoogleTest

GoogleTest is the accepted long-term unit-test framework for this repository.
The repository currently still uses a tiny built-in test harness so the current code can build in
an empty workspace without network access or vendoring.

That built-in harness is now a bootstrap-only choice, not an open framework decision.
When the project starts pulling third-party dependencies for regular development, tests should move
to GoogleTest rather than reopening the framework choice.

GoogleTest fits the repository better because it gives:

- a conventional and widely understood fixture model
- broad ecosystem familiarity for C++ contributors
- stable parameterized-test and matcher support as protocol/state-machine coverage grows
- an easier path for future CI and IDE integration than maintaining a custom harness

## Optional transport dependencies

### ngtcp2

This is the planned first real QUIC stack.
It should remain behind a transport abstraction so it does not leak into protocol state types or
relay policy code.

### Compatible TLS library

`ngtcp2` requires a TLS stack.
The exact choice should be driven by build simplicity, platform packaging, and integration quality.

### nghttp3

This is not needed for the current direct-QUIC milestone.
It becomes relevant only if HTTP/3 or WebTransport support is introduced later.

## Future dependencies

These should stay out of the repository until there is a concrete reason:

- fuzzing tools
- benchmarking tools
- metrics libraries
- media parsing/packaging libraries
- authentication/authorization stacks

## Operational dependencies

### Quill

Quill is the accepted logging library for this repository once real logging is wired into the
build.
The current codebase does not link it yet, but the dependency choice itself is no longer deferred.

Quill is a good fit here because the project is expected to grow into a transport-heavy relay where
logging overhead, thread handoff behavior, and filtered diagnostics matter.

The expected use is:

- subsystem-oriented logs for transport, protocol, and relay behavior
- filtered verbosity levels for local debugging and integration testing
- captureable diagnostics during test and bring-up work
- structured-enough output to support future operational tooling without inventing a custom logger
