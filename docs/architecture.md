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

- protocol model code lives under `include/saltpepper/moqt` and `src/moqt`
- transport-facing code lives under `include/saltpepper/transport` and `src/transport`
- tests are able to exercise protocol and state-management logic without touching real networking

This split matters because a production relay eventually needs:

- multiple transport back ends
- deterministic tests for state machines
- parser and serializer tests that do not depend on packet timing
- the ability to evolve internal scheduling and caching without rewriting transport code

## Transport direction

The long-term transport direction is native QUIC first.
The initial real QUIC target is `ngtcp2`.

Even so, relay/session logic should not be written as if `ngtcp2` were the architecture.
Instead, the code should treat `ngtcp2` as one implementation of a transport boundary.

The boundary introduced in this repository is intentionally small:

- `TransportSession` represents the operations the relay will eventually need
- `FakeTransportSession` is an in-memory implementation used by tests

This is not yet a complete transport design.
It is only enough abstraction to prevent transport-specific state from leaking into protocol code.

## Event loop direction

The concurrency model should be compatible with a Boost.Asio-style event loop.
That does not mean the code needs to be deeply coupled to Boost.Asio types on day one.

The practical goal is this:

- async work should flow through explicit interfaces
- the relay core should be usable from a single-threaded event loop first
- later milestones can add strands, executors, timers, and coroutine adapters if they are justified

This keeps the beginner story understandable:

- first understand the state machine
- then understand the event loop
- then understand concurrency

Instead of mixing all three at once.

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

This sequence matches the project goal of teaching the protocol in layers instead of dropping a
nearly complete relay skeleton into an empty repository.
