# ADR 0001: Prefer Standalone Asio Over Boost.Asio for the Initial Architecture

## Status

Accepted for the prototype phase.

## Context

The project needs an async I/O and event-loop story early, even though the first milestone does not
yet connect real networking.

Two obvious options are:

- standalone Asio
- Boost.Asio

Both offer a similar programming model.
The question is not "which one is theoretically cleaner?"
The question is "which one keeps the beginner story understandable while keeping the dependency
surface tight as the relay grows?"

## Decision

Prefer standalone Asio as the initial architectural reference point.

This means:

- design public async boundaries so they can map cleanly onto Asio executors and handlers
- avoid hardwiring the core logic to any specific transport library
- defer direct Asio linkage until the code actually needs timers, sockets, or executor wiring

## Why this decision is reasonable

### Smaller dependency surface

The project can adopt the Asio programming model without taking a broader Boost dependency.
That keeps the eventual transport-layer dependency story narrower and easier to explain.

### Similar event-loop model without Boost-specific coupling

The architecture mainly needs:

- explicit async boundaries
- serial execution where appropriate
- a path toward timers and socket integration

Standalone Asio provides that same conceptual target without making Boost part of the baseline.

### Better fit for a small public surface

The repository keeps its public API narrow through explicit headers and transport abstractions.
Keeping third-party surface area smaller reduces the amount of compatibility and packaging guidance
the project needs to carry while it is still in an early teaching-oriented phase.

## Consequences

### Positive

- keeps the async I/O model familiar without adding the broader Boost dependency set
- preserves a clean fit with the project's preference for explicit abstractions before deep
  coroutine usage
- leaves room to integrate timers, sockets, and executors later without changing the core
  transport/session boundary

### Negative

- some C++ environments package Boost more uniformly than standalone Asio
- examples from the wider ecosystem may need minor translation when they use `boost::asio`

## Non-goals

This decision does not mean:

- relay code should expose Asio types everywhere
- `ngtcp2` integration should be written as Asio-specific business logic
- the project is committing to coroutine-heavy design early

The right interpretation is narrower:
standalone Asio is the preferred event-loop model, and the codebase should remain compatible with
that model as it grows.
