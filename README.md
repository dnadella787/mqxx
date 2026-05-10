# mqxx

`mqxx` starts as a prototype Media over QUIC (MOQT) relay written in modern C++.
The repository is intentionally structured so the first milestone stays beginner-friendly while the
architecture can grow into a production-grade system over the next two to three years.

The repository is currently pinned to the IETF MOQT baseline
`draft-ietf-moq-transport-17`.

## Current milestone

This repository currently implements only the first protocol milestone:

- track namespaces
- full track names
- canonical text rendering/parsing for track names
- an in-memory namespace registry for prefix matching
- a transport abstraction plus a fake transport for tests

Objects, groups, priorities, delivery policy, real QUIC wiring, authentication, and media-specific
payload handling are not implemented yet.

## Design rules captured in this repository

- Start directly with QUIC, with `ngtcp2` as the initial real transport target.
- Keep relay logic insulated from the transport stack via an explicit transport abstraction.
- Keep the codebase application-first, but give each module its own `include/` seam under `src/`.
- Keep shared cross-module support code behind `src/common/include/...`.
- Prefer a standalone Asio-style event-loop model over early coroutine-heavy design.
- Build protocol concepts incrementally in this order:
  1. namespaces + tracks
  2. objects
  3. groups
  4. priorities and delivery policy
- Treat `draft-ietf-moq-transport-17` as the active MOQT protocol baseline until a later draft is
  adopted explicitly.
- Prefer explicit result types with structured errors over exception-based control flow.
- Keep dependencies minimal and categorized.
- Use Quill when the project introduces a real logging dependency.
- Standardize on GoogleTest when the repository migrates off the bootstrap test harness.
- Keep the docs verbose and beginner-friendly, but keep the code comments restrained.
- Require strong unit tests from the start, plus fake-transport tests that do not depend on real
  QUIC.

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Useful options:

- `-DMQXX_ENABLE_SANITIZERS=ON`
- `-DMQXX_WARNINGS_AS_ERRORS=ON`
- `-DMQXX_ENABLE_CLANG_TIDY=OFF`

`compile_commands.json` is exported automatically for editor tooling and CLion-style workflows.
The current baseline is C++26 for project targets, while the implementation still uses practical
modern features from C++20 and later where they materially help readability and correctness.

## Read first

- [Architecture Notes](docs/architecture.md)
- [Protocol Notes](docs/protocol-notes.md)
- [Dependency Plan](docs/dependencies.md)
- [Standalone Asio Decision](docs/decisions/0001-standalone-asio-over-boost-asio.md)
- [GoogleTest Decision](docs/decisions/0002-googletest-over-catch2.md)
- [Quill Logging Decision](docs/decisions/0003-quill-for-logging.md)
- [MOQT Draft 17 Pin](docs/decisions/0004-pin-moqt-baseline-to-draft-17.md)
- [Result Types Over Exceptions](docs/decisions/0005-prefer-result-types-over-exceptions.md)
- [TMP Notes](docs/tmp-notes.md)
