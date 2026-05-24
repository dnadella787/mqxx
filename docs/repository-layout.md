# Repository Layout

This repository is currently a scaffold. The directories remain so future work can be added in a
predictable place.

## Top level

- `CMakeLists.txt`
  Root project entry point.
- `cmake/`
  Shared warning and sanitizer helpers.
- `conanfile.py`
  Conan recipe and CMake toolchain/preset generation.
- `profiles/`
  Checked-in Conan profiles for local development.
- `docs/`
  Setup and organization notes only.
- `mqxx/`
  Code modules.
- `tests/`
  Test build wiring only; no test sources are currently checked in.

## Module directories

- `mqxx/common/`
  Shared utility and foundational types should go here.
- `mqxx/moqt/`
  MOQT protocol, naming, session, and other domain-specific logic should go here.
- `mqxx/transport/`
  Transport abstractions and concrete transport integrations should go here.

Each module already has a local `CMakeLists.txt`. Keep target wiring local to the module when new
files are added.

## Current target structure

- `mqxx_common`
  `INTERFACE` scaffold target for shared includes and common compile requirements.
- `mqxx_moqt`
  `INTERFACE` scaffold target for the MOQT module.
- `mqxx_transport`
  `INTERFACE` scaffold target for the transport module.
- `mqxx_core`
  `INTERFACE` scaffold target that links the module targets together.

This is intentionally minimal. The targets are present so tooling and future edits have a stable
starting point, but there is no checked-in implementation.

## Rebuild guidance

When reintroducing code, prefer to keep:

- public interfaces grouped coherently by module
- implementation details localized to the owning module
- test sources under `tests/`
- build wiring added in the nearest local `CMakeLists.txt` instead of inflating the root file
