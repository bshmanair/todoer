# todoer

A super-fast zero-allocation-focused command-line task management utility built with modern **C++23/26** and **simdjson**.

This app leverages cache-aligned data structures, data continuity principles and strict RAII lifecycle design.

[Project inspiration (click here)](https://roadmap.sh/projects/task-tracker)

## Performance and Architectural Highlights
*   **Zero-Copy Argument Parsing:** Utilizes `std::string_view` across the entire command layer to completely eliminate redundant heap allocations when reading CLI inputs.
*   **Cache-Aligned Memory:** The internal `Task` object properties are explicitly ordered by memory alignment restrictions (largest to smallest) to strip away up to 73% of compiler-inserted padding bytes.
*   **O(1) Incremental Continuity:** Task keys utilize monotonic continuity. New tasks calculate the next unique tracking ID in strict constant time ($O(1)$) by fetching the last element of the underlying container, maintaining timeline consistency even if prior tasks are deleted.
*   **Optimal Lookup Mechanics:** Uses C++23's `std::flat_map` to keep tasks tightly packed contiguously in memory, yielding massive L1/L2 CPU cache line hits during high-frequency $O(\log n)$ queries and mutations.
*   **Single-Pass I/O:** Leverages `simdjson`'s structural validation to digest and structurally parse the underlying JSON task files in a single pass over disk memory.

## Requirements and Tooling

To build this project, ensure you have the following installed:

*   **Compiler:** Clang 18+ or GCC 14+ (Fully supporting `-std=c++26`)
*   **Build System:** CMake (>= 3.28) and Ninja
*   **Libraries:** `simdjson`

## Building the Project

The workspace includes a structured developer `Makefile` mapping straight into optimized `Ninja` targets:

### Standard Build
Build the binary with optimization flags and generate automatic LSP compilation databases:

```bash
make
```