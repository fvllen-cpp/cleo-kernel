# Cleo Kernel

## Core Philosophy

The Cleo kernel treats the system as a flow graph rather than a collection of independent threads. 
The kernel schedules computational nodes connected by channels. Instead of shared-memory 
synchronization and mutex-heavy programming, communication occurs through explicit message passing 
and ownership transfer. 

The kernel is fundamentally pipeline-oriented.

Example pipelines:
Camera -> preprocessing -> AI inference -> planner -> motor controller
IMU -> attitude estimation -> flight controller -> actuators

## Design Goals

Ideally, this model should enable:
- deterministic execution ordering
- replayable execution and traceability
- explicit data dependencies
- reduced synchronization complexity
- strong isolation between components
- specialized workloads over general-purpose desktop compatibility

## Concurrency Model

Traditional OSes expose preemptive threads as the primary execution primitive. Threads share memory,
synchronize through locks, and are scheduled independently or are dynamically scheduled based on the 
resource access protocol in place. This can create hidden coupling and non-determinism around 
high contention resources. 

Instead of the more traditional model, Cleo uses:
- nodes
- channels
- lightweight execution contexts
- event-driven scheduling

A node represents a unit of computation. 
A channel represents a dependency edge between nodes. 

Nodes execute when: 
- input messages are available
- timing constraints permit execution
- dependencies are satisfied

Threads still exist internaly but are considered implementation details of the scheduler rather 
than application-visible abstractions. 

Cleo intentionally avoids exposing:
- mutexes
- condition variables 
- semaphores
to application level software

Applications instead communicate through:
- mailboxes 
- streams 
- typed channels
- lock-free queues

## Why Message Passing

Message passing was selected because it creates explicit data flow and significantly simplifies 
reasoning about concurrency. 
I also didn't want to build another toy kernel that's a worse linux/zephyr copy.

Advantages:
- Isolation: components cannot accidentally corrupt shared state
- Replayability: message ordering can be logged and reproduced
- Scalability: reduced lock contention on multicore systems
- Fault containment: crashes are localized to services or nodes
- Temporal predictability: IPC timing is easier to analyze than arbitrary lock interactions
- Cache Behavior: explicit ownership transfer reduces false sharing. pipelining can increase cache
    locality across nodes.

## Scheduling Architecture

We use a graph-aware scheduler

Scheduling unit:
- Graph node

Execution unit: 
- lightweight execution context (fiber/coroutines)

Scheduling policy:
- Earliest deadline first (EDF) + Constant Bandwidth Server (CBS) to start
- Long term plan: CORD — Eisenklam et al., arXiv 2501.08484, January 2025 — "Co-design of Resource 
Allocation and Deadline Decomposition with Generative Profiling." + Mixed-Criticality Scheduling — 
Baruah et al., 2012 — "Scheduling Real-Time Mixed-Criticality Jobs."

Node metadata:
- execution budget
- deadline
- periodicity
- jitter tolerance
- CPU affinity
- memory locality hints

The scheduler operates on dependency graphs rather than independent threads.

Advantages:
- better cache locality
- natural pipelining
- easier deadline analysis
- reduced context-switch overhead

## Deterministic Replay System

The replay subsystem is a core feature.

The kernel records:
- interrupt arrival ordering
- scheduling decisions
- IPC events
- timer events
- capability transfers

Replay goals:
- reproduce race conditions
- debug missed deadlines
- inspect timing anomalies
- deterministic debugging

Implementation strategy:
- per-core lock-free trace buffers
- timestamped event streams 
- offline merge during replay

## Kernel Synchronization Primitives

Primary synchronization primitives:

1. Spinlocks
- short critical sections
- interrupt-safe metadata protection
- scheduler queues

2. Sequence Locks
- timestamp reads
- mostly-read structures

3. RCU-style mechanisms
- read heavy tables
- graph topology updates 

4. Atomic operations
- lock-free queues
- reference counting
- owndership transfers

5. Per core structures
- minimizes contention

Application-visible synchronization:
- channels 
- event queues
- bounded mailboxes

Primitives that will be avoided:
- global kernel mutexes
- blocking semaphores as primary coordination
- coarse-grained scheduler locks
Reason: 
These become scalability bottlenecks on multicore systems

## Kernel Data Structures 

1. Intrusive linked lists
- no heap allocation overhead
- predictable memory layout

2. Lock-free ring buffers
- IPC channels 
- trace logging 
- streaming pipelines

3. Fixed-capacity hash tables
- capability lookup
- object registries

4. Slab allocators
- kernel objects
- channel descriptions
- scheduler entities

5. DAG representations
- dependency scheduling
- execution graphs

Structures that will be avoided:
- STL containers with unrestricted allocation
- dynamically growing queues
- trees structures with unpredictable balancing costs

## Memory Management Architecture

Still debating virtual memory vs flat physical address spaces. 
But we will go with virtual memory to start because:

1. Isolation - services require protection boundaries
2. Capability enforcement - virtual mappings simplify access control 
3. Zero-copy IPC - pages can be remapped between address spaces

Architecture: 
- 4KB seems to be the sweet spot in literature
- optional huge pages later
- kernel mapped in higher-half memory

Physical memory management:
 - buddy allocator for page allocation
 - slab allocators layered on top

Virtual memory manager:
- page tables per address space
- capability backed mappings

Memory ownership model:
- transfer semantics over copying
- region-based allocation for streaming workloads
- PMR allocators to runtime

Zero-copy strategy:
this might be cooked but:
- ownership of pages transfers through channels
- mappings are updated
- reference counts are adjusted

## Userspace Runtime

The userspace runtime provides:
- graph APIs
- coroutine execution
- typed channels
- allocator integration
- tracing hooks

Coroutines are preferred because:
- lower overhead than threads
- natural async semantics
- efficient even-driven execution

## Risks
- Existing POSIX software will not run easily or at all (yet)
- IPC overhead - message passsing can cost more than direct shared memory for some workloads
- Full determinism across many cores is hard
- User-space drivers required robust IPC performance
- Debugging the kernel itself 

## Visualization
- The goal is to interface with Perfetto to allow for trace, replay, and debug visualization
