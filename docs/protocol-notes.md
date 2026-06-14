# Protocol Notes

## MOQT Track Naming

This repository currently models the track naming concepts from
`draft-ietf-moq-transport-18`, Section 2.4.1.

Implemented now:

- `moqt::track_namespace` stores an ordered sequence of binary namespace fields.
- `moqt::track_name` stores the binary track name.
- `moqt::full_track_name` pairs a namespace with a track name.
- construction returns `std::expected` so invalid protocol values are explicit.
- accessors expose read-only views or references to owned bytes.
- equality and ordering compare the exact stored byte sequences.
- namespace prefixes may contain zero fields.
- namespace fields must be non-empty.
- namespace field count is limited to 32.
- namespace byte length and full track name byte length are limited to 4096 bytes.

These types are deliberately transport-agnostic. They own their bytes and do not assume that names
are text.

Deferred intentionally:

- wire encoding and decoding
- session close behavior for malformed wire values
- control-plane state machines
- subscribe and fetch flows
- object and group delivery
- track aliases
- QUIC transport integration

## Namespace Registry

The in-memory `moqt::namespace_registry` stores namespace entries by `moqt::track_namespace`.

Implemented now:

- insertion returns `std::expected` with the inserted entry or an explicit duplicate error.
- duplicate insertion leaves the existing entry and registry size unchanged.
- exact lookup returns the entry whose namespace exactly equals the query namespace.
- longest-prefix lookup returns the registered namespace with the most whole fields matching the
  leading fields of the query namespace.
- the empty namespace is a valid prefix of every namespace.
- empty registry and not-found lookups return `std::nullopt`.

Prefix matching is field-wise. A registered field must equal the whole query field at the same
position; matching only the leading bytes within a field is not a prefix match.

Deferred intentionally:

- publish and subscribe control messages
- relay policy beyond deterministic lookup semantics
- concurrent access
- production indexing and performance tuning
- transport integration
