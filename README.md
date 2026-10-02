# Operating Systems Project

This project contains implementations of two operating-system topics:

1. Process Management
2. Memory Management

It includes both Python and C++ versions so you can compare implementations.

## Repository contents

- `process_management.py` – Python demonstration of process creation, synchronization, termination, and scheduling.
- `memory_management.py` – Python memory-allocation simulation using First-Fit, Best-Fit, and Worst-Fit.
- `process_management.cpp` – C++ version using POSIX process-management primitives (`fork`, `kill`, `waitpid`, `pipe`, `sleep`).
- `memory_management.cpp` – C++ version simulating memory allocation and deallocation with three algorithms.
- `CMakeLists.txt` – builds both C++ programs.

## Build and run the C++ programs

```bash
mkdir -p build
cd build
cmake ..
make
./process_management
./memory_management
```

## Build and run the Python programs

```bash
python3 process_management.py
python3 memory_management.py
```

## Assignment coverage

- Process creation: implemented via `fork()`
- Execution: child processes perform work and report status
- Termination: `SIGTERM` and `waitpid()` demonstration
- Scheduling: round-robin style stop/resume using `SIGSTOP` and `SIGCONT`
- Synchronization: pipe-based start signal
- Memory management: First-Fit, Best-Fit, Worst-Fit comparison

This project was created to satisfy the operating systems assignment requirements.
