#ifndef IVM_STRING_H
#define IVM_STRING_H

#include <stdint.h>
#include <stdbool.h>

/* IvmString — replaces raw C strings for anything that touches untrusted or
 * runtime-produced data (file reads, stdin, concat/slice results).
 *
 * Design:
 *  - Length-prefixed. NEVER NUL-terminated by contract, so nothing downstream
 *    can "forget" the terminator or run off the end scanning for one — every
 *    op carries and checks an explicit uint32_t length.
 *  - Immutable: concat/slice always allocate a NEW handle. There is no
 *    in-place mutation, which matches the bump arena underneath (no
 *    per-block free/grow anyway) and means two IvmStrings can never alias
 *    and corrupt each other.
 *  - Lives inside the sandbox's IvsArena via the same handle table as
 *    everything else (ivm_handle_alloc/deref) — bytecode holds a handle,
 *    never a raw pointer. Tagged on the VM stack as G_PUSH_IVMSTRING,
 *    distinct from G_PUSH_HANDLE (generic byte buffers from
 *    G_MEMORY_ALLOCATE / G_PUSH_ARRAY), so consumers (SYS_STDOUT_WRITE,
 *    G_SHOW, etc.) know which layout they're looking at.
 */

typedef struct {
    uint32_t len; /* valid byte count - bytes follow immediately after this header. */
} IVMStringHeader;

static inline const char *obj_string_bytes(const IVMStringHeader *h)
{
    return (const char*)(h + 1);
}

/* The API (ivm_string_new/get/concat/slice/find) is
 * declared in ivm.h, which includes this header AFTER IVMContext is defined,
 * and implemented in ivm_string.c. */

#endif /* IVM_STRING_H */