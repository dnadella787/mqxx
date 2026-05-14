# Beginner Guide

This document is for a reader who does **not** already know Media over QUIC (MOQT), but does have
a basic mental model for:

- HTTP/1.1 request/response
- TCP as a reliable byte stream
- UDP as unordered packets with no built-in reliability

It explains:

1. what MOQT is trying to do
2. how this repository maps onto that problem
3. what is actually implemented today
4. why the current implementation starts where it does
5. what comes next

## Start with the transport problem

HTTP/1.1 is good at request/response:

- client asks for a resource
- server sends the response
- the client usually wants the whole thing

That model becomes awkward for live or fast-moving data:

- a live camera feed
- score updates
- chat-like streams of small objects
- many subscribers joining at different times
- relays forwarding the same data to many downstream clients

TCP gives reliability, but it is one ordered byte stream per connection.
If one part is delayed, later bytes are held back behind it.
That is often the wrong trade for live media or low-latency object delivery.

UDP removes that head-of-line blocking, but then the application has to rebuild almost everything:

- reliability rules
- congestion handling
- stream separation
- retransmission rules
- security

QUIC is the modern middle ground:

- runs over UDP
- provides encryption and connection management
- supports multiple streams inside one connection
- can deliver reliable ordered stream data
- can also support datagram-style delivery for data that should not be retransmitted forever

MOQT is an application protocol built on top of QUIC.

## What MOQT is, in plain language

MOQT is a protocol for publishing, subscribing to, fetching, and relaying named streams of
objects.

The important ideas are:

- **namespace**: a grouping prefix, similar to a path prefix or topic family
- **track**: one named stream within a namespace
- **object**: one unit of delivered data on that track
- **group / subgroup**: structure used to organize and deliver objects
- **publisher**: the side making track data available
- **subscriber**: the side asking for that data
- **relay**: something in the middle that can subscribe upstream and publish downstream

If an HTTP analogy helps:

- a namespace is a bit like a route prefix
- a track is a bit like a named resource that keeps changing over time
- a subscription is not a one-shot `GET`; it is more like "keep sending me future objects"
- a fetch is more like "give me a bounded historical range"

That analogy is only partial.
MOQT is not HTTP with different names.
It is a protocol specifically shaped for object delivery, live updates, and relay behavior.

## What this repository is trying to become

`mqxx` is being built toward a C++ standalone MOQT relay, publisher, and subscriber
implementation with behavior aligned to
Moxygen's core runtime model.

That means the long-term goal is not just a few protocol structs.
The repo is aiming for a usable runtime model for:

- session control behavior
- publisher and subscriber APIs
- relay forwarding
- cache seams
- sample non-media applications

The pinned protocol baseline is `draft-ietf-moq-transport-18`.
That matters because MOQT is still evolving, and the code needs one concrete target instead of
"whatever the latest draft says this week."

## What is actually implemented today

Today the repository has **foundations**, not a full MOQT stack.

Implemented now:

- full track name data types
- text rendering and parsing for serialized names
- prefix matching for namespaces
- a small in-memory namespace registry
- internal role-based interface types for publisher/subscriber-style runtime seams
- relay/cache seam interfaces
- a fake session boundary for deterministic tests

Not implemented yet:

- real QUIC connections
- MOQT setup handshake
- subscribe / subscribe-update / fetch message logic
- object delivery state machines
- relay forwarding logic
- cache behavior beyond interface seams
- native QUIC or WebTransport adapters

So the current code is best read as:

- protocol model groundwork
- runtime seam groundwork
- test harness groundwork

It is **not** yet a working relay.

## The most concrete part today: track names

The most complete current implementation is in:

- [mqxx/moqt/full_track_name.hpp](../mqxx/moqt/full_track_name.hpp)
- [mqxx/moqt/full_track_name.cpp](../mqxx/moqt/full_track_name.cpp)

### Why track names matter early

