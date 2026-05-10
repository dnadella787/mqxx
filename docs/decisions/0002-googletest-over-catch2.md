# ADR 0002: Prefer GoogleTest Over Catch2 for the Main Test Framework

## Status

Accepted for the prototype-to-growth transition.

## Context

The repository started with a tiny built-in test harness so the first protocol milestone could
build in a nearly empty workspace.
That was useful for bootstrapping, but it is not the right long-term test framework as protocol
parsers, state machines, fake transports, and later integration tests grow in number and
complexity.

The project needed to make an explicit choice between adopting GoogleTest or Catch2 once a real
third-party test dependency becomes acceptable.

## Decision

Prefer GoogleTest over Catch2 for the repository's long-term unit and integration-style test
framework.

This means:

- new framework-based tests should target GoogleTest
- the current built-in harness should be treated as temporary bootstrap infrastructure
- the repository should not invest further in Catch2-specific test style or migration work

## Why this decision is reasonable

### More conventional fixture model

The codebase is expected to add more transport and stateful protocol tests.
GoogleTest's fixture-oriented structure maps cleanly onto that kind of setup and teardown.

### Wider ecosystem familiarity

GoogleTest is a very common baseline in C++ projects.
That lowers the cost for new contributors reading or extending the tests.

### Better fit for growing protocol coverage

As parser, serializer, and state-machine coverage expands, parameterized tests and the broader
GoogleTest ecosystem become more useful than maintaining a custom harness or adopting a more
expression-oriented style.

## Consequences

### Positive

- gives the project a conventional testing baseline as coverage grows
- avoids spending more time on a custom test framework
- aligns well with fixture-heavy transport and protocol test scenarios

### Negative

- adds a nontrivial third-party dependency to the test build
- delays the benefit until the repository actually performs the migration from the bootstrap
  harness

## Non-goals

This decision does not mean:

- the repository must migrate every existing test immediately
- GoogleTest needs to leak into production code design
- the project is standardizing on any broader Google C++ stack beyond the test framework
