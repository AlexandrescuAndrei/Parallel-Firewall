# Parallel Packet-Processing Firewall in C

A multithreaded packet-processing application written in **C** using **POSIX threads**.

The project implements a producer-consumer system in which packets are read from an input file, placed into a shared ring buffer, and processed concurrently by multiple consumer threads. Each packet is analyzed, classified as either **PASS** or **DROP**, assigned a hash, and written to an output file together with its timestamp.

The main focus of the project is concurrent programming in C, especially thread synchronization, shared data structures, condition variables, and coordinating multiple workers while preserving the correct order of the final output.

# How It Works

Packets have a fixed size of 256 bytes and contain a source address, destination address, timestamp, and payload.

A producer reads these packets from the input file and publishes them into a shared ring buffer. Several consumer threads continuously retrieve packets from the buffer and process them independently.

For every packet, a consumer applies the filtering logic, computes a hash based on the packet contents, and prepares the result that will be written to the output file.

The number of consumers is configurable when the application is started, with support for between 1 and 32 worker threads.

# Concurrency and Synchronization

The ring buffer is shared between the producer and all consumer threads, so access to it is synchronized using POSIX thread primitives.

A `pthread_mutex_t` protects the internal state of the buffer, while the `not_empty` and `not_full` condition variables coordinate the producer and consumers.

When the buffer is empty, consumers wait until new packets become available. When the buffer is full, the producer waits until one of the consumers removes data and creates free space. This allows the threads to block efficiently instead of continuously checking the state of the buffer.

After the producer reaches the end of the input file, it marks the ring buffer as stopped and wakes any waiting consumers. The consumers finish processing the packets that are still available and then terminate. The main thread waits for all of them using `pthread_join`.

# Packet Processing and Ordered Output

The firewall decision is based on the source address of each packet. The implementation contains a number of allowed source-address ranges, and packets are classified as **PASS** when their source belongs to one of these ranges. All other packets are classified as **DROP**.

A hash is also calculated for every packet using its complete contents.

Because multiple consumers process packets concurrently, they are not guaranteed to finish in the same order in which the packets were received. Additional synchronization is therefore used before writing the results so that the output remains ordered and deterministic.

This keeps the packet-processing work parallel while still producing the expected output order.

# Serial and Parallel Implementations

The repository contains both a parallel and a serial version of the packet processor.

The parallel implementation uses the producer-consumer model, a synchronized ring buffer, and multiple worker threads.

The serial implementation performs the same packet filtering and hashing operations one packet at a time, without using multiple threads.

Keeping both implementations in the project provides a straightforward sequential reference alongside the multithreaded solution.

# Project Structure

- `firewall.c` — main parallel application, initialization, and thread management
- `producer.c` / `producer.h` — reads packets from the input file and publishes them to the ring buffer
- `consumer.c` / `consumer.h` — creates consumer threads and handles packet processing and output synchronization
- `ring_buffer.c` / `ring_buffer.h` — synchronized ring buffer shared between the producer and consumers
- `packet.c` / `packet.h` — packet representation, filtering rules, and hashing
- `serial.c` — sequential implementation of the packet processor
- `Makefile` — build configuration

# Build and Run

The project is built using **GCC**, **GNU Make**, and the POSIX Threads library.

Running `make` builds both the `firewall` and `serial` executables.

The parallel version is executed using `./firewall <input-file> <output-file> <num-consumers>`, where the number of consumer threads must be between 1 and 32.

The serial version can be executed using `./serial <input-file> <output-file>`.

The current Makefile also references shared utility and logging files through the `UTILS_PATH` variable.

# Technologies and Concepts

- C
- POSIX Threads (`pthread`)
- Multithreading
- Producer-consumer pattern
- Mutexes and condition variables
- Ring buffers
- Thread synchronization
- Shared-memory communication
- Ordered concurrent processing
- Low-level file I/O
- GCC
- GNU Make
