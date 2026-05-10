# ADR 0005: Prefer Result Types Over Exceptions

## Status

Accepted for repository code.

## Context

This repository is building protocol parsing, transport seams, and later stateful relay logic.
Those areas benefit from explicit error handling because callers usually need to inspect failure
states and decide what to do next.

Exception-based control flow would make that behavior less visible at the interface and would push
important protocol and transport error cases out of the type system.

## Decision

Prefer explicit result types with structured error values over exceptions in repository code.

This means:

- fallible operations should return a `common::result<Value, Error>`-style type
- error conditions should be modeled with explicit enums or other structured error types
- exceptions should not be the normal error-reporting mechanism for protocol or transport code

## Why this decision is reasonable

### Better fit for protocol code

Protocol parsing and validation usually need caller-visible failure categories, not just a generic
exception path.

### Better fit for transport seams

Transport interfaces should make failure part of the interface so adapters and callers can handle
unsupported operations and runtime errors explicitly.

### Clearer control flow

A result-returning interface makes success and failure paths obvious at the call site, which fits
the repository's teaching-oriented goals.

## Consequences

### Positive

- keeps error behavior visible in types and signatures
- makes tests easier to write against explicit failure values
- aligns the codebase more closely with Rust-style error handling discipline

### Negative

- introduces some boilerplate around result construction and propagation
- requires discipline so error enums remain coherent instead of proliferating randomly

## Non-goals

This decision does not mean:

- every helper must be turned into a generic error framework
- the repository must mimic Rust syntax exactly
- third-party libraries that use exceptions can never be used internally where that behavior is
  contained