Before a subscriber can ask for anything, both sides need to agree on what is being named.
Track naming is one of the first protocol concepts that is:

- central to the whole system
- easy to specify precisely
- easy to unit test
- transport-independent

That makes it a good first implementation target.

### What a full track name looks like here

A runtime track name is:

- a `track_namespace`, which is a vector of binary fields
- a `track_name`, which is another binary field

The code treats these as **binary-safe values**, not normal text strings.
That is important because MOQT names are protocol data, not just friendly UI labels.

### How serialization works

This repo implements a text form that is safe for debugging, tests, and deterministic comparisons.

Important details from the parser/renderer:

- literal bytes are ASCII letters, digits, and `_`
- other bytes are escaped as `.xx` using lowercase hex
- namespace fields are joined with `-`
- the namespace and the leaf track name are separated by `--`

Example:

- namespace fields: `example.com`, `meeting_7`
- track name: `camera-main`
- serialized form: `example.2ecom-meeting_7--camera.2dmain`

### Validation rules already enforced

The parser is intentionally strict.
It rejects:

- missing `--` separator
- empty namespace fields
- invalid hex escapes
- uppercase hex escapes
- redundant escapes for bytes that should have been left literal

It also enforces two limits in code today:

- at most 32 namespace fields
- at most 4096 total binary bytes across the full track name

Those rules are already useful even before wire-format control messages exist, because they give
the repo one precise, testable piece of protocol behavior.

## Namespace registry: a small helper, not the final discovery system

The namespace helper lives in:

- [mqxx/moqt/namespace_registry.hpp](../mqxx/moqt/namespace_registry.hpp)
- [mqxx/moqt/namespace_registry.cpp](../mqxx/moqt/namespace_registry.cpp)

It currently does three things:

- record an announced track
- withdraw a track
- return tracks whose namespace starts with a prefix

Two details matter:

- duplicate announces are rejected
- stored tracks are kept in deterministic serialized-name order

That second choice is not because the protocol requires alphabetical ordering.
It is there because deterministic order makes tests simpler and more trustworthy.

This type is **not** meant to become the central abstraction of the repo.
It is just a helper that future discovery and relay logic can reuse.

## The internal MOQT runtime seams already exist as interfaces

The main runtime seam headers are:

- [mqxx/moqt/session_types.hpp](../mqxx/moqt/session_types.hpp)
- [mqxx/moqt/session_roles.hpp](../mqxx/moqt/session_roles.hpp)
- [mqxx/moqt/relay_seams.hpp](../mqxx/moqt/relay_seams.hpp)

This is where the repo starts to look like the system it wants to become.

### Why define these interfaces before the protocol state machines?

Because the repo wants to avoid painting itself into a corner.
If the runtime seam model is wrong, later control-plane code and relay code will have to be
rewritten around it.

So the repository defines the *shape* of the system early:

- `publisher`
- `subscriber`
- `subscription_handle`
- `fetch_handle`
- `publish_namespace_handle`
- `track_consumer`
- `subgroup_consumer`
- `fetch_consumer`

And it defines the basic request/delivery structs those roles need:

- subscription requests
- fetch requests
- publish-namespace requests
- track status
- object location and object message
- group/subgroup boundaries
- reset signals

What is missing is the behavior behind those interfaces.

That is deliberate.
The repo is saying:

"These are the seams we want future code to fit into."

## The session boundary: how protocol code will avoid transport leakage

The transport-facing seam lives in:

- [mqxx/transport/session.hpp](../mqxx/transport/session.hpp)
- [mqxx/transport/fake_session.hpp](../mqxx/transport/fake_session.hpp)
- [mqxx/transport/fake_session.cpp](../mqxx/transport/fake_session.cpp)

### Why not expose QUIC details everywhere?

Because protocol logic and transport plumbing change for different reasons.

If every protocol state machine directly depends on one concrete QUIC stack, the code quickly
becomes harder to test and harder to adapt.

