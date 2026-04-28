# Dependency Plan

## Philosophy

The project should keep dependencies minimal, but "minimal" does not mean "avoid every library."
Networking code, testing tools, and transport integration often justify focused dependencies if they
remove a large amount of custom infrastructure or risk.

Dependencies are grouped here so the early prototype stays understandable.

## Required core dependencies

### CMake

The repository now depends on a CMake configuration with dependable named-module support.
In practice, this means using a recent CMake release rather than treating modules as an optional
future improvement.

Used for builds, IDE integration, `compile_commands.json`, and a portable project structure.

### Compiler with strong C++26 module support

The project now builds as C++26.
The implementation also makes practical use of modern features from C++20 and later such as
`std::span`, ranges algorithms, defaulted comparisons, designated initializers, and non-type
template parameters for conservative compile-time descriptors.

Because the project now prefers named C++ modules, compiler support should be evaluated more
strictly than "does it compile ordinary C++ code?".

What matters here is practical support for:

- named modules
- module dependency scanning
- stable diagnostics around exported/imported declarations
- workable IDE integration

In practice, that means choosing a modern Clang or MSVC toolchain carefully, and testing the exact
compiler/CMake combination instead of assuming all "C++26" environments behave the same.

### Boost.Asio

This is the current decision for the eventual event-loop and async I/O integration layer.
The current repository does not link Boost yet because the first milestone does not need live I/O,
but the architecture is being shaped around Boost.Asio-style boundaries from the start.

When Boost.Asio integration arrives, the repository should be careful about module boundaries.
Third-party libraries often remain header-driven even in module-first projects.
That is acceptable; the important part is that the project's own core boundaries should still favor
named modules.

## Test-only dependencies

### Future choice: GoogleTest or Catch2

The repository currently uses a tiny built-in test harness so the first milestone can build in an
empty workspace without network access or vendoring.

That is a bootstrap choice, not a long-term framework decision.
As the codebase grows, it should adopt one dedicated C++ test framework:

- GoogleTest if the project wants the more conventional fixture-heavy style
- Catch2 if the project wants a lighter, more expression-oriented style

Either is acceptable.
The decision should be made before the first larger parser/state-machine wave lands.

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

These should stay out of the prototype until there is a concrete reason:

- fuzzing tools
- benchmarking tools
- metrics libraries
- logging frameworks
- media parsing/packaging libraries
- authentication/authorization stacks

## Tooling consequences of choosing modules

This decision affects tooling, not just source syntax.

The project should expect to validate:

- compiler support for named modules
- CMake module dependency scanning behavior
- module-compatible generator support such as Ninja
- clang-format and clang-tidy behavior around module interface units
- CLion indexing and navigation quality

This is one reason the module migration should happen in small steps instead of one large rewrite.

## Why logging is not added yet

The prototype needs observability, but not necessarily a full logging dependency.
Early milestones can use small local logging helpers or standard streams while the architecture is
still fluid.

A logging framework becomes justified only when the project has:

- multiple subsystems
- filtering/verbosity needs
- structured logging requirements
- tests that need captureable diagnostic output
