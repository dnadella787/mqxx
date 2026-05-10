# Code Explanation

## Purpose of this document

This document explains the code that currently exists in `mqxx` in more depth than the README.
It is meant for a reader who is comfortable with C++ in general, but is new to QUIC, MOQT, and
the design choices in this repository.

Where this document discusses MOQT concepts, it assumes the repository's current pinned baseline:
`draft-ietf-moq-transport-17`.

The current codebase is intentionally small.
That is a feature, not a missing piece.
The repository is trying to establish good boundaries before it grows into real session handling,
relay forwarding, and QUIC transport integration.

## How the code is organized

The project uses a standalone-application source layout with per-module include seams:

- module interfaces live under `src/<module>/include/...`
- shared support interfaces live under `src/common/include/...`
- implementation files live under `src/...`
- the current bootstrap test harness lives in `tests/test_framework.hpp`

The dependency structure is:

1. [src/common/include/mqxx/common/byte_buffer.hpp](/Users/dhanushnadella/Projects/mqxx/src/common/include/mqxx/common/byte_buffer.hpp)
2. [src/common/include/mqxx/common/result.hpp](/Users/dhanushnadella/Projects/mqxx/src/common/include/mqxx/common/result.hpp)
3. [src/moqt/full_track_name.cpp](/Users/dhanushnadella/Projects/mqxx/src/moqt/full_track_name.cpp)
4. [src/moqt/include/mqxx/moqt/full_track_name.hpp](/Users/dhanushnadella/Projects/mqxx/src/moqt/include/mqxx/moqt/full_track_name.hpp)
5. [src/moqt/namespace_registry.cpp](/Users/dhanushnadella/Projects/mqxx/src/moqt/namespace_registry.cpp)
6. [src/moqt/include/mqxx/moqt/namespace_registry.hpp](/Users/dhanushnadella/Projects/mqxx/src/moqt/include/mqxx/moqt/namespace_registry.hpp)
7. [src/moqt/include/mqxx/moqt/static_track_descriptor.hpp](/Users/dhanushnadella/Projects/mqxx/src/moqt/include/mqxx/moqt/static_track_descriptor.hpp)
8. [src/transport/include/mqxx/transport/transport_session.hpp](/Users/dhanushnadella/Projects/mqxx/src/transport/include/mqxx/transport/transport_session.hpp)
9. [src/transport/fake_transport_session.cpp](/Users/dhanushnadella/Projects/mqxx/src/transport/fake_transport_session.cpp)
10. [src/transport/include/mqxx/transport/fake_transport_session.hpp](/Users/dhanushnadella/Projects/mqxx/src/transport/include/mqxx/transport/fake_transport_session.hpp)
11. [tests/test_framework.hpp](/Users/dhanushnadella/Projects/mqxx/tests/test_framework.hpp)

## Full track names

Sources:
[src/common/include/mqxx/common/byte_buffer.hpp](/Users/dhanushnadella/Projects/mqxx/src/common/include/mqxx/common/byte_buffer.hpp)
[src/common/include/mqxx/common/result.hpp](/Users/dhanushnadella/Projects/mqxx/src/common/include/mqxx/common/result.hpp)
[src/moqt/include/mqxx/moqt/full_track_name.hpp](/Users/dhanushnadella/Projects/mqxx/src/moqt/include/mqxx/moqt/full_track_name.hpp)
[src/moqt/full_track_name.cpp](/Users/dhanushnadella/Projects/mqxx/src/moqt/full_track_name.cpp)

This subsystem defines the binary-safe model for MOQT track names.

- `mqxx::common::byte_string` is the shared byte-buffer type used across modules.
- `mqxx::common::result<Value, Error>` is the shared success/error carrier for fallible operations.
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
[src/moqt/include/mqxx/moqt/namespace_registry.hpp](/Users/dhanushnadella/Projects/mqxx/src/moqt/include/mqxx/moqt/namespace_registry.hpp)
[src/moqt/namespace_registry.cpp](/Users/dhanushnadella/Projects/mqxx/src/moqt/namespace_registry.cpp)

`namespace_registry` is intentionally tiny.
It is not a full discovery protocol implementation.
It is a deterministic in-memory model that can:

- announce a track if it is not already present
- withdraw a previously announced track
- return tracks whose namespaces share a given prefix

The registry sorts by serialized track name so tests can assert stable ordering.

## Static track descriptor

Source:
[src/moqt/include/mqxx/moqt/static_track_descriptor.hpp](/Users/dhanushnadella/Projects/mqxx/src/moqt/include/mqxx/moqt/static_track_descriptor.hpp)

`static_track_descriptor` is a small template helper for fixed compile-time track shapes.
It stores namespace fields and a track name as non-type template parameters, then converts them
into a runtime `full_track_name` when needed.

That gives the project one conservative TMP example without turning the codebase into a template
exercise.

## Transport boundary

Sources:
[src/transport/include/mqxx/transport/transport_session.hpp](/Users/dhanushnadella/Projects/mqxx/src/transport/include/mqxx/transport/transport_session.hpp)
[src/transport/include/mqxx/transport/fake_transport_session.hpp](/Users/dhanushnadella/Projects/mqxx/src/transport/include/mqxx/transport/fake_transport_session.hpp)
[src/transport/fake_transport_session.cpp](/Users/dhanushnadella/Projects/mqxx/src/transport/fake_transport_session.cpp)

`transport_session` is the current transport abstraction.
It exposes only the operations the early relay logic is expected to need:

- query whether datagrams are supported
- open a unidirectional stream
- write bytes to a stream
- send a datagram with an explicit success/error result

The transport code now shares its raw byte container and byte-view aliases with other modules
through `src/common/include/mqxx/common/byte_buffer.hpp` instead of defining those concepts
independently.
It also shares `src/common/include/mqxx/common/result.hpp` so transport adapters can report
structured errors without exceptions.

`fake_transport_session` records those writes in memory so tests can inspect behavior without
touching real networking.

## Test harness

Source:
[tests/test_framework.hpp](/Users/dhanushnadella/Projects/mqxx/tests/test_framework.hpp)

The repository currently uses a tiny built-in test harness instead of a third-party dependency.
It provides:

- a test registration mechanism
- `expect_true`
- `expect_equal`
- a shared registry that `tests/test_main.cpp` executes

That harness is temporary bootstrap infrastructure.
The dependency decision for the long-term framework is now GoogleTest, but the build has not been
migrated yet.

## Why the current code remains easy to follow

The code stays readable because it is solving a small, coherent problem set:

- binary-safe names
- deterministic prefix lookup
- one small compile-time descriptor helper
- one narrow transport abstraction
- one fake transport for tests