So this repository introduces a lower-level `session` interface that models the things MOQT cares
about:

- sending control bytes
- opening uni streams
- sending stream data
- sending datagrams
- receiving inbound events
- seeing resets and shutdown
- receiving flow-control and delivery notifications

That is a more useful seam than a "just give me a socket" abstraction, but still lower-level than
the `publisher` / `subscriber` runtime seam.

### Why the fake session exists now

`fake_session` is one of the most important architectural pieces even though it is small.

It gives tests a deterministic in-memory stand-in for a network session:

- it records outbound control writes
- it records outbound uni-stream writes
- it records outbound datagrams
- it can queue inbound events in FIFO order
- it can simulate datagram support being absent
- it can simulate the send side being closed

This is how the repo plans to test future MOQT behavior without needing real QUIC for every test.

That matters because protocol state machines are easier to validate when tests can say:

- "peer sent this control message"
- "then this uni-stream payload arrived"
- "then the session shut down"

without needing packet timing, certificates, or a real event loop.

## Why the code is structured this way

A newcomer might reasonably ask:

"Why not just build the real relay first?"

The answer is that distributed transport code becomes messy fast.
This repo is trying to build in an order that keeps the next step testable.

### Reason 1: precise protocol pieces first

Track names are a crisp piece of protocol behavior.
They let the project prove:

- binary-safe data modeling
- strict parsing discipline
- deterministic tests

before larger control flows are introduced.

### Reason 2: runtime seams before runtime complexity

The repo wants the main runtime model to be:

- role-based
- relay-friendly
- transport-agnostic

Defining those interfaces early is a way of making later changes converge on the intended design.

### Reason 3: fake-session seam before real transport

Real QUIC integration brings:

- TLS setup
- socket integration
- event loops
- platform behavior
- timing-related complexity

If the first tests depend on all of that, debugging the protocol becomes much slower.

So the repo builds the fake-session seam first, then intends to plug real adapters into the same
shape later.

## What "Moxygen-aligned" means here

This repo is not trying to copy Moxygen file-for-file.
It is trying to align with the **behavioral envelope** of Moxygen's core MOQT runtime model.

That means aiming for the same kinds of system responsibilities:

- sessions that exchange MOQT control behavior
- symmetric publisher/subscriber roles
- relays that are both downstream publishers and upstream subscribers
- seams where cache and forwarding logic can live

What it does **not** mean is:

- adopting Meta-specific internal libraries everywhere
- inheriting Moxygen's exact code organization
- implementing browser/media tooling first

## What should happen next

The next major implementation steps should be control-plane behavior, not unrelated polish.

The docs and current interfaces point toward:

1. session setup and setup-response state machines
2. publish-namespace and withdraw behavior
3. subscribe / subscribe-update / unsubscribe behavior
4. fetch behavior
5. track status flows
6. teardown and reset handling

After those become stable behind fake-session tests, the repo can grow into:

- deeper object/group delivery logic
- real relay forwarding logic
- cache behavior
- native QUIC and WebTransport-oriented adapters
- sample publisher/subscriber/relay apps

## If you are new, what to read next

Suggested reading order:

1. [README.md](../README.md)
2. this guide
3. [docs/architecture.md](architecture.md)
4. [docs/protocol-notes.md](protocol-notes.md)
5. [docs/code-explanation.md](code-explanation.md)
6. the current code for:
   [mqxx/moqt/full_track_name.cpp](../mqxx/moqt/full_track_name.cpp),
   [mqxx/moqt/namespace_registry.cpp](../mqxx/moqt/namespace_registry.cpp), and
   [mqxx/transport/fake_session.cpp](../mqxx/transport/fake_session.cpp)

If you keep one mental model in your head while reading, use this one:

- MOQT is about named object streams over QUIC
- this repository has started by defining the names, seams, and tests
- the real protocol behavior is the next layer to build on top of those seams
