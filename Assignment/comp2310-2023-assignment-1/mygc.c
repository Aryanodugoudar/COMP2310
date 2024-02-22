#include "mymalloc.c"
#include "mymalloc.h"
#include "mygc.h"
#include <stddef.h>
#include <stdbool.h>

static void *start_of_stack = NULL;

// Structure to represent allocated blocks for GC tracking
typedef struct GCBlock
{
  void *ptr;            // Pointer to the allocated memory
  struct GCBlock *next; // Next block in the linked list
  bool marked;          // Flag to mark if the block is reachable
} GCBlock;

// Linked list to track allocated blocks for GC
static GCBlock *gc_blocks = NULL;

void set_start_of_stack(void *start_addr)
{
  start_of_stack = start_addr;
}

void *my_malloc_gc(size_t size)
{
  // Allocate memory using my_malloc
  void *ptr = my_malloc(size);

  // Add the allocated block to the list of tracked blocks for GC
  GCBlock *new_block = (GCBlock *)my_malloc(sizeof(GCBlock));
  if (new_block)
  {
    new_block->ptr = ptr;
    new_block->marked = false;
    new_block->next = gc_blocks;
    gc_blocks = new_block;
  }

  return ptr;
}

void my_free_gc(void *ptr)
{

  GCBlock *current = gc_blocks;
  GCBlock *prev = NULL;

  while (current)
  {
    if (current->ptr == ptr)
    {
      // Mark the block as free
      current->marked = false;

      // Free the memory using my_free
      my_free(ptr);

      // Remove the block from the list
      if (prev)
      {
        prev->next = current->next;
      }
      else
      {
        gc_blocks = current->next;
      }

      // Free the GCBlock structure
      my_free(current);

      break;
    }

    prev = current;
    current = current->next;
  }
}

void *get_end_of_stack()
{
  return __builtin_frame_address(1);
}

// Helper function to mark reachable memory
static void mark_memory(void *ptr)
{
  GCBlock *block = gc_blocks;
  while (block)
  {
    if (block->ptr == ptr && !block->marked)
    {
      block->marked = true;
      Header *header = (Header *)((char *)block->ptr - kMateDataSize);
      size_t block_size = get_size(header);
      char *start = (char *)block->ptr;
      char *end = start + block_size;

      // Iterate through the block and mark any pointers within it
      for (char *p = start; p < end; p += sizeof(void *))
      {
        void *potential_ptr = *((void **)p);
        if (potential_ptr && potential_ptr >= start && potential_ptr < end)
        {
          mark_memory(potential_ptr);
        }
      }
      break;
    }
    block = block->next;
  }
}

void my_gc()
{
  void *end_of_stack = get_end_of_stack();

  // Mark reachable memory starting from the end of the stack
  mark_memory(end_of_stack);

  // Free unmarked memory blocks
  GCBlock *block = gc_blocks;
  GCBlock *prev_block = NULL;
  while (block)
  {
    if (!block->marked)
    {
      // This block is not reachable, free it
      my_free(block->ptr);
      if (prev_block)
      {
        prev_block->next = block->next;
      }
      else
      {
        gc_blocks = block->next;
      }
      GCBlock *temp = block;
      block = block->next;
      my_free(temp);
    }
    else
    {
      // Reset the mark flag for the next cycle
      block->marked = false;
      prev_block = block;
      block = block->next;
    }
  }
}
