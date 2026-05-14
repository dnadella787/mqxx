# Project Decisions

This file is the short decision summary for contributors and agents.
Read this first for repository-wide defaults and current direction.

## Current direction

- `mqxx` is being built toward Moxygen-level behavioral parity for core standalone MOQT relay,
  publisher, and subscriber behavior.
- The target scope is MOQT session/control behavior, symmetric publisher/subscriber APIs,
  relay/cache composition, and non-media sample flows.
- FLV/media-packaging workflows, browser/player integration, and `moq-encoder-player`-style flows
  remain deferred.

## Protocol baseline

- Treat `draft-ietf-moq-transport-18` as the active protocol baseline.
- Map protocol-facing work back to [protocol-notes.md](protocol-notes.md).
- Moving to a later draft is an explicit repository change, not an implicit assumption.

## Runtime seam shape

- The main runtime seams are role-based MOQT interfaces, not the transport layer.
- Favor `publisher`, `subscriber`, request handles, and consumer interfaces as the stable internal
  runtime model and sample-app model.
- Keep the code in the top-level `mqxx/` tree so the repository has one obvious implementation
  subtree, following the Moxygen-style layout.
- Keep `full_track_name`, namespace helpers, and result/error types as support types.
- Treat `namespace_registry` as a helper for discovery/relay logic, not the center of the design.

## Transport and runtime boundaries

- Keep protocol logic transport-agnostic.
- Use the lower-level `transport::session` boundary for control messages, uni-stream lifecycle,
  datagrams, resets, shutdown, and delivery/flow-control notifications.
- Keep both native QUIC and WebTransport-oriented adapters as first-class architectural targets.
- Stay compatible with a standalone Asio-style event-loop model without leaking Asio types into the
  public MOQT model.

## Repository-wide implementation defaults

- Prefer explicit `result<Value, Error>`-style returns over exceptions for normal protocol
  and transport error paths.
- Keep headers small, keep implementation in `.cpp` files, and avoid growing header-only internal
  frameworks.
- Use template metaprogramming conservatively; `static_track_descriptor` is the intended scale, not
  a signal to build TMP-heavy infrastructure.
- Keep protocol and relay tests deterministic and fake-session-friendly before leaning on real
  networking.

## Dependencies

- Preferred eventual event-loop integration model: standalone Asio.
- Preferred long-term unit/integration test framework: GoogleTest.
- Preferred logging library when operational logging is added: Quill.
- None of those choices should leak directly into the public MOQT data model.

## Immediate practical caveats

- Quill is not wired into the build yet.
- The current code defines role/session seams and fake-session infrastructure, but full draft-18
  control-plane behavior is still being implemented.
