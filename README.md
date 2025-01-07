# Unsafe-Lua
### NOTE: ⚠️ This project is not working as expected, as it relies on the GC of Lua. It need some changes to work properly as a manual memory management for Lua

Unsafe-Lua is a Lua C library that provides low-level memory management capabilities directly accessible from Lua scripts. It allows for manual allocation, deallocation, and manipulation of memory, making it particularly useful for scenarios where performance and control over memory are critical.

## Features

- **Heap Memory Management:**
  - Allocate (`heap_malloc`) and free (`heap_free`) memory on the heap.
  - Access and modify heap-allocated memory via pointers.

- **Arena Allocator:**
  - Efficient memory allocation using a pre-allocated block of memory.
  - Allocate (`arena_alloc`), reset (`arena_reset`), and free (`arena_free`) the memory arena.

- **Stack Allocator:**
  - Push and pop memory blocks in a stack-like structure.
  - Reset (`stack_reset`) and free (`stack_free`) the memory stack.

- **Pointer Manipulation:**
  - Read and write raw memory using pointers.
  - Check if a pointer is null.

