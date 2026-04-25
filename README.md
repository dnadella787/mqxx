# saltpepper

`saltpepper` starts as a prototype Media over QUIC (MOQT) relay written in modern C++.
The repository is intentionally structured so the first milestone stays beginner-friendly,
while the architecture can grow into a production-grade system over the next two to three years.

## Current milestone

This repository currently implements only the first protocol milestone:

- track namespaces
- full track names
- canonical text rendering/parsing for track names
- an in-memory namespace registry for prefix matching
- a transport abstraction plus a fake transport for tests

Objects, groups, priorities, delivery policy, real QUIC wiring, authentication, and media-specific
payload handling are not implemented yet.

That limitation is deliberate. The project should never pretend incomplete protocol support is
complete support.

## Design rules captured in this repository

- Start directly with QUIC, with `ngtcp2` as the initial real transport target.
- Keep relay logic insulated from the transport stack via an explicit transport abstraction.
- Prefer a Boost.Asio-style event-loop model over early coroutine-heavy design.
- Build protocol concepts incrementally in this order:
  1. namespaces + tracks
  2. objects
  3. groups
  4. priorities and delivery policy
- Keep dependencies minimal and categorized.
- Keep the docs verbose and beginner-friendly, but keep the code comments restrained.
- Require strong unit tests from the start, plus fake-transport tests that do not depend on real QUIC.

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Useful options:

- `-DSALTPEPPER_ENABLE_SANITIZERS=ON`
- `-DSALTPEPPER_WARNINGS_AS_ERRORS=ON`
- `-DSALTPEPPER_ENABLE_CLANG_TIDY=OFF`

`compile_commands.json` is exported automatically for editor tooling and CLion-style workflows.

## Read first

- [Architecture Notes](docs/architecture.md)
- [Protocol Notes](docs/protocol-notes.md)
- [Dependency Plan](docs/dependencies.md)
- [Boost.Asio Decision](docs/decisions/0001-boost-asio-over-standalone-asio.md)
- [TMP Notes](docs/tmp-notes.md)
