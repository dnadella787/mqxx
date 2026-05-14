# mqxx

`mqxx` is a C++ Media over QUIC (MOQT) relay, publisher, and subscriber implementation that is
being rebased toward
behavioral parity with Moxygen's core runtime model.
The goal is not just similar names or a roughly comparable API surface.
The goal is parity in how MOQT sessions, publisher/subscriber roles, relay forwarding, cache seams,
and non-media sample flows behave.

The repository is currently pinned to the IETF MOQT baseline
`draft-ietf-moq-transport-18`.

## Behavioral parity scope

Parity target now:

- MOQT session and control-plane behavior
- symmetric publisher and subscriber runtime seams
- relay forwarding and cache composition seams
- non-media sample application behavior

Explicitly deferred for later phases:

- FLV and media-packaging tooling
- browser and player workflows
- `moq-encoder-player`-style integration

## Current implementation slice

The repository is still early in that plan.
What exists today is the start of the parity-oriented surface:

- full track name and namespace utilities
- deterministic text rendering/parsing for track names
- an in-memory `namespace_registry` helper for discovery-oriented logic
- internal role-based MOQT interface types for publishers, subscribers, handles, and consumers
- relay/cache seam interfaces
- a richer fake session boundary for control, uni-stream, datagram, reset, shutdown, and delivery
  events

Real control-plane state machines, object/group delivery logic, relay forwarding behavior, and
native transport adapters are still ahead.

## Design rules captured in this repository

- Keep protocol logic transport-agnostic and map it back to draft-18 explicitly.
- Treat Moxygen-level runtime behavior as the north star for the MOQT runtime model.
- Keep the primary runtime seams role-based: publishers, subscribers, handles, and consumers.
- Demote `namespace_registry` from "main abstraction" to a helper used inside discovery and relay
  logic.
- Keep relay logic insulated from transport adapters behind a session boundary.
- Design transport so native QUIC and WebTransport-oriented adapters are both first-class targets.
- Prefer explicit result types with structured errors over exception-based control flow.
- Keep the code under the top-level `mqxx/` tree, following the Moxygen-style single-subtree
  layout rather than a split header/export structure.
- Keep implementation `.cpp` files alongside the rest of the code under `mqxx/`, rather than
  splitting the standalone relay into a separate `src/` tree.
- Prefer a standalone Asio-style event-loop model over early coroutine-heavy design.
- Keep dependencies minimal and categorized.
- Use Quill when the project introduces a real logging dependency.
- Standardize on GoogleTest when the repository migrates off the bootstrap test harness.
- Keep the docs verbose and beginner-friendly, but keep the code comments restrained.
- Require strong unit tests, fake-session tests, and relay-behavior tests before leaning on real
  networking.

## Build stages

The roadmap is now:

1. protocol/core types retained and expanded
2. MOQT session/control-plane state machines
3. symmetric publisher/subscriber runtime seams
4. object/group delivery consumer APIs
5. relay and cache composition layer
6. fake-session behavioral test harness
7. native QUIC adapter and WebTransport-oriented adapter
8. sample publisher, subscriber, and relay applications

The current repository sits between stages 1, 3, 5, and 6:
the interfaces and fake-session seams exist, but the protocol behavior behind them is still
incomplete.

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

The CMake layout follows the source tree:

- the root `CMakeLists.txt` owns project-wide options, warning policy, sanitizer wiring, and
  target assembly
- each module under `mqxx/` owns its own sources and public headers through a local
  `CMakeLists.txt`
- `tests/CMakeLists.txt` owns the test executable separately from the library modules

That keeps the build structure aligned with the directory structure, so adding a new module usually
means adding a new subdirectory plus a small local `CMakeLists.txt` instead of growing the root
file indefinitely.

`compile_commands.json` is exported automatically for editor tooling and CLion-style workflows.
The current baseline is C++26 for project targets, while the implementation still uses practical
modern features from C++20 and later where they materially help readability and correctness.

## Read first

- [Beginner Guide](docs/beginner-guide.md)
- [Architecture Notes](docs/architecture.md)
- [Protocol Notes](docs/protocol-notes.md)
- [Project Decisions](docs/project-decisions.md)
- [Dependency Plan](docs/dependencies.md)
- [Code Explanation](docs/code-explanation.md)
