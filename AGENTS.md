# Agent Guidance

This file is intentionally ignored by Git.
It exists as local guidance for human and AI contributors working in this repository.

## Core implementation preference

- Prefer traditional headers plus `.cpp` implementation files for core library code.
- Follow the Moxygen-style public header pattern in this repo:
  keep project headers under the dedicated top-level `mqxx/` code subtree, not under the repo
  root at large.
- Keep public headers small and readable.
- Prefer explicit types and functions over broad umbrella headers.
- Avoid unnecessary layering or compatibility shims unless they solve a real interoperability need.

## Project header layout

- Treat `mqxx/` as the deliberate code/header tree for this repository.
- This project is currently standalone-first, not package-first.
- Do not treat the entire repo root as a casual dumping ground for headers; keep code under `mqxx/`
  and keep includes in `mqxx/...` form.
- When adding new headers, keep the include contract stable inside the project even if there is no
  external package consumer yet.

## When header-only code is still acceptable

- templates that must be visible at the point of use
- generated configuration headers
- platform/compiler compatibility shims
- small utilities where a separate `.cpp` file adds no value

## Documentation expectations

- If you add or change protocol code, update `docs/protocol-notes.md`.
- If you add a nontrivial include/layout pattern, explain it in beginner-friendly terms in the docs.
- Keep comments in code restrained; put the longer teaching material in docs.

## Local project skills

The repository also keeps local Codex skills under `.codex/skills/`.
These are ignored by Git on purpose.

Current local skills:

- `cpp26-modules` (legacy skill; current repo layout is header/source based)
- `moqt-protocol-work`
- `ngtcp2-transport`
- `codec-state-machine-tests`
- `media-relay-architecture`
