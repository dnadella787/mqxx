# Protocol Notes

## Scope of this document

This file is the project's running map between code and the MOQT draft.
Every protocol-related implementation should be traceable back to a draft section here.

Because MOQT is still evolving, section numbers can change between draft revisions.
The references below are pinned to the current baseline used by this repository:

- `draft-ietf-moq-transport-18`
- published: 2026-05-12
- datatracker page: <https://datatracker.ietf.org/doc/draft-ietf-moq-transport/>

All protocol notes in this file should be read as draft-18-specific unless a later update says
otherwise.

## Behavioral target

The project target is now behavioral parity with Moxygen's core MOQT runtime model while still
mapping the implementation back to `draft-ietf-moq-transport-18`.

That means this file has two jobs:

- explain what draft-18 concepts the current code models
- explain which parity-oriented runtime concepts are only represented as interfaces so far

The current repository does not yet implement full MOQT control behavior.
It defines core types and runtime seams so that setup, subscribe, fetch, publish-namespace, relay,
and cache behavior can be built without rewriting the runtime model again.

## What is implemented right now

### Namespace and track rendering/parsing

Relevant draft sections:

- Section 1.5, "Representing Namespace and Track Names"
- Section 1.5.1, "Parsing Serialized Names"
- Section 2.4, "Track"
- Section 2.4.1, "Track Naming"

Current code:

- `mqxx/moqt/full_track_name.hpp`
- `mqxx/moqt/full_track_name.cpp`

What we implement:

- a runtime representation of full track names
- canonical rendering of namespace fields and track names to a safe text form
- canonical parsing rules that reject invalid or non-canonical encodings
- namespace-prefix matching helpers

What we do not implement yet:

- MOQT wire encoding for control messages
- SUBSCRIBE, FETCH, or PUBLISH message serialization
- authorization semantics around track names

### Namespace discovery model

Relevant draft sections:

- Section 6, "Namespace Discovery"
- Section 6.1, "Subscribing to Namespaces"
- Section 6.2, "Publishing Namespaces"

Current code:

- `mqxx/moqt/namespace_registry.hpp`
- `mqxx/moqt/namespace_registry.cpp`

What we implement:

- prefix matching for track namespaces
- a deterministic in-memory registry that acts like a tiny protocol-free helper for discovery state

What we do not implement yet:

- bidi request streams
- request IDs
- `SUBSCRIBE_NAMESPACE`
- `PUBLISH_NAMESPACE`
- `REQUEST_OK` / `REQUEST_ERROR`
- loop prevention, authorization, or forwarding policy

### Role-based runtime surface

Relevant draft sections:

- Section 3, "Sessions"
- Section 6, "Namespace Discovery"
- Section 9, "Relays"
- Section 9.4, "Subscriber Interactions"

Current code:

- `mqxx/moqt/session_types.hpp`
- `mqxx/moqt/session_roles.hpp`
- `mqxx/moqt/relay_seams.hpp`

What we implement:

- public request and delivery types for subscription, fetch, publish-namespace, object delivery,
  status delivery, and resets
- role-based interfaces for publisher/subscriber symmetry
- explicit handle types for subscribe-update, unsubscribe, fetch cancel, and namespace withdrawal
- relay/cache seam interfaces that let a relay compose subscriber and publisher behavior without
  transport leakage

What we do not implement yet:

- actual setup or subscribe state machines
- control message parsing/serialization
- request ID allocation and matching
- live relay forwarding behavior
- cache eviction, fan-in policy, or authorization policy

### Session boundary and fake-session harness

Relevant draft sections:

- Section 3.1.3, "WebTransport"
- Section 3.1.4, "Native QUIC"
- Section 3.4, "Unidirectional Stream Types"
- Section 7, "Priorities"
- Section 8, "Delivery Timeouts and Data Reliability"
- Section 9, "Relays"
- Section 9.1, "Caching Relays"
- Section 9.2, "Forward Handling"
- Section 9.4, "Subscriber Interactions"
- Section 10, "Control Messages"

Current code:

- `mqxx/transport/session.hpp`
- `mqxx/transport/fake_session.hpp`
- `mqxx/transport/fake_session.cpp`

What we implement:

- a session boundary that can carry:
  - outbound and inbound control bytes
  - uni-stream lifecycle and data
  - datagram send/receive behavior
  - stream reset signals
  - peer/session shutdown signals
  - flow-control notifications
  - delivery notifications
- an in-memory fake session for deterministic protocol and relay tests

What we do not implement yet:

- QUIC connections
- WebTransport-oriented transport wiring
- MOQT session setup
- stream prioritization
- draft-aligned control message encoders/decoders
- actual relay/cache behavior

## Immediate next protocol work

The next protocol-heavy work should target draft-18 control behavior directly:

- session setup and setup-response
- publish-namespace / withdraw behavior
- subscribe / subscribe-update / unsubscribe
- fetch request and fetch completion behavior
- track status and status update flows
- teardown and reset handling

Those changes should use the existing role and session seams rather than bypassing them.

## What is explicitly deferred

### Objects

Relevant draft sections:

- Section 2.1, "Objects"

Deferred reason:

The repository now has object-shaped delivery types in its runtime model, but it does not yet have
draft-aligned object state machines or relay behavior.
The next implementation work should add those behaviors carefully instead of pretending the current
types already imply full object support.

### Groups

Relevant draft sections:

- Section 2.3, "Groups"
- Section 2.3.1, "Group IDs"

Deferred reason:

The runtime model now includes group and subgroup boundary concepts because they are part of the
runtime envelope the relay eventually needs.
Actual group lifecycle behavior, ordering rules, and stream mapping are still deferred.

### Priorities and delivery policy

Relevant draft sections:

- Section 7.1, "Definitions"
- Section 7.2, "Scheduling Algorithm"
- Section 7.3, "Considerations for Setting Priorities"

Deferred reason:

Priority behavior is one of the easiest places to build something that looks plausible but behaves
incorrectly under load or congestion.
The project should not add scheduler logic until object and group state are already testable.
