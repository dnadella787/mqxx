# Repository Layout

This repository is still intentionally small. The directory structure is already in place so new
protocol and transport work can be added in predictable modules without reworking the build layout.

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
  Code modules and the deliberate project header subtree.
- `tests/`
  GoogleTest sources and test build wiring.

## Module directories

- `mqxx/common/`
  Shared utility and foundational types go here. Public headers stay under the `mqxx/` subtree.
- `mqxx/moqt/`
  MOQT protocol, naming, session, and other domain-specific logic should go here.
- `mqxx/transport/`
  Transport abstractions and concrete transport integrations should go here.

Each module already has a local `CMakeLists.txt`. Keep target wiring local to the module when new
files are added, and keep includes rooted at `mqxx/...`.

## Current target structure

- `mqxx_common`
  `INTERFACE` target for shared includes, common compile requirements, and shared headers.
- `mqxx_moqt`
  `INTERFACE` placeholder target for the MOQT module.
- `mqxx_transport`
  `INTERFACE` placeholder target for the transport module.
- `mqxx_core`
  `INTERFACE` placeholder target that links the module targets together.

This is intentionally minimal. The targets are present so tooling and future edits have a stable
starting point, and only the first shared common header plus its tests are checked in today.

## Rebuild guidance

When reintroducing code, prefer to keep:

- public interfaces grouped coherently by module
- implementation details localized to the owning module
- test sources under `tests/`
- build wiring added in the nearest local `CMakeLists.txt` instead of inflating the root file
