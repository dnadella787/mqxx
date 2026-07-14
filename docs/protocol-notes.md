# Protocol Notes

## MOQT Track Naming

This repository currently models the track naming concepts from
`draft-ietf-moq-transport-18`, Section 2.4.1.

Implemented now:

- `moqt::track_namespace` stores an ordered sequence of binary namespace fields.
- `moqt::track_name` stores the binary track name.
- `moqt::full_track_name` pairs a namespace with a track name.
- the track identity types expose `describe()` convenience methods; namespace and full-track
  rendering use the Section `1.5` textual form, and track names use the same byte escaping for the
  leaf bytes only.
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

- session close behavior for malformed wire values
- control-plane state machines
- subscribe and fetch flows
- object and group delivery
- track aliases
- QUIC transport integration

## MOQT Wire Primitives And Name Rendering

This repository now models the MOQT-specific wire encoding rules from
`draft-ietf-moq-transport-18`, Sections `1.4`, `1.5`, `1.5.1`, and `2.4.1`.

Implemented now:

- `moqt::decode_varint()` and `moqt::encode_varint()` implement the MOQT `1..9` byte integer
  format.
- encoders always emit the minimal valid varint length.
- decoders accept any valid varint length, including non-minimal encodings.
- `moqt::location` codecs serialize and parse ordered `(group, object)` pairs.
- key-value pair codecs parse and render ordered parameter sequences using delta-coded types,
  varint values for even types, and length-prefixed binary values for odd types.
- reason phrases are length-prefixed, limited to `1024` bytes, and validated as UTF-8.
- track namespace and full track name wire codecs build the existing `track_namespace`,
  `track_name`, and `full_track_name` model types.
- Section `2.4.1` limits are enforced during decode as explicit library-level codec errors:
  namespace field count up to `32`, non-empty namespace fields only, namespace byte length up to
  `4096`, and full track name byte length up to `4096`.
- `describe()` on the track identity types renders the Section `1.5` textual form.
- only ASCII letters, digits, and underscore render literally; every other byte renders as `.xx`
  with lowercase hex digits.
- namespace fields are separated by `-`, and full track names are separated by `--`.
- empty namespaces are valid, and an empty-namespace full track name renders as `--track`.

These codecs stay MOQT-local rather than promoting the varint or name-rendering rules into a
transport-agnostic utility layer.

Deferred intentionally:

- mapping malformed wire input to MOQT Session close behavior
- control-plane messages and state machines that consume these codecs

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
