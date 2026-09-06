# 🚀 Kedis-C

**A Redis-inspired in-memory datastore written in C to explore storage engines, networking, persistence, concurrency, and systems architecture.**

> Built to understand systems, not just use them.

---

# 📖 Overview

Kedis-C is a Redis-inspired in-memory datastore being implemented from scratch in C.

The project is the lower-level evolution of **Kedis-Python**, an earlier Python prototype used to experiment with Kedis's architecture, command model, networking, persistence, transactions, replication, and data structures.

Kedis-C takes those ideas into a systems-oriented implementation where memory management, data layout, networking, I/O, concurrency, and failure handling are explicit parts of the design.

Rather than attempting to reproduce Redis feature-for-feature, Kedis focuses on understanding the architectural and engineering principles behind high-performance backend systems.

---

# 🧬 Project Evolution

Kedis began as a Python prototype:

```text
Kedis-Python
     │
     ├── Experiment with architecture
     ├── Validate command semantics
     ├── Explore persistence
     ├── Explore networking
     ├── Explore transactions
     └── Explore replication
             │
             ▼
          Kedis-C
             │
             ├── Explicit memory management
             ├── Low-level data structures
             ├── Socket networking
             ├── Persistence
             ├── Concurrency
             └── Systems-level performance
```

The Python implementation serves as the experimentation platform.

Kedis-C is the deeper systems implementation.

---

# 🎯 Design Goals

Kedis-C is being built to explore:

* storage engine design
* command-driven architectures
* networking fundamentals
* custom wire protocols
* persistence strategies
* memory management
* data structure implementation
* concurrency
* failure recovery
* systems performance
* database architecture

The goal is not Redis compatibility.

The goal is understanding.

---

# 🏗️ Architecture

The system is being developed as a collection of independent subsystems:

```text
                         Client
                           │
                           ▼
                    ┌──────────────┐
                    │   Network    │
                    │     Layer    │
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
                    │   Protocol   │
                    │    Parser    │
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
                    │   Command    │
                    │    Engine    │
                    └──────┬───────┘
                           │
                           ▼
                  ┌───────────────────┐
                  │   Storage Engine  │
                  └─────────┬─────────┘
                            │
             ┌──────────────┼──────────────┐
             ▼              ▼              ▼
          Indexing      Persistence    Transactions
                            │
                            ▼
                           WAL
```

The architecture will evolve as individual components are implemented, tested, benchmarked, and integrated.

---

# 📦 Planned Data Structures

Kedis-C is designed to use specialized data structures depending on the requirements of each subsystem.

Potential components include:

* Hash tables
* Trees
* Skip lists
* Dynamic arrays
* Linked structures
* Custom indexing structures

Reusable low-level structures can be provided by **CDSA**, the accompanying C data-structures library.

---

# 🔌 Protocol

Kedis-Python introduced **KESP**, Kedis's custom binary-safe wire protocol.

Kedis-C will provide a low-level implementation of the protocol rather than depending on an existing database protocol.

The protocol is designed around explicit framing and byte lengths so that message boundaries are deterministic.

---

# 💾 Persistence

Persistence is one of the major areas explored by Kedis.

The Python prototype currently uses Append-Only File (AOF) persistence.

Kedis-C will use this experience as a basis for exploring lower-level:

* sequential writes
* buffering
* synchronization
* recovery
* crash consistency
* write-ahead logging

---

# 🌐 Networking

The Python prototype uses `asyncio` for concurrent client connections.

Kedis-C will move the networking layer into C and explicitly handle:

* sockets
* connection management
* message framing
* I/O
* concurrent clients

The goal is to understand what high-level asynchronous networking abstractions are doing underneath the hood.

---

# 🔄 Transactions

The Python prototype supports transactional execution using:

```text
MULTI
SET a 1
SET b 2
EXEC
```

It also explores optimistic locking through `WATCH` / `UNWATCH`.

