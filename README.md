# MPI

This repository contains a simple example program, `async_double_buffer.cpp`,
which demonstrates asynchronous point-to-point communication using MPI. The
program sends messages between two ranks with a double-buffering scheme and now
also fragments each large buffer into 4&nbsp;KiB pieces before sending.

## Build

Compile the example with an MPI C++ compiler wrapper such as `mpic++`:

```bash
mpic++ -std=c++17 -o async_double_buffer async_double_buffer.cpp
```

## Usage

Run the program with at least two ranks:

```bash
mpirun -np 2 ./async_double_buffer
```

The sender transmits a 1&nbsp;MB buffer in 4&nbsp;KiB fragments over ten
iterations, and the receiver prints the first byte of each iteration to verify
the transfer.
