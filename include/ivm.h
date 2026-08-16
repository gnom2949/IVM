#ifndef IVM_H
#define IVM_H
#if defined(__cplusplus)
    extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <string.h>
#include "../src/ivm-in.h" 

//extern int ip; // Instruction pointer.
//extern int sp; // Stack pointer. 
//extern int pc; // Program counter.
//extern uint8_t bytecode[1024];

/*======================= OBJ's ==========================*/
void        obj_emit_byte (IVMContext *ctx, uint8_t byte);
int         obj_verify_header (FILE *fd);
int         obj_load_file (IVMContext *ctx, const char *filename);
void        obj_emit_float (IVMContext *ctx, float f);
void        obj_save_file (IVMContext *ctx, const char *filename);
void        obj_emit_integer (IVMContext *ctx, int i);
void        obj_emit_string (IVMContext *ctx, const char *str);
void        obj_emit_double (IVMContext *ctx, double d);
void        obj_execute_runtime (IVMContext *ctx);
IVMContext *obj_create (void);
void        obj_corrupt (IVMContext *cxt);
void        obj_kick (IVMContext *ctx);
int         obj_handle_alloc(IVMContext *ctx, size_t size);
void        obj_handle_free(IVMContext *ctx, int handle);
void       *obj_handle_deref(IVMContext *ctx, int handle);
int         obj_string_new(IVMContext *ctx, const char *bytes, uint32_t len);
int         obj_string_reserve (IVMContext *ctx, uint32_t len, char **out_bytes);
bool        obj_string_get(IVMContext *ctx, int handle, const char **out_bytes, uint32_t *out_len);
int         obj_string_concat(IVMContext *ctx, int h1, int h2);
int         obj_string_slice(IVMContext *ctx, int h, uint32_t start, uint32_t end);
bool        obj_string_find(IVMContext *ctx, int h, int needle_h, uint32_t *out_pos);
/* ================================= API ============================================= */
// Manage the runtime //
IVMContext *ivmApiCreate();
void        ivmApiUnload (IVMContext *ctx, const char *filename);
void        ivmApiLoad (IVMContext *ctx, const char *filename);
void        ivmApiCorrupt (IVMContext *ctx);
void        ivmApiPushInt8 (IVMContext *ctx, int8_t value);
void        ivmApiPushInt16 (IVMContext *ctx, int16_t value);
void        ivmApiPushInt32 (IVMContext *ctx, int32_t value);
void        ivmApiPushFloat (IVMContext *ctx, float value);
void        ivmApiPushDouble (IVMContext *ctx, double value);
void        ivmApiPushString (IVMContext *ctx, const char *string);
int         ivmApiGetLabel (IVMContext *ctx);
void        ivmApiHalt(IVMContext *ctx);
void        ivmApiPushBoolean (IVMContext *ctx, bool value);
void        ivmApiPushUnsignedInt8 (IVMContext *ctx, uint8_t value);
void        ivmApiPushUnsignedInt16 (IVMContext *ctx, uint16_t value);
void        ivmApiPushUnsignedInt32 (IVMContext *ctx, uint32_t value);
void        ivmApiPushChar (IVMContext *ctx, char character);
void        ivmApiAdd();
void        ivmApiShow();

// Memory management
void ivmApiMemoryAlloc (IVMContext *ctx, size_t size);
void ivmApiMemoryFree (IVMContext *ctx);

// Debug functions
#if defined(IVM_TOGGLE_DEBUG_FUNCTIONS)
void ivmApiDebugPatchAddress (IVMContext *ctx, int addresToPatch, int targetValue);
void ivmApiGetMemoryStats (IVMContext *ctx);
void ivmApiMemoryMap(IVMContext *ctx);
#endif
// loop's
IVMLoopWhile ivmWhileStart (IVMContext *ctx);
IVMLoopWhile ivmWhileEnd (IVMContext *ctx);
void         ivmApiWhileCheckCond (IVMContext *ctx, IVMLoopWhile *w);

#endif /* IVM_H */

#if defined(__cplusplus)
    }
#endif /* __cplusplus */