# ADR 0004: Pin the Repository to MOQT Draft 17

## Status

Accepted for the current protocol implementation phase.

## Context

MOQT is still evolving, and draft-to-draft changes can shift section numbers, message semantics,
and terminology.
Leaving the repository implicitly aligned with "the latest draft" would make the code, tests, and
teaching material drift without an explicit review point.

The project needs one stable protocol baseline so implementation notes and future changes can be
evaluated against a concrete document rather than a moving target.

## Decision

Pin the repository's MOQT protocol baseline to `draft-ietf-moq-transport-17`.

This means:

- protocol-facing code and tests should be interpreted against draft 17
- `docs/protocol-notes.md` is the source of truth for section mappings against that draft
- moving to a later draft should be treated as an explicit repository change, not an implicit
  assumption

## Why this decision is reasonable

### Stable implementation target

A fixed draft gives the parser, naming logic, and future control-message work a concrete semantic
target.

### Clearer review and upgrade path

When the repository later moves from draft 17 to a newer draft, the delta can be reviewed
explicitly in code and docs instead of being mixed into unrelated changes.

### Better teaching value

This repository is intentionally beginner-friendly.
Beginner-facing protocol notes are easier to trust when they point to one exact draft revision.

## Consequences

### Positive

- reduces ambiguity in protocol discussions
- keeps code and docs traceable to one concrete MOQT revision
- makes future draft upgrades easier to reason about

### Negative

- later MOQT draft revisions may diverge from the pinned baseline
- the repository must update the pin deliberately when it is ready to absorb protocol changes

## Non-goals

This decision does not mean:

- draft 17 is assumed to be the final long-term protocol shape
- every future draft change must be rejected
- non-protocol implementation work needs to reference the draft unless it depends on protocol
  semantics
