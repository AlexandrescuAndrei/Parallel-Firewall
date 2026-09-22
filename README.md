# Parallel Packet-Processing Firewall in C

A multithreaded packet-processing application written in **C** using **POSIX threads**.

The program reads packets from an input file, processes them concurrently using multiple worker threads, and writes the results to an output file. Each packet is classified as either `PASS` or `DROP`, assigned a hash, and associated with its original timestamp.

The project focuses mainly on multithreading, synchronization, and the producer-consumer model.

## How It Works

Packets are read from the input file by a producer and inserted into a shared ring buffer.

Several consumer threads run in parallel and continuously retrieve packets from this buffer. Each consumer processes a packet independently by determining whether it should be accepted or dropped and by computing its hash.

The final result written for each packet contains:

- the firewall decision (`PASS` or `DROP`)
- the packet hash
- the packet timestamp

Packets have a fixed size of 256 bytes and contain a source address, destination address, timestamp, and payload.

The number of consumer threads can be selected when starting the program, with support for up to 32 consumers.

## Concurrency and Synchronization

The shared ring buffer is protected using POSIX synchronization primitives.

A `pthread_mutex_t` protects access to the buffer, while condition variables are used to coordinate the producer and consumers:

- consumers wait when the buffer is empty
- the producer waits when the buffer is full
- waiting threads are notified when new data becomes available or buffer space is released

This avoids busy waiting and allows the threads to sleep until they can continue useful work.

The producer signals the end of the input stream once all packets have been inserted, allowing the consumer threads to finish processing the remaining packets and terminate cleanly.

## Ordered Output

Packet processing happens in parallel, which means different consumer threads may finish their work in a different order.

The output still needs to follow the expected packet ordering, so the consumers use additional synchronization before writing results to the output file.

This allows the expensive packet-processing work to happen concurrently while keeping the final output deterministic.

## Packet Filtering

Each packet is checked against a set of allowed source-address ranges.

If the packet source belongs to one of the accepted ranges, the packet is marked as:

```text
PASS
```

Otherwise it is marked as:

```text
DROP
```

A hash is also calculated for every packet using its complete 256-byte contents.

The output format is:

```text
ACTION HASH TIMESTAMP
```

## Serial and Parallel Versions

The repository contains both a parallel and a serial implementation.

The parallel version uses a producer thread workflow together with multiple consumer threads and a synchronized ring buffer.

The serial version processes packets one at a time without worker threads.

Having both versions makes it possible to compare a straightforward sequential implementation with the multithreaded producer-consumer approach.

## Project Structure

- `firewall.c` - main parallel application and consumer thread coordination
- `producer.c` / `producer.h` - reads packets and publishes them to the ring buffer
- `consumer.c` / `consumer.h` - creates worker threads and processes packets
- `ring_buffer.c` / `ring_buffer.h` - synchronized shared ring buffer
- `packet.c` / `packet.h` - packet structure, filtering rules, and hashing
- `serial.c` - sequential version of the packet processor
- `Makefile` - build configuration

## Build

The project uses GCC, GNU Make, and POSIX threads.

```bash
make
```

This builds two executables:

```text
firewall
serial
```

The parallel version is started with:

```bash
./firewall <input-file> <output-file> <num-consumers>
```

where `num-consumers` must be between 1 and 32.

The serial version is started with:

```bash
./serial <input-file> <output-file>
```

The current Makefile also expects the shared utility and logging files referenced through `UTILS_PATH`.

## Technologies and Concepts

- C
- POSIX Threads (`pthread`)
- Mutexes
- Condition variables
- Producer-consumer pattern
- Ring buffers
- Multithreading
- Thread synchronization
- Shared-memory communication
- Low-level file I/O
- GCC
- GNU Make
