# ADR 0003: Use Quill for Logging

## Status

Accepted for future integration.

## Context

The early prototype intentionally avoided taking a logging dependency while the architecture and
module boundaries were still fluid.
That deferment made sense at the start, but it is no longer the right long-term answer.

The project is growing toward a QUIC-based relay with multiple subsystems, transport bring-up, and
stateful protocol behavior.
That kind of system needs a real logging story rather than ad hoc streams or local helpers.

## Decision

Use Quill as the project's logging library when logging is integrated into the build and runtime.

This means:

- new logging design work should assume Quill rather than inventing a custom abstraction first
- local stream-based diagnostics are temporary, not the target operational model
- future transport, protocol, and relay subsystems should be wired with Quill-compatible logging
  boundaries

## Why this decision is reasonable

### Better fit for transport-heavy runtime diagnostics

Relay and QUIC bring-up work often needs detailed logs without turning the logging path itself into
a bottleneck.
Quill is a good fit for that operational profile.

### Useful filtering and formatting model

The project needs a path toward subsystem-specific verbosity control and captureable diagnostics
for debugging.
Quill provides that without the repository having to grow a homegrown logging framework.

### Clearer dependency direction

Recording the decision now prevents the docs from staying in an indefinite "no logger yet" state
after the project already knows what it wants to use.

## Consequences

### Positive

- establishes a concrete operational logging direction
- avoids spending design effort on temporary local logging helpers
- supports the expected growth from protocol prototype to multi-subsystem relay

### Negative

- adds another third-party dependency to explain, package, and version
- the codebase still needs a careful integration step so logging does not spread through public
  interfaces unnecessarily

## Non-goals

This decision does not mean:

- every source file needs logging immediately
- public protocol data types should expose Quill types
- the repository is committing to a large observability stack beyond choosing the logger
