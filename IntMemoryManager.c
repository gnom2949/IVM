/* IntMemoryManager.c
* Copyright © 2026 Aleksandr Silaev
* This file is part of the Int Memory Manager library.
* The Int Memory Manager library is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*/

#include "IntMemoryManager.h"
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/mman.h>
#define AR_SIZE (1024 * 1024 * 100) // 100MB of memory, maybe i shound have smaller than that size, but is testing and this version of cameradarC needed a unstable branch

static IntMemoryRange *base = NULL; // base of the linked list of memory blocks
static pthread_mutex_t imm_lock = PTHREAD_MUTEX_INITIALIZER;  

static int imm_init()
{
  if (base != NULL) return 0;

  base  = mmap (NULL, AR_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (base == MAP_FAILED) return -1;

  base->size = AR_SIZE - BLOCK_SIZE;
  base->free = 1;
  base->hex = IMM_HEX;
  base->next = NULL;
  base->prev = NULL;
  base->ptr = (void *)(base + 1);

  return 0;
}

IntMemoryRange *find_free_block_of_memory (IntMemoryRange **last, size_t size)
{
    IntMemoryRange *cur = base;

    while (cur && !(cur->free && cur->size >= size)) {
        if (last) *last = cur;
        cur = cur->next;
    }
    return cur;
}

static void *_internal_MemoryAllocate (size_t size)
{
    if (size <= 0) return NULL;
    if (imm_init() != 0) return NULL;

    size = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);

    IntMemoryRange *last = NULL;
    IntMemoryRange *block = find_free_block_of_memory (&last, size);

    if (block) {
        if (block->size >= size + BLOCK_SIZE + ALIGNMENT) {
          IntMemoryRange *nblock = (IntMemoryRange *)((char *)block->ptr + size);

          nblock->hex = IMM_HEX;
          nblock->size = block->size - size - BLOCK_SIZE;
          nblock->free = 1;
          nblock->next = block->next;
          nblock->prev = block;
          nblock->ptr = (void*)(nblock + 1);

          if (block->next) block->next->prev = nblock;
          block->next = nblock;
          block->size = size;
        }
        block->free = 0;
        return block->ptr;
    }
  return NULL;
}

void *MemoryAllocate (size_t size) {
    pthread_mutex_lock(&imm_lock);
    void *ptr = _internal_MemoryAllocate(size);
    pthread_mutex_unlock(&imm_lock);
    return ptr;
}

static void _internal_coalesce (IntMemoryRange *block)
{
  if (!block || !block->free || block->hex != IMM_HEX) return;

  if (block->next) {
      if (block->next->hex == IMM_HEX && block->next->free) {
        IntMemoryRange *nblock = block->next;
        block->size += BLOCK_SIZE + nblock->size;
        block->next = nblock->next;
        if (block->next) block->next->prev = block;
      }
  }

  if (block->prev) {
    if (block->prev->hex == IMM_HEX && block->prev->free) {
      IntMemoryRange *pblock = block->prev;
      pblock->size += BLOCK_SIZE + block->size;
      pblock->next = block->next;
      if (block->next) block->next->prev = pblock;
    }
  }
}

void _internal_cleanbit (void *ptr)
{
    if (!ptr) return;

    IntMemoryRange *block = (IntMemoryRange *)ptr - 1;

    if (block->hex != IMM_HEX) {
      return;
    }

    if ((uintptr_t)ptr < (uintptr_t)base + BLOCK_SIZE) return;

    if (block->free) return;

    block->free = 1;

    _internal_coalesce (block);
}

void cleanbit (void *ptr)
{
    if (!ptr) return;
    pthread_mutex_lock(&imm_lock);
    _internal_cleanbit(ptr);
    pthread_mutex_unlock(&imm_lock);
}

void *MemoryReAllocate (void   *ptr, size_t  size)
{
  if (!ptr) return MemoryAllocate (size);

  pthread_mutex_lock (&imm_lock);
  IntMemoryRange *oblock = (IntMemoryRange *)ptr - 1;
  if (oblock->hex != IMM_HEX) {
    pthread_mutex_unlock (&imm_lock);
    return NULL;
  }

  if (oblock->size >= size) {
    pthread_mutex_unlock (&imm_lock);
    return ptr;
  }

  void *fresh_ptr = _internal_MemoryAllocate (size);
  if (fresh_ptr) {
    memcpy (fresh_ptr, ptr, oblock->size);
    _internal_cleanbit (ptr);
  }

  pthread_mutex_unlock (&imm_lock);
  return fresh_ptr;
}

void MemoryPullStats (IntMemoryStatistics *out_stats)
{
  pthread_mutex_lock (&imm_lock);
  out_stats->total_mapped = AR_SIZE;
  out_stats->currently_used = 0;
  out_stats->active_blocks = 0;

  IntMemoryRange *cur = base;
  while (cur)
  {
    if (!cur->free)
    {
      out_stats->currently_used += cur->size;
      out_stats->active_blocks++;
      out_stats->total_allocations++;
    }
    cur = cur->next;
  }
  pthread_mutex_unlock (&imm_lock);
}

void MemoryRenderMap (const char *name)
{
  pthread_mutex_lock (&imm_lock);
  const int WIDTH = 50;
  printf ("\n        \033[1m====== [ %s Memory Map ] ======\033[0m \n", name);
  printf ("[");

  IntMemoryRange *cur = base;
  int chars_printed = 0;

  while (cur && chars_printed < WIDTH)
  {
    // Calculate how many "squares" we have
    float ratio = (float)(cur->size + BLOCK_SIZE) / AR_SIZE; // ratio failed
    int chars = (int)(ratio * WIDTH);
    if (chars <= 0 && cur->size > 0) chars = 1;

    
    if (!cur->free)
    {
      printf ("\033[33m#\033[0m");
    } else {
      printf ("\033[36m~");
    }

    for (int i = 1; i < chars && (chars_printed + i) < WIDTH; i++)
    {
      printf ("%c", !cur->free ? '#' : '~');
    }

    chars_printed += chars;
    cur = cur->next;
  }
  
  while (chars_printed < WIDTH)
  {
    printf ("\033[36m~\033[0m");
    chars_printed++;
  }

  printf ("]");
  printf ("\n\033[0m ================================================== \n");
  pthread_mutex_unlock (&imm_lock);
}