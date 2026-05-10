# Protocol Notes

## Scope of this document

This file is the project's running map between code and the MOQT draft.
Every protocol-related implementation should be traceable back to a draft section here.

Because MOQT is still evolving, section numbers can change between draft revisions.
The references below are pinned to the current baseline used by this repository:

- `draft-ietf-moq-transport-17`
- published: 2026-03-02
- datatracker page: <https://datatracker.ietf.org/doc/html/draft-ietf-moq-transport-17>

All protocol notes in this file should be read as draft-17-specific unless a later update says
otherwise.

## What is implemented right now

### Namespace and track rendering/parsing

Relevant draft sections:

- Section 1.5, "Representing Namespace and Track Names"
- Section 1.5.1, "Parsing Serialized Names"
- Section 2.4, "Track"
- Section 2.4.1, "Track Naming"

Current code:

- `src/moqt/include/mqxx/moqt/full_track_name.hpp`
- `src/moqt/full_track_name.cpp`

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

- `src/moqt/include/mqxx/moqt/namespace_registry.hpp`
- `src/moqt/namespace_registry.cpp`

What we implement:

- prefix matching for track namespaces
- a deterministic in-memory registry that acts like a tiny protocol-free model of discovery state

What we do not implement yet:

- bidi request streams
- request IDs
- `SUBSCRIBE_NAMESPACE`
- `PUBLISH_NAMESPACE`
- `REQUEST_OK` / `REQUEST_ERROR`
- loop prevention, authorization, or forwarding policy

### Relay role and transport boundary

Relevant draft sections:

- Section 3.1.2, "QUIC"
- Section 7, "Priorities"
- Section 8, "Relays"
- Section 8.1, "Caching Relays"
- Section 8.2, "Forward Handling"
- Section 8.4, "Subscriber Interactions"

Current code:

- `src/transport/include/mqxx/transport/transport_session.hpp`
- `src/transport/include/mqxx/transport/fake_transport_session.hpp`
- `src/transport/fake_transport_session.cpp`

What we implement:

- only a minimal transport abstraction
- only an in-memory fake transport used for tests

What we do not implement yet:

- QUIC connections
- MOQT session setup
- stream prioritization
- caching
- subscription aggregation
- authorization checks
- forwarding behavior

## What is explicitly deferred

### Objects

Relevant draft sections:

- Section 2.1, "Objects"

Deferred reason:

The project is still in the namespaces-and-tracks milestone.
Adding object logic before the naming and discovery foundations are stable would make the first
teaching step harder to follow.

### Groups

Relevant draft sections:

- Section 2.3, "Groups"
- Section 2.3.1, "Group IDs"

Deferred reason:

Groups add ordering, join-point, and later scheduler implications.
Those concepts are better introduced after objects have a clean representation.

### Priorities and delivery policy

Relevant draft sections:

- Section 7.1, "Definitions"
- Section 7.2, "Scheduling Algorithm"
- Section 7.3, "Considerations for Setting Priorities"

Deferred reason:

Priority behavior is one of the easiest places to build something that looks plausible but behaves
incorrectly under load or congestion.
The project should not add scheduler logic until object and group state are already testable.
