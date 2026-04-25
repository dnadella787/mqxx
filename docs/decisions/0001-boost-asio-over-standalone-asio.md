# ADR 0001: Prefer Boost.Asio Over Standalone Asio for the Initial Architecture

## Status

Accepted for the prototype phase.

## Context

The project needs an async I/O and event-loop story early, even though the first milestone does not
yet connect real networking.

Two obvious options are:

- Boost.Asio
- standalone Asio

Both offer a similar programming model.
The question is not "which one is theoretically cleaner?"
The question is "which one keeps the beginner story understandable while making it easier to grow
into a real relay?"

## Decision

Prefer Boost.Asio as the initial architectural reference point.

This means:

- design public async boundaries so they can map cleanly onto Boost.Asio executors and handlers
- avoid hardwiring the core logic to any specific transport library
- defer direct Boost.Asio linkage until the code actually needs timers, sockets, or executor wiring

## Why this decision is reasonable

### Better fit for a teaching-oriented prototype

Many C++ developers encounter Boost.Asio first.
That makes the learning material easier to align with existing examples, talks, and prior art.

### Easier packaging in many C++ environments

Boost is widely packaged and commonly supported by IDEs and system package managers.
That reduces friction once the project starts using real networking.

### The event-loop style matters more than the include path

At this stage the architecture mainly needs:

- explicit async boundaries
- serial execution where appropriate
- a path toward timers and socket integration

Boost.Asio gives a good conceptual target for all of that.

## Consequences

### Positive

- easier path toward timers, sockets, and executor-style composition
- more familiar ecosystem references for newcomers
- clean fit with the project's preference for explicit abstractions before deep coroutine usage

### Negative

- the project will eventually carry a Boost dependency when live I/O lands
- some developers prefer standalone Asio to reduce dependency surface

## Non-goals

This decision does not mean:

- relay code should expose Boost types everywhere
- `ngtcp2` integration should be written as Boost-specific business logic
- the project is committing to coroutine-heavy design early

The right interpretation is narrower:
Boost.Asio is the preferred event-loop model, and the codebase should remain compatible with that
model as it grows.
