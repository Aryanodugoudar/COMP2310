# <p style="text-align: center;">COMP2310 Assignment 1</p> 
##### <p style="text-align: center;">Aryan Odugoudar - u7689173</p>

## Introduction

This report discusses the implementation of `mymalloc.c` and `mygc.c`, encompassing a custom memory allocator and garbage collector. The assignment's specifications guided this implementation.

## 1. Implementation
A metadata structure, Header, serves as the backbone:
```markdown
typedef struct Header { 
    size_t size;
    size_t left_size;
    struct Header *next;
    struct Header *prev;
} Header;
```

### &emsp; Multiple Explicit Free Lists

- A global array stores heads for various free lists, each employing a doubly linked list structure. The allocation determines the free list index using:
```
idx = min(aligned_Requested_Size / unit_Size, NLISTS - 1)
```

- After finding the free list using the index, the `fit_list()` function is called to find the fitting block using a first-fit strategy. If the fit block is found, it is removed from the free list and an attempt is made to split it using `split_block()`

### &emsp; Fence Posts

- For each consistent chunk, a dummy block is placed at the end to mark the end of the chunk. In the metadata, there is a field called `left_size`, which records the size of the neighboring block in a consistent chunk.

- In a consistent chunk, the `left_size` field in the first block is always 0, and the last block is always a dummy (size field is always 0), while any other block in the same chunk has a non-zero `size` and `left_size`. This allows for easy detection of the boundary of each consistent chunk,preventing segmentation faults caused by coalescing free blocks without boundary awareness.

### &emsp;  Constant Time Coalescing with Boundary Tags

- After calling the `myfree()` function and a block is freed, the `size` and `left_size` fields are used to obtain the header pointers of left and right blocks. If the neighboring block is free, the `size` or `left_size` fields are updated, and the consistent free blocks are merged.

## 2. Optimizations

### &emsp;  Metadata Reduction

- As the size of each block is aligned to one word size, the least significant bit of the size field is used to record whether the block is free or not. Additionally, when a block is allocated, it takes over the space of two pointers.

### &emsp; Requesting Additional Chunks from the OS

- If no block fits in this free list, the `init_list()` function is called to request a new chunk from the OS. After requesting a new chunk, it is initialized as a new block and added to the free list.

### &emsp; Optimize Large Allocation Requests

- Allocation sizes less than kLargeAllocationSize use free lists, while larger requests use `mmap()`. The left_size metadata records block types, guiding `myfree()` for proper deallocation.

## 3. Garbage Collector
The implemented garbage collector (GC) in this C program is designed to automate memory management efficiently. It consists of several key functions:

### &emsp; my_malloc_gc(size_t size):
- This function is an extension of the standard memory allocation function, `my_malloc`. It allocates memory for a given size and returns a pointer to the allocated memory.

- Additionally, it creates a data structure (GCBlock) to track the allocated memory block. This block structure contains information about the allocated memory's pointer, whether it has been marked as reachable (initially marked as false), and a reference to the next block in a linked list.

### &emsp; my_free_gc(void *ptr):
- This function complements `my_malloc_gc`. It takes a pointer to previously allocated memory and performs two crucial actions:

     - It marks the corresponding GCBlock as "unreachable" by setting the `marked` flag to false.

     - It calls my_free to release the memory.

     - After successfully freeing the memory, it removes the GCBlock structure from the linked list.

### &emsp; get_end_of_stack():
- Retrieves the stack's end, an upper memory boundary for marking.

### &emsp; mark_memory(void *ptr):
- Marks allocated memory as "reachable." Recursively marks pointers within, ensuring correct identification.

- It then iterates through the memory block, identifying and marking any pointers within it that are also considered reachable. This recursive marking ensures that all reachable memory is correctly identified.

### &emsp; my_gc():
- The main garbage collection function. It initiates the garbage collection process.

- It starts by identifying the end of the stack using `get_end_of_stack()`.

- It then proceeds to mark all reachable memory, starting from the end of the stack. The mark_memory function is used to traverse the memory blocks, marking each as reachable if it is part of the program's active memory graph.

- After marking all reachable memory, the my_gc function proceeds to free unmarked memory blocks. Any GCBlock with a `marked` flag set to false represents memory that is no longer in use and is safe to release using `my_free`.

- Finally, the GCBlock structures associated with freed memory blocks are also removed from the linked list.



## 4. Two Implementation Challenges

- One challenge was the use of VSCode to edit the code, which introduced bugs and issues with pointers, resulting in segmentation faults. The VSCode debug mode couldn't locate the line with the error, so I used the GDB debugger to identify and resolve the error.

- Another challenge was detecting whether the pointer to be freed is in the range allocated by `mymalloc()`, especially when implementing multiple free lists and optimizing chunk requests. To address this, a `Range_Mark` struct was created to record the range of each free list, functioning as a single linked list without a head.

## 5. Two Key Observations

- During testing of `random_free.c`, an error was encountered because there was no check for a case where the test case attempted to call `my_free` with a memory address that wasn't allocated by `my_malloc`. To address this, a function called `is_my_malloc_address` was implemented to handle such test cases.

- To detect if a pointer is in the allocated range when attempting to free it, I tried to install a signal handler to detect `SIGSEGV`. However, this method took an average of 1.5 seconds in the default benchmark. Instead, the `Range_Mark` method was used, which took an average of 0.5 seconds.

## Conclusion
This implementation offers a custom memory allocator featuring explicit free lists, coalescing, and large allocation optimizations. It adeptly handles error cases, ensuring sound memory management, while Range_Mark aids in preventing invalid deallocations.


