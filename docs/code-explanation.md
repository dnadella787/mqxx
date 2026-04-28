# Code Explanation

## Purpose of this document

This document explains the code that currently exists in `mqxx` in more depth than the README.
It is meant for a reader who is comfortable with C++ in general, but is new to QUIC, MOQT, and
the specific design choices in this repository.

The current codebase is intentionally small.
That is a feature, not a missing piece.
The repository is trying to establish good boundaries before it grows into real session handling,
relay forwarding, and QUIC transport integration.

Right now there are really four code topics:

- full track names and their parser/renderer
- a namespace registry built on top of that naming model
- a small compile-time descriptor helper
- a transport abstraction with a fake implementation

The tests mirror those four topics closely.

## How the code is organized

The project uses named C++ modules rather than internal headers.
That is why most of the important code lives in `.cppm` files.

Current modules:

- `mqxx.moqt.full_track_name`
- `mqxx.moqt.namespace_registry`
- `mqxx.moqt.static_track_descriptor`
- `mqxx.transport.session`
- `mqxx.transport.fake_transport_session`
- `mqxx.test.framework`

The rough dependency structure is:

1. `mqxx.moqt.full_track_name`
2. `mqxx.moqt.namespace_registry` imports `mqxx.moqt.full_track_name`
3. `mqxx.moqt.static_track_descriptor` imports `mqxx.moqt.full_track_name`
4. `mqxx.transport.session`
5. `mqxx.transport.fake_transport_session` imports `mqxx.transport.session`
6. tests import the relevant project modules plus `mqxx.test.framework`

That layering is deliberate.
The parser and small state-model code should be understandable and testable before transport and
later relay logic exist.

## Big picture: what problem the current MOQT code is solving

The code is not implementing full MOQT yet.
It is implementing the smallest useful subset of MOQT-shaped concepts:

- a track has a namespace
- a track has a track name
- those pieces are binary values, not ordinary text strings
- tests and diagnostics need a deterministic text form
- namespace prefixes need to be matched in a predictable way

Those requirements lead directly to the code in `mqxx.moqt.full_track_name` and
`mqxx.moqt.namespace_registry`.

## Module: `mqxx.moqt.full_track_name`

Source: [src/moqt/full_track_name.cppm](/Users/dhanushnadella/Projects/mqxx/src/moqt/full_track_name.cppm)

This is the most important module in the repository right now.
If you understand this file, most of the rest of the project becomes straightforward.

### The core types

The first two aliases encode an important design decision:

- `byte_string = std::vector<std::uint8_t>`
- `track_namespace = std::vector<byte_string>`

These names make the code speak in protocol terms.

Why use `std::vector<std::uint8_t>` instead of `std::string`?

Because MOQT naming is binary-safe.
The code should not quietly assume:

- UTF-8
- printable text
- ordinary path-like strings

A namespace is modeled as a vector of fields, and each field is binary.
That is why `track_namespace` is a vector of `byte_string`.

The main runtime value is:

- `full_track_name`

It contains:

- `track_namespace track_namespace`
- `byte_string track_name`

That structure matches the conceptual model:

- namespace fields first
- track leaf name second

The equality operator is defaulted, which is enough because the type is a pure value object.

### Parse result instead of exceptions

Parsing returns:

- `name_parse_result`

This contains:

- `std::optional<full_track_name> value`
- `name_parse_error error`

This is a simple explicit result model.
It is good for protocol parsing because:

- the caller can inspect error cases directly
- tests can assert exact parse failures
- the code avoids using exceptions for ordinary malformed input

`ok()` is just a small convenience wrapper around `value.has_value()`.

### The serialized text format

The repository needs a deterministic text representation of binary names for:

- tests
- logs
- documentation examples

It uses these rules:

- bytes that are safe ASCII alnum or underscore are emitted literally
- everything else becomes `.` followed by two lowercase hex digits
- namespace fields are joined with `-`
- namespace and track name are separated by `--`

Examples:

- `camera_01` stays `camera_01`
- `.` becomes `.2e`
- `-` becomes `.2d`

