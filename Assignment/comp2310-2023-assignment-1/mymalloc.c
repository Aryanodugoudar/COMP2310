#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/mman.h>
#include"mymalloc.h"

#define kNumSizeClasses 8
// Define the maximum allocation size
const size_t kMaxAllocationSize = (16ull<<20);

// Block structure
struct Block{
  size_t size;
  struct Block *next;
};

// Fence structure
struct Fence{
  size_t size;
  struct Block *next;
};

// Size class constants for multiple free lists
const size_t kSizeClass[kNumSizeClasses] = {16, 32, 64, 128, 256, 512, 1024, 2048};
struct Block *free_lists[kNumSizeClasses] = {NULL};

// Fence posts
struct Fence *start_fence;
struct Fence *end_fence;

size_t align_size(size_t size){
    if (size % kAlignment != 0){
        size += kAlignment - (size % kAlignment);
    }
    return size;
}

bool request_chunk(size_t size){
    size_t num_blocks = size / ARENA_SIZE;
    if(size % ARENA_SIZE != 0){
        num_blocks++;
    }

    // Use mmap for memory allocation
    struct Block *new_chunk = mmap(NULL, num_blocks * ARENA_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if(new_chunk == MAP_FAILED){
        perror("mmap");
        return false; 
    }

    for(size_t i = 0; i < num_blocks; i++){
        new_chunk[i].size = ARENA_SIZE;
        new_chunk[i].next = free_lists[0];
        free_lists[0] = &new_chunk[i];
    }
    return true;
}

void split_block(struct Block *block, size_t size){
    while (block->size > size * 2){
        struct Block *split = (struct Block *)((char *)block + block->size / 2);
        split->size = block->size / 2;
        split->next = free_lists[0];
        free_lists[0] = split;
        block->size /= 2;
    }
}

int get_size_class(size_t size){
    for(int i = 0; i < kNumSizeClasses; i++){
        if(size <= kSizeClass[i]) {
            return i;
        }
    }
    return kNumSizeClasses - 1;
}

void *my_malloc(size_t size){
    if(size == 0 || size > kMaxAllocationSize){
        return NULL;
    }

    size = align_size(size);

    int size_class = get_size_class(size);

    //(best-fit)
    struct Block *current = free_lists[size_class];
    struct Block *best = NULL;
    size_t best_diff = INT_MAX;

    while(current != NULL){
        size_t diff = current->size - size;
        if(diff < best_diff){
            best = current;
            best_diff = diff;
        }

        current = current->next;
    }

    if(best == NULL){
        size_t chunk_size = ARENA_SIZE;
        if(size > chunk_size){
            chunk_size = align_size(size);
        }

        if(!request_chunk(chunk_size)){
            return NULL;
        }
        struct Block *block = mmap(NULL, chunk_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
        block->size = size;
        block->next = free_lists[size_class];
        free_lists[size_class] = block;
        return((char *)block + sizeof(struct Fence));
    }

    split_block(best, size);

    return((char *)best + sizeof(struct Fence));
}


void merge_blocks(struct Block *block){
    int size_class = get_size_class(block->size);
    while (size_class < kNumSizeClasses - 1){
        uintptr_t buddy_address = ((uintptr_t)block ^ block->size);
        struct Block *buddy = (struct Block *)buddy_address;
        if(buddy->size == block->size && buddy->next == free_lists[size_class]){
            if(buddy < block){
                struct Block *temp = block;
                block = buddy;
                buddy = temp;
            }
            free_lists[size_class] = buddy->next;
            block->size *= 2;
            size_class++;
        } 
        else{
            break;
        }
    }

    block->next = free_lists[size_class];
    free_lists[size_class] = block;
}

void my_free(void *ptr){
    if (ptr == NULL){
        return;
    }
    struct Block *block = (struct Block *)((char *)ptr - sizeof(struct Fence));
    if(block->size == UINT_MAX){
        return;
    }
    merge_blocks(block);
}
