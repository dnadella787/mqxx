# Architecture Notes

## Why this repository is structured the way it is

This project is meant to teach as much as it builds.
That creates a tension:

- a prototype should be small
- a relay architecture should not paint itself into a corner

The solution used here is to keep each milestone narrow, but to choose boundaries that would still
make sense in a larger system.

## Current architectural slice

The current codebase implements only the first milestone: namespaces and tracks.
That is enough to establish several important boundaries early:

- public protocol model declarations live under `include/mqxx/moqt`
- public transport-facing declarations live under `include/mqxx/transport`
- behavior-heavy implementation lives in `src/`
- tests exercise protocol and state-management logic without touching real networking

This split matters because a production relay eventually needs:

- multiple transport back ends
- deterministic tests for state machines
- parser and serializer tests that do not depend on packet timing
- the ability to evolve internal scheduling and caching without rewriting transport code

## Source layout direction

The repository now uses a conventional include-directory layout:

- public headers in `include/mqxx/...`
- implementation files in `src/...`
- test-only helpers in `tests/`

That structure is intentionally small and boring.
It gives the project:

- explicit public include paths
- straightforward build-system behavior across toolchains
- implementation files that can change without rewriting the public surface

## Transport direction

The long-term transport direction is native QUIC first.
The initial real QUIC target is `ngtcp2`.

Even so, relay/session logic should not be written as if `ngtcp2` were the architecture.
Instead, the code should treat `ngtcp2` as one implementation of a transport boundary.

The boundary introduced in this repository is intentionally small:

- `transport_session` represents the operations the relay will eventually need
- `fake_transport_session` is an in-memory implementation used by tests

This is not yet a complete transport design.
It is only enough abstraction to prevent transport-specific state from leaking into protocol code.

## Event loop direction

The concurrency model should be compatible with a standalone Asio-style event loop.
That does not mean the code needs to be deeply coupled to Asio types on day one.

The practical goal is this:

- async work should flow through explicit interfaces
- the relay core should be usable from a single-threaded event loop first
- later milestones can add strands, executors, timers, and coroutine adapters if they are justified

## Testing strategy baked into the layout

The repository starts with tests because protocol work becomes brittle very quickly if the only
validation path is an end-to-end network run.

The intended testing pyramid is:

- parser/serializer unit tests first
- state-machine tests second
- fake-transport tests third
- real transport integration tests once QUIC is connected

That is why the first code focuses on full track names and namespace matching.
These are protocol-shaped concepts that are easy to specify, easy to test, and foundational for
later session and relay logic.

## Milestone roadmap

### Milestone 1: Namespaces and Tracks

Implemented now:

- canonical full track name rendering/parsing
- prefix matching for namespace discovery
- conservative compile-time track descriptors for developer-defined schemas
- fake transport infrastructure for future session tests

Not implemented yet:

- wire-format control messages
- real relay sessions
- QUIC handshake/session establishment

### Milestone 2: Objects

The next milestone should add:

- object identifiers and object metadata types
- parser/serializer logic for object-oriented data structures
- state handling for known / unknown / does-not-exist object states
- tests that cover malformed input and boundary conditions heavily

### Milestone 3: Groups

This stage should add:

- group IDs
- group ordering rules
- group-level state machines
- later subgroup support where it is needed for stream mapping

### Milestone 4: Priorities and Delivery Policy

Only after the earlier milestones are solid should the project add:

- subscriber priority
- publisher priority
- group order interactions
- delivery policy and scheduler logic