This is not claiming to be the whole MOQT wire format.
It is a safe, canonical text form for local use.

### Internal helper: `is_literal_byte`

This helper answers:

"Can this byte be emitted directly without escaping?"

Currently the answer is yes for:

- ASCII letters and digits via `std::isalnum`
- underscore

Everything else is escaped.

That choice keeps the rendered form easy to read while still making separators unambiguous.

### Internal helper: `hex_digit_to_value`

This helper converts one hex character into a numeric nibble.
It accepts:

- `0`-`9`
- `a`-`f`
- `A`-`F`

But the parser later rejects uppercase escapes for canonicality.

That subtle split is useful:

- this helper decodes raw hex syntax
- higher-level parse logic decides whether the syntax is canonical

### Internal helper: `append_parsed_component`

This is the heart of the parser.
It parses one namespace field or one track name component from the repository’s serialized form.

Its job is:

- walk the text one character at a time
- copy literal bytes directly
- decode `.xx` escape sequences
- reject malformed or non-canonical encodings

The important validations are:

- if `.` is not followed by two characters: `invalid_escape`
- if either hex digit is uppercase: `uppercase_hex_escape`
- if either hex digit is not actually hex: `invalid_escape`
- if the decoded byte could have been emitted literally: `redundant_escape`

That last check matters.
It enforces canonical form.

Without it, both of these would decode to the same thing:

- `audio`
- `.61udio`

The code deliberately rejects the second form so there is exactly one valid serialized form for a
given byte sequence.

### Internal helper: `total_binary_length`

This computes the total number of binary bytes in the parsed name.

It is used to enforce:

- `kMaxFullTrackNameLength = 4096`

That limit is a project-side guardrail.
It is not there for aesthetics.
It prevents the local representation from silently accepting absurdly large names.

### `render_serialized_name`

This function converts one `byte_string` into the escaped text form.

Important implementation details:

- it uses `std::to_array("0123456789abcdef")` for a compact lowercase hex table
- it emits lowercase escapes consistently
- it never emits a redundant escape

This means render output is canonical by construction.

### `render_serialized_full_track_name`

This function turns a full runtime name into the combined textual form.

It:

1. renders each namespace field
2. joins them with `-`
3. appends `--`
4. renders the track name

An empty namespace is allowed.
So a name like:

- empty namespace
- track name `catalog`

renders as:

- `--catalog`

### `parse_serialized_full_track_name`

This function parses the whole textual form back into `full_track_name`.

Its steps are:

1. find the first `--`
2. split the input into namespace part and track part
3. split the namespace part on `-`
4. parse each namespace field with `append_parsed_component`
5. parse the track part with `append_parsed_component`
6. enforce namespace field count and total length limits

Important behavioral details:

- missing `--` is an error
- empty namespace fields are rejected
- empty overall namespace is allowed
- track names may contain escaped bytes
- namespace field count is capped at `32`

The parser returns a structured error rather than throwing.

### `namespace_starts_with`

This is a small helper, but it is foundational for later discovery and subscription work.

It answers:

"Is this namespace a prefix of that other namespace?"

Implementation details:

- first reject if prefix is longer than namespace
- then compare only the leading `prefix.size()` fields
- uses `std::ranges::equal` plus `std::views::take`

This is a clean example of using newer standard library features where they improve readability
without making the code clever for its own sake.

## Module: `mqxx.moqt.namespace_registry`

Source: [src/moqt/namespace_registry.cppm](/Users/dhanushnadella/Projects/mqxx/src/moqt/namespace_registry.cppm)

This module is intentionally much smaller than a real namespace discovery implementation.

It does not:

- speak to the network
- send MOQT messages
- track request IDs
- enforce auth

Instead, it is an in-memory model for:

- storing known tracks
- deduplicating them
- querying by namespace prefix

That makes it a very good testable stepping stone.

### Type: `namespace_registry`

This class just holds:

- `std::vector<full_track_name> tracks_`

The internal storage is plain and simple on purpose.
At this stage, clarity is more important than a more complex index structure.

### `announce_track`