Kedis-C will use these experiments as a foundation for exploring transaction processing and concurrency at a lower level.

---

# 🔁 Replication

Kedis-Python includes master-replica replication:

```text
              ┌──────────────┐
              │    Master    │
              └──────┬───────┘
                     │
              ┌──────┴──────┐
              ▼             ▼
        ┌───────────┐ ┌───────────┐
        │  Replica  │ │  Replica  │
        └───────────┘ └───────────┘
```

Replication in Kedis-C is intended to explore:

* state transfer
* write propagation
* connection management
* consistency
* failure handling

---

# 🧠 Memory Management

Unlike the Python implementation, Kedis-C explicitly manages memory.

This makes memory management a first-class part of the project.

Areas of exploration include:

* allocation and deallocation
* ownership
* lifetime management
* memory layout
* fragmentation
* allocation performance

Where appropriate, Kedis-C can leverage the allocator and data-structure infrastructure developed in CDSA.

---

# ⚡ Performance

Performance measurements will be treated as part of the engineering process rather than as a single headline number.

Benchmarks will be used to investigate:

* command throughput
* latency
* networking overhead
* persistence overhead
* allocation behavior
* data-structure performance
* concurrency scaling

Results will be documented alongside their workload and configuration.

---

# 🧪 Testing

Kedis-C will use multiple layers of testing:

* unit tests
* integration tests
* protocol tests
* persistence/recovery tests
* stress tests
* fuzz testing
* benchmarks

Low-level systems code can fail in ways that ordinary functional tests don't expose, so fuzzing and stress testing are an important part of the development process.

---

# 🛣️ Roadmap

## Phase 1 — Foundation

- [ ] Project structure
- [ ] Build system
- [ ] Core data model
- [ ] Memory management
- [ ] Basic command engine
- [ ] Unit testing infrastructure

## Phase 2 — Networking

- [ ] Socket server
- [ ] Client connections
- [ ] KESP parser
- [ ] Request/response handling
- [ ] Concurrent clients

## Phase 3 — Storage

- [ ] Storage engine
- [ ] Indexing
- [ ] Data structures
- [ ] TTL
- [ ] Memory management

## Phase 4 — Persistence

- [ ] WAL
- [ ] Buffered writes
- [ ] Recovery
- [ ] Crash testing
- [ ] Persistence benchmarks

## Phase 5 — Advanced Systems

- [ ] Transactions
- [ ] Concurrency control
- [ ] Replication
- [ ] Failure handling
- [ ] Performance optimization

---

# 🧱 Relationship With CDSA

Kedis-C is being developed alongside **CDSA**, a reusable C data-structures library.

The separation is intentional:

```text
CDSA
 │
 ├── Data structures
 ├── Allocators
 └── Generic low-level utilities
             │
             ▼
          Kedis-C
             │
             ├── Storage engine
             ├── Networking
             ├── Protocol
             ├── Persistence
             └── Concurrency
```

This allows Kedis-C to focus on database and systems architecture instead of reimplementing generic infrastructure inside the database itself.

---

# 📚 What Kedis Is Teaching

Kedis is ultimately a systems-learning project.

Through Kedis-Python and Kedis-C, the project explores:

* how databases organize state
* how commands become operations
* how clients communicate with servers
* how data survives process failure
* how indexes affect performance
* how concurrency introduces new failure modes
* how memory management affects systems
* how architectural decisions create tradeoffs

---

# 🤝 Contributing

Suggestions, issues, discussions, and contributions are welcome.

Kedis is primarily a learning and systems-engineering project, but contributions and experimentation are encouraged.

---

# 👨‍💻 Author

**V SS Karthik**

AI/ML Student • Systems Enthusiast • Builder of developer tools and infrastructure projects

> "Built to understand systems, not just use them."

---

# 📜 License

MIT License

Copyright (c) 2026 V SS Karthik
