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

- protocol model code lives in named modules under `src/moqt`
- transport-facing code lives in named modules under `src/transport`
- tests are able to exercise protocol and state-management logic without touching real networking

This is no longer only a future direction.
The repository has moved its internal project code to named modules for the current milestone.

This split matters because a production relay eventually needs:

- multiple transport back ends
- deterministic tests for state machines
- parser and serializer tests that do not depend on packet timing
- the ability to evolve internal scheduling and caching without rewriting transport code

## Source layout direction with C++ modules

The repository started with headers because they were the easiest bootstrap format for a tiny
prototype.
That bootstrap layout has now been retired for core project code.

The direction from here is:

- use named modules for protocol and transport abstractions
- use small module interfaces instead of umbrella headers
- keep implementation details unexported by default
- use headers only where a module boundary is awkward or not worth the cost

The current module layout is intentionally small:

- `mqxx.moqt.full_track_name`
- `mqxx.moqt.namespace_registry`
- `mqxx.moqt.static_track_descriptor`
- `mqxx.transport.session`
- `mqxx.transport.fake_transport_session`

That structure is useful for more than style:

- it makes exported API surfaces more intentional
- it reduces accidental transitive inclusion
- it gives newcomers a clearer signal about what is public versus internal

Important constraint:

The project should adopt modules in a conservative, beginner-friendly way.
That means:

- avoid clever partition graphs early
- avoid over-abstracting around compiler quirks
- explain the module layout in the docs whenever a new module family is introduced

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

Recent refactoring direction:

- these first milestone components have been migrated into named modules
- tests now import project modules directly
- the next milestones should extend this structure instead of reintroducing internal headers

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
