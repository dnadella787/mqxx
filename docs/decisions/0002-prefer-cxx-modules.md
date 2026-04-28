# ADR 0002: Prefer Named C++ Modules for Project Code

## Status

Accepted for the prototype phase.

## Context

The repository started with a traditional header/source layout because that was the fastest way to
bootstrap an empty codebase.

That bootstrap choice is no longer the preferred long-term direction.
The project wants:

- clearer public API boundaries
- fewer accidental transitive dependencies
- a structure that teaches what is exported versus internal
- a modern C++ code organization model that can scale with the relay

Named C++ modules are a better fit for those goals than continuing to grow a header-first internal
architecture.

## Decision

Prefer named C++ modules for new core project code.

This means:

- new internal library surfaces should default to module interfaces and implementation units
- headers should become the exception, not the default
- module boundaries should follow architectural boundaries such as MOQT model, transport, session,
  parsing, and scheduling subsystems

## What this does not mean

This decision does not require:

- an immediate rewrite of the whole repository
- wrapping every third-party dependency in a custom module layer on day one
- advanced partition usage before the team actually needs it
- ignoring compiler and tooling limitations

The migration should be incremental and boring.

## Why this decision is reasonable

### Better API hygiene

Modules make exports explicit.
That is a good fit for a relay codebase where transport boundaries, parser boundaries, and
state-machine boundaries should remain deliberate.

### Better teaching value

This project is supposed to teach.
Named modules help show the difference between public surface area and implementation details more
cleanly than a pile of headers does.

### Lower accidental coupling over time

As the project grows into objects, groups, priorities, and eventually relay/session orchestration,
it will become more important to prevent everything from including everything else.

## Consequences

### Positive

- clearer exported surfaces
- better pressure against accidental dependency sprawl
- a more modern internal structure for long-term growth

### Negative

- toolchain compatibility matters more
- CMake and IDE support need closer scrutiny
- migration from the bootstrap header layout will take deliberate work

## Migration guidance

The first module migration steps should be small:

- convert one subsystem at a time
- keep behavior changes separate from packaging changes
- keep tests stable during the migration
- document each nontrivial module boundary in beginner-friendly language

Likely early candidates:

- full track name model/parsing
- namespace registry
- fake transport session

## Current repository state

The first milestone has now been migrated to named modules for:

- full track name model and parser logic
- namespace registry
- static track descriptors
- transport session abstractions
- fake transport session support

The next changes should extend this module layout instead of reintroducing internal headers for
project code.
