# Architecture Notes

## Architectural target

This repository is no longer framed as a names-and-tracks teaching skeleton that might someday
grow into a relay.
The current architectural target is a C++ standalone MOQT relay, publisher, and subscriber
implementation that moves toward
Moxygen-level behavioral parity for core runtime flows.

That means the architecture should be evaluated against questions like:

- can a MOQT session/control implementation express the same behaviors cleanly?
- can publisher and subscriber roles be modeled symmetrically?
- can a relay act as both subscriber and publisher without transport leakage?
- can cache and forwarding policy be inserted without rewriting the protocol core?

The project is still intentionally incremental, but the increments now need to line up with that
target rather than with a narrow track-naming prototype.

The protocol baseline remains pinned to `draft-ietf-moq-transport-18`.
Behavioral parity means parity with a known runtime model while still mapping back to the pinned
draft.

## Current architectural slice

The current code establishes the public and testing seams for that direction:

- public headers and implementation files live together under the top-level `mqxx/...` tree
- tests exercise deterministic logic without depending on real networking

What is present now:

- full track name rendering/parsing
- namespace prefix matching
- `namespace_registry` as a helper for discovery-oriented state
- `publisher`, `subscriber`, handle, and consumer interfaces
- relay/cache seam interfaces
- a richer session boundary plus an in-memory fake session for tests

What is still missing:

- explicit MOQT setup/setup-response state machines
- subscribe, subscribe-update, fetch, and publish-namespace control behavior
- object and group delivery runtime behavior
- relay forwarding and aggregation logic
- native QUIC and WebTransport-oriented adapters

## Runtime seam direction

The main internal runtime contract should now be role-based rather than transport-first.
The runtime and future sample applications should build against:

- `publisher`
- `subscriber`
- `subscription_handle`
- `fetch_handle`
- `publish_namespace_handle`
- `track_consumer`
- `subgroup_consumer`
- `fetch_consumer`

Helper types such as `full_track_name`, `track_namespace`, and result types remain important,
but they are support types for the role-based runtime model.
`namespace_registry` stays in the codebase, though it is now a helper rather than the center of
the architecture.

## Relay composition

A relay should compose around the same public interfaces it exposes to applications.
The architectural intent is:

- upstream behavior is expressed through `subscriber`
- downstream behavior is expressed through `publisher`
- a relay type can implement both via `relay_endpoint`
- cache and policy hooks live beside that composition rather than inside the transport layer

The current relay seams are intentionally simple:

- `relay_object_cache` for object retention and lookup
- `subscription_aggregator` for fan-in tracking
- `forwarding_policy` for cache-vs-upstream decisions

Those seams are deliberately early because relay behavior tends to get tangled if caching and
forwarding are introduced after the transport adapter already owns too much policy.

## Session boundary direction

The previous minimal transport abstraction was enough to prove that fake tests were possible,
but it was too narrow for parity-oriented MOQT behavior.

The session boundary now needs to cover:

- outbound control messages
- inbound control messages
- uni-stream open and write operations
- inbound uni-stream data
- datagram send and receive
- stream reset signals
- peer/session shutdown signals
- flow-control notifications
- delivery notifications

That still does not make the session layer the primary runtime seam.
It is lower-level infrastructure that transport adapters implement so protocol state machines can
stay transport-agnostic.

The long-term transport direction remains dual-track:

- native QUIC, initially with `ngtcp2`
- a WebTransport-oriented adapter for browser-adjacent deployments

Neither should define the MOQT core model.

## Event loop and error handling

The runtime should remain compatible with a standalone Asio-style event loop.
The near-term assumptions are still:

- explicit async boundaries
- single-thread-friendly behavior first
- later strands, timers, executors, or coroutine adapters only when justified

The same applies to error handling:

- prefer structured result types
- keep reset and teardown signals explicit
- avoid exception-driven control flow across protocol and session seams

## Testing strategy

The repository should verify behavior in layers:

1. parser/serializer tests
2. protocol state-machine tests
3. fake-session peer-behavior tests
4. relay/cache behavior tests
5. sample-app smoke coverage using fake sessions first
6. real transport integration tests after behavioral parity is stable

The current code only covers the first layer fully and the session seam partially.
That is acceptable as long as the docs describe the gap honestly and new code keeps building toward
the parity target.

## Build stages

1. protocol/core types retained and expanded
2. MOQT session/control-plane state machines
3. symmetric publisher/subscriber runtime seams
4. object/group delivery consumer APIs
5. relay and cache composition layer
6. fake-session behavioral test coverage
7. native QUIC adapter and WebTransport-oriented adapter
8. sample publisher, subscriber, and relay applications
