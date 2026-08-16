// ivs_arena.h — hardened bump allocator for IVM (Int Virtual Machine) instances.
// Based on tsoding/arena (https://github.com/tsoding/arena), adapted for
// untrusted-code sandboxing:
//   - fixed capacity, NEVER auto-grows (no unbounded mmap/VirtualAlloc from a script)
//   - overflow-safe size check (remaining-space check, not count+size)
//   - zero-on-alloc and zero-on-reset (no data leakage across instance reuse)
//   - no per-block free (whole-arena lifetime only — see notes at bottom)
//
// Not thread-safe by design: one arena per IVM instance, one instance per
// OS process (per your fork+supervisor model), so no lock is needed here.

#ifndef IVS_ARENA_H
#define IVS_ARENA_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <unistd.h>
#endif

typedef struct {
    uint8_t  *data;      // backing region (mmap / VirtualAlloc)
    size_t   capacity;  // fixed at init — the sandbox's memory budget
    size_t   count;     // bump offset; invariant: count <= capacity
} IvsArena;

// Reserves+commits `capacity` bytes up front. Returns false on host failure
// (caller must not touch the arena further in that case).
static inline bool xainit (IvsArena *a, size_t capacity)
{
    if (!a || capacity == 0) return false;

#if defined(_WIN32)
    void *region = VirtualAlloc (NULL, capacity, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!region) return false;
#else
    void *region = mmap (NULL, capacity, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (region == MAP_FAILED) return false;
#endif

    a->data     = (uint8_t *) region;
    a->capacity = capacity;
    a->count    = 0;
    return true;
}

// Fixed-capacity bump alloc. Returns NULL when the sandbox's budget is
// exhausted — the VM must treat that as a script-level error (e.g. push
// G_PUSH_BOOLEAN false / raise a runtime error), never fall back to the
// host allocator and never attempt to grow the region.
#define IVM_ARENA_ALIGN (sizeof(void*) * 2) 

static inline void *xalloc (IvsArena *a, size_t size)
{
    if (!a || !a->data || size == 0) return NULL;
 
    // Align the bump offset up front, overflow-safe (count/capacity are
    // small in practice, but guard the addition anyway).
    size_t aligned_count = (a->count + (IVM_ARENA_ALIGN - 1)) & ~(size_t)(IVM_ARENA_ALIGN - 1);
    if (aligned_count < a->count || aligned_count > a->capacity) return NULL; // wrapped or ran past capacity
 
    // remaining-space check — avoids overflow from count + size wrapping
    // when a malicious script requests a huge `size`.
    size_t remaining = a->capacity - aligned_count;
    if (size > remaining) return NULL;
 
    void *ptr = a->data + aligned_count;
    a->count = aligned_count + size;
 
    memset (ptr, 0, size); // zero-on-alloc
 
    return ptr;
}

// Rewinds the whole arena for reuse (e.g. between runs in a supervised,
// respawned instance). Zeroes exactly the region that was touched, so
// nothing from the previous run is readable by the next one.
static inline void xreset (IvsArena *a)
{
    if (!a || !a->data) return;
    memset (a->data, 0, a->count);
    a->count = 0;
}

// Releases the backing region entirely — call this on instance teardown,
// not as a per-allocation free (there is no per-allocation free, see below).
static inline void xdestroy (IvsArena *a)
{
    if (!a || !a->data) return;

#if defined(_WIN32)
    VirtualFree (a->data, 0, MEM_RELEASE);
#else
    munmap (a->data, a->capacity);
#endif

    a->data     = NULL;
    a->capacity = 0;
    a->count    = 0;
}

#endif /* IVS_ARENA_H */