# TMP Notes

## Why template metaprogramming appears in this repository at all

The project wants to use template metaprogramming conservatively from the start.
That does not mean "use templates everywhere."
It means:

- use compile-time structure when it improves correctness
- keep the runtime model readable
- explain the pattern in plain language

The first TMP pattern in this repository is `StaticTrackDescriptor`.

## What `StaticTrackDescriptor` does

`StaticTrackDescriptor` lets code describe a known track schema at compile time.
For example, test code or application glue may know that a track always lives under a certain
namespace prefix and has a fixed track name.

That allows:

- compile-time namespace field counting
- compile-time storage of the literal pieces
- easy conversion into the runtime `FullTrackName` type

## Why this is useful

This project expects later milestones to define more protocol-oriented descriptors such as:

- message schemas
- validation rules
- track families
- namespace conventions

Starting with a very small descriptor keeps the concept grounded.

## Why this pattern stays conservative

`StaticTrackDescriptor` does not try to replace the runtime protocol model.
MOQT names are binary values, and the runtime types still own that reality.

The descriptor is only a convenience for code that already knows its names at compile time.
That is a good trade:

- correctness improves for fixed schemas
- runtime code stays normal and readable
- new contributors do not have to learn advanced TMP just to understand the parser
