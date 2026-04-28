# TMP Notes

## Why template metaprogramming appears in this repository at all

The project wants to use template metaprogramming conservatively from the start.
That does not mean "use templates everywhere."
It means:

- use compile-time structure when it improves correctness
- keep the runtime model readable
- explain the pattern in plain language

The project also now prefers named C++ modules for core code.
That matters for TMP because many C++ codebases historically hide large amounts of TMP in headers.
This repository should resist that pattern where possible.

The first TMP pattern in this repository is `static_track_descriptor`, exported from the
`mqxx.moqt.static_track_descriptor` module.

## What `static_track_descriptor` does

`static_track_descriptor` lets code describe a known track schema at compile time.
For example, test code or application glue may know that a track always lives under a certain
namespace prefix and has a fixed track name.

That allows:

- compile-time namespace field counting
- compile-time storage of the literal pieces
- easy conversion into the runtime `full_track_name` type

## Why this is useful

This project expects later milestones to define more protocol-oriented descriptors such as:

- message schemas
- validation rules
- track families
- namespace conventions

Starting with a very small descriptor keeps the concept grounded.

## Why this pattern stays conservative

`static_track_descriptor` does not try to replace the runtime protocol model.
MOQT names are binary values, and the runtime types still own that reality.

The descriptor is only a convenience for code that already knows its names at compile time.
That is a good trade:

- correctness improves for fixed schemas
- runtime code stays normal and readable
- new contributors do not have to learn advanced TMP just to understand the parser

## How TMP should interact with modules

As the repository migrates toward modules, TMP utilities should usually live in one of two places:

- directly in a small module interface when they are part of the exported API
- in a non-exported implementation unit or partition when they are internal machinery

What should be avoided:

- giant header-only TMP utilities that become accidental dependencies everywhere
- exporting metaprogramming helpers that are only implementation details
- using modules as an excuse to hide overly complex type tricks

The goal is not "fancy module metaprogramming."
The goal is a codebase where compile-time structure supports correctness without making the learning
curve steeper than it needs to be.
