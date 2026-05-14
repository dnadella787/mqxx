# Code Explanation

## Purpose of this document

This document explains the code that currently exists in `mqxx` in more depth than the README.
It is meant for a reader who is comfortable with C++ in general, but is new to QUIC, MOQT, and
the design choices in this repository.

Where this document discusses MOQT concepts, it assumes the repository's current pinned baseline:
`draft-ietf-moq-transport-18`.

The current codebase is intentionally small.
That is a feature, not a missing piece.
The repository is trying to establish good boundaries before it grows into real session handling,
relay forwarding, and QUIC transport integration.

## How the code is organized

The project uses a Moxygen-style split:

- headers and implementation files live together under the top-level `mqxx/...` tree
- the current bootstrap test harness lives in `tests/test_framework.hpp`

The dependency structure is:

1. [mqxx/common/byte_buffer.hpp](../mqxx/common/byte_buffer.hpp)
2. [mqxx/common/result.hpp](../mqxx/common/result.hpp)
3. [mqxx/moqt/full_track_name.cpp](../mqxx/moqt/full_track_name.cpp)
4. [mqxx/moqt/full_track_name.hpp](../mqxx/moqt/full_track_name.hpp)
5. [mqxx/moqt/namespace_registry.cpp](../mqxx/moqt/namespace_registry.cpp)
6. [mqxx/moqt/namespace_registry.hpp](../mqxx/moqt/namespace_registry.hpp)
7. [mqxx/moqt/static_track_descriptor.hpp](../mqxx/moqt/static_track_descriptor.hpp)
8. [mqxx/moqt/session_types.hpp](../mqxx/moqt/session_types.hpp)
9. [mqxx/moqt/session_roles.hpp](../mqxx/moqt/session_roles.hpp)
10. [mqxx/moqt/relay_seams.hpp](../mqxx/moqt/relay_seams.hpp)
11. [mqxx/transport/session.hpp](../mqxx/transport/session.hpp)
12. [mqxx/transport/fake_session.cpp](../mqxx/transport/fake_session.cpp)
13. [mqxx/transport/fake_session.hpp](../mqxx/transport/fake_session.hpp)
14. [tests/test_framework.hpp](../tests/test_framework.hpp)

## Full track names

Sources:
[mqxx/common/byte_buffer.hpp](../mqxx/common/byte_buffer.hpp)
[mqxx/common/result.hpp](../mqxx/common/result.hpp)
[mqxx/moqt/full_track_name.hpp](../mqxx/moqt/full_track_name.hpp)
[mqxx/moqt/full_track_name.cpp](../mqxx/moqt/full_track_name.cpp)

This subsystem defines the binary-safe model for MOQT track names.

- `mqxx::byte_string` is the shared byte-buffer type used across modules.
- `mqxx::result<Value, Error>` is the shared success/error carrier for fallible operations.
- `moqt::byte_string` aliases that shared type because the protocol treats names as binary, not as
  ordinary text strings.
- `track_namespace` is a vector of binary namespace fields.
- `full_track_name` holds a namespace and a track leaf name.
- `name_parse_result` is an explicit result type that returns either a parsed value or a parse
  error.

The implementation provides:

- canonical escaping of non-literal bytes with `.xx`
- parsing that rejects malformed and non-canonical escapes
- namespace-prefix matching helpers for later discovery logic

## Namespace registry

Sources:
[mqxx/moqt/namespace_registry.hpp](../mqxx/moqt/namespace_registry.hpp)
[mqxx/moqt/namespace_registry.cpp](../mqxx/moqt/namespace_registry.cpp)

`namespace_registry` is intentionally tiny.
It is not a full discovery protocol implementation.
It is a deterministic in-memory model that can:

- announce a track if it is not already present
- withdraw a previously announced track
- return tracks whose namespaces share a given prefix

The registry sorts by serialized track name so tests can assert stable ordering.

## Static track descriptor

Source:
[mqxx/moqt/static_track_descriptor.hpp](../mqxx/moqt/static_track_descriptor.hpp)

`static_track_descriptor` is a small template helper for fixed compile-time track shapes.
It stores namespace fields and a track name as non-type template parameters, then converts them
into a runtime `full_track_name` when needed.

That gives the project one conservative TMP example without turning the codebase into a template
exercise.
The short repository default is: keep TMP local and small.

## Role-based MOQT surface

Sources:
[mqxx/moqt/session_types.hpp](../mqxx/moqt/session_types.hpp)
[mqxx/moqt/session_roles.hpp](../mqxx/moqt/session_roles.hpp)
[mqxx/moqt/relay_seams.hpp](../mqxx/moqt/relay_seams.hpp)

The main MOQT runtime seam is now being moved toward role-based interfaces instead of helper types
plus a transport abstraction.
The key pieces are:

- request and delivery types for subscribe, fetch, publish-namespace, object delivery, status, and
  reset behavior
- `publisher` and `subscriber` interfaces
- `subscription_handle`, `fetch_handle`, and `publish_namespace_handle`
- `track_consumer`, `subgroup_consumer`, and `fetch_consumer`
- `relay_endpoint`, `relay_object_cache`, `subscription_aggregator`, and `forwarding_policy`

These types are mostly interface seams today.
They do not mean the repository already has full MOQT session behavior.
They exist so later session-control and relay work can land on a stable runtime model.

## Session boundary

Sources:
[mqxx/transport/session.hpp](../mqxx/transport/session.hpp)
[mqxx/transport/fake_session.hpp](../mqxx/transport/fake_session.hpp)
[mqxx/transport/fake_session.cpp](../mqxx/transport/fake_session.cpp)

`session` is the current transport-facing boundary.
It is richer than the earlier transport abstraction and is meant to support parity-oriented MOQT
behavior later.
It exposes operations and events for:

- control-byte send and receive
- uni-stream open, write, and receive
- datagram send and receive
- stream reset and session shutdown signals
- flow-control and delivery notifications

`fake_session` records outbound writes and lets tests queue inbound events in memory so protocol and
relay behavior can be exercised without real networking.

## Test harness

Source:
[tests/test_framework.hpp](../tests/test_framework.hpp)

The repository currently uses a tiny built-in test harness instead of a third-party dependency.
It provides:

- a test registration mechanism
- `expect_true`
- `expect_equal`
- a shared registry that `tests/test_main.cpp` executes

That harness is temporary bootstrap infrastructure.
The long-term framework direction is GoogleTest, but the build has not been migrated yet.

## Why the current code remains easy to follow

The code stays readable because it is solving a small, coherent problem set:

- binary-safe names
- deterministic prefix lookup
- one small compile-time descriptor helper
- role-based public MOQT seams
- one richer session abstraction
- one fake session for tests