This function inserts a track if it is not already present.

Its behavior:

1. use `std::ranges::find` to detect exact duplicates
2. if already present, return `false`
3. otherwise push it into `tracks_`
4. sort the vector by rendered full track name
5. return `true`

Why sort after every insertion?

Because the registry is being used as a deterministic model.
Tests and later logic benefit from stable output order.

That is more important right now than optimizing insertion cost.

### `withdraw_track`

This removes an exact track if it exists.

Its behavior:

- find exact match
- return `false` if absent
- erase and return `true` if present

Again, simple value semantics are enough.

### `tracks_with_prefix`

This returns all tracks whose namespace begins with a given prefix.

Its behavior:

- iterate over all stored tracks
- call `namespace_starts_with`
- copy matching tracks into a result vector

This is a linear scan, which is fine for the current milestone.
Later, if namespace discovery and relay forwarding become much larger, this could evolve into a
more indexed structure.

## Module: `mqxx.moqt.static_track_descriptor`

Source: [src/moqt/static_track_descriptor.cppm](/Users/dhanushnadella/Projects/mqxx/src/moqt/static_track_descriptor.cppm)

This is the repository’s first small piece of conservative template metaprogramming.

It is not there to make the code fancy.
It exists to show one useful pattern:

- some protocol-shaped names are known completely at compile time

### Type: `fixed_string`

`fixed_string` is a small helper that allows string literals to be used as non-type template
parameters.

Its job is:

- store a compile-time character array
- offer a `view()` helper that returns a `std::string_view`

This is what enables templates like:

- `static_track_descriptor<"camera_hd", "example.com", "meeting_7">`

### Type: `static_track_descriptor`

This template represents a known track shape at compile time.

Template parameters:

- one `track_name_value`
- zero or more `namespace_field_values`

What it provides:

- `namespace_field_count`
- `namespace_fields()`
- `track_name()`
- `make_runtime_name()`

### Why this exists

Sometimes application glue or test code knows a track name statically.
In that case, compile-time structure helps by:

- making the pieces explicit
- validating namespace-field count at compile time
- producing a runtime `full_track_name` value when needed

### `make_runtime_name`

This is the bridge between compile-time and runtime representations.

It:

1. creates a runtime `full_track_name`
2. reserves namespace storage
3. converts each compile-time field into `byte_string`
4. copies the compile-time track name into runtime storage

This is intentionally simple.
The TMP pattern does not replace the runtime model.
It just feeds it.

## Module: `mqxx.transport.session`

Source: [src/transport/transport_session.cppm](/Users/dhanushnadella/Projects/mqxx/src/transport/transport_session.cppm)

This is the beginning of the transport abstraction boundary.

The key architectural idea is:

- the relay core should not be written as if `ngtcp2` is the architecture

So the repository introduces a small abstract transport surface first.

### Type: `stream_id`

This is a tiny value type containing:

- `std::uint64_t value`

It exists so stream identity is represented in domain terms instead of just passing raw integers
everywhere.

### Interface: `transport_session`

This abstract class defines the smallest set of transport operations currently needed by the tests
and future relay direction:

- `supports_datagrams()`
- `open_uni_stream()`
- `write_stream(...)`
- `send_datagram(...)`

This is not yet a full QUIC API.
It is just enough to establish the idea that:

- stream writes and datagrams are different
- the caller should not directly know transport backend details

### Value types: `stream_write` and `datagram_write`

These are simple record types used by the fake transport implementation.

They hold:

- stream ID plus bytes plus FIN for streams
- bytes only for datagrams

They are not abstractions by themselves.
They are test-observable records.

## Module: `mqxx.transport.fake_transport_session`

Source: [src/transport/fake_transport_session.cppm](/Users/dhanushnadella/Projects/mqxx/src/transport/fake_transport_session.cppm)

This is a fake implementation of `transport_session`.

Its purpose is not to simulate real QUIC in detail.
Its purpose is to make higher-level logic testable without real sockets or QUIC handshakes.

### Type: `fake_transport_session`

It inherits from `transport_session` and stores:

- whether datagrams are supported
- the next synthetic stream ID
- a vector of recorded stream writes
- a vector of recorded datagram writes

### Constructor

The constructor takes:

- `bool datagram_support = true`

This lets tests model both:

- transports that support datagrams
- transports that do not

### `supports_datagrams`

This just returns the configured capability flag.

### `open_uni_stream`

This returns a synthetic `stream_id` and increments the internal counter.

The fake does not need to model real QUIC stream-numbering rules yet.
It only needs a deterministic, inspectable identifier.

### `write_stream`

This records a stream write by copying:

- stream ID
- bytes
- FIN flag

Copying is fine here because this is testing infrastructure.
We want simple ownership and easy later inspection.

### `send_datagram`

This first checks capability.

If datagrams are disabled:

- it throws `std::logic_error`

If datagrams are enabled:

- it records a `datagram_write`

This is useful because it turns unsupported behavior into a clear test failure instead of silent
state.

### `stream_writes` and `datagram_writes`

These expose the recorded writes by const reference.

That gives tests a direct, deterministic way to assert behavior.

## Module: `mqxx.test.framework`

Source: [tests/test_framework.cppm](/Users/dhanushnadella/Projects/mqxx/tests/test_framework.cppm)

This is a tiny homegrown test framework.

It exists because the repository currently wants:

- zero extra test dependency during bootstrap
- a buildable test suite in an otherwise empty repo

That is a temporary bootstrap choice, not necessarily a long-term one.

### Type: `test_failure`

This is just a `std::runtime_error` subtype used for assertion failures.

### Type: `test_case`

This records:

- test name
- callable body

### `registry()`

This returns a process-global vector of registered tests.

It uses a function-local static for simple initialization behavior.

### Type: `test_registration`

Constructing one of these pushes a new test into the registry.
The macro-based test files rely on this to self-register tests at static initialization time.

### `expect_equal` and `expect_true`

These are the assertion primitives.

They:

- check a condition
- build a readable failure message with file and line
- throw `test_failure` on failure

This keeps the framework tiny while still producing usable diagnostics.

## How the test files are structured

The test sources are:

- [tests/moqt_name_tests.cpp](/Users/dhanushnadella/Projects/mqxx/tests/moqt_name_tests.cpp)
- [tests/moqt_namespace_registry_tests.cpp](/Users/dhanushnadella/Projects/mqxx/tests/moqt_namespace_registry_tests.cpp)
- [tests/moqt_static_track_descriptor_tests.cpp](/Users/dhanushnadella/Projects/mqxx/tests/moqt_static_track_descriptor_tests.cpp)
- [tests/transport_fake_transport_session_tests.cpp](/Users/dhanushnadella/Projects/mqxx/tests/transport_fake_transport_session_tests.cpp)
- [tests/test_main.cpp](/Users/dhanushnadella/Projects/mqxx/tests/test_main.cpp)

Each test file defines a few macros locally:

- `SP_TEST`
- `SP_EXPECT`
- `SP_EXPECT_EQ`

That approach is a bit repetitive, but it keeps the bootstrap framework extremely small and avoids
introducing another header just for test macros.

`test_main.cpp` simply:

1. iterates over the registered tests
2. runs them one by one
3. catches exceptions
4. prints pass/fail output
5. returns a failing exit code if any test failed

## What each test group is proving

### `moqt_name_tests.cpp`

These tests prove that:

- safe bytes render literally
- unsafe bytes are escaped
- render and parse round-trip correctly
- empty namespace is supported
- malformed syntax is rejected
- non-canonical syntax is rejected

This is important because parser and canonicalization bugs tend to multiply later if not pinned down
early.

### `moqt_namespace_registry_tests.cpp`

These tests prove that:

- duplicate announcements are rejected
- prefix queries work
- ordering is deterministic
- withdrawals remove entries correctly

This makes the registry a reliable small state model.

### `moqt_static_track_descriptor_tests.cpp`

These tests prove that:

- compile-time descriptor shape is exposed correctly
- the descriptor can produce a runtime `full_track_name`

This validates both the compile-time and runtime bridge.

### `transport_fake_transport_session_tests.cpp`

These tests prove that:

- stream writes are recorded faithfully
- datagrams are recorded when supported
- datagrams fail loudly when unsupported

This is the foundation for later fake-transport-driven session tests.

## Why this code is written this way

Several choices may look overbuilt for such a small repository.
They are there because the repository is optimizing for future correctness and testability, not just
for writing the fewest lines today.

### Why modules already

Because the repository wants explicit exported surfaces from the start.
That helps separate:

- public project-facing API
- internal helper logic

even in a small codebase.

### Why snake_case type names

Because the repository has chosen snake_case consistently for its own code symbols.
That keeps naming uniform across:

- types
- free functions
- test helpers

### Why no real QUIC code yet

Because introducing real QUIC too early would blur multiple concerns at once:

- protocol modeling
- parser correctness
- async I/O
- TLS integration
- packet timing

The current code is deliberately building the conceptual ground floor first.

### Why fake transport before real transport

Because state and protocol logic should be testable independently from:

- sockets
- system call timing
- TLS handshakes
- network flakiness

That separation becomes more valuable, not less, as the code grows.

## How to read the code in the best order

If you are studying the code, this order works well:

1. [src/moqt/full_track_name.cppm](/Users/dhanushnadella/Projects/mqxx/src/moqt/full_track_name.cppm)
2. [tests/moqt_name_tests.cpp](/Users/dhanushnadella/Projects/mqxx/tests/moqt_name_tests.cpp)
3. [src/moqt/namespace_registry.cppm](/Users/dhanushnadella/Projects/mqxx/src/moqt/namespace_registry.cppm)
4. [tests/moqt_namespace_registry_tests.cpp](/Users/dhanushnadella/Projects/mqxx/tests/moqt_namespace_registry_tests.cpp)
5. [src/moqt/static_track_descriptor.cppm](/Users/dhanushnadella/Projects/mqxx/src/moqt/static_track_descriptor.cppm)
6. [tests/moqt_static_track_descriptor_tests.cpp](/Users/dhanushnadella/Projects/mqxx/tests/moqt_static_track_descriptor_tests.cpp)
7. [src/transport/transport_session.cppm](/Users/dhanushnadella/Projects/mqxx/src/transport/transport_session.cppm)
8. [src/transport/fake_transport_session.cppm](/Users/dhanushnadella/Projects/mqxx/src/transport/fake_transport_session.cppm)
9. [tests/transport_fake_transport_session_tests.cpp](/Users/dhanushnadella/Projects/mqxx/tests/transport_fake_transport_session_tests.cpp)
10. [tests/test_framework.cppm](/Users/dhanushnadella/Projects/mqxx/tests/test_framework.cppm)
11. [tests/test_main.cpp](/Users/dhanushnadella/Projects/mqxx/tests/test_main.cpp)

That order moves from protocol model, to supporting state model, to compile-time helper, to
transport abstraction, to test infrastructure.

## What is still missing on purpose

This explanation would be misleading if it implied the repository already has a relay core.
It does not.

Not implemented yet:

- QUIC socket integration
- ngtcp2 session wiring
- MOQT control messages
- object/group/priority logic
- subscriber management
- relay forwarding policy
- caching behavior
- auth
- media payload semantics

The current code is the first layer that makes those future pieces less risky to add.

## Summary

The current codebase is small, but it is not arbitrary.
Each existing module is establishing one boundary:

- `mqxx.moqt.full_track_name`: binary-safe naming and canonical parsing
- `mqxx.moqt.namespace_registry`: deterministic prefix-based state model
- `mqxx.moqt.static_track_descriptor`: conservative compile-time description of known track shapes
- `mqxx.transport.session`: transport abstraction boundary
- `mqxx.transport.fake_transport_session`: deterministic fake transport for tests
- `mqxx.test.framework`: bootstrap testing infrastructure

If future changes preserve those boundaries, the repository should be able to grow into real QUIC
and MOQT functionality without collapsing into transport-specific or test-hostile code.
