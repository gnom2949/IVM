/* ivm-api.c, The IVM API, IVM™.
* Copyright(C) 2026 Aleksandr Silaev.
* SPDX-License-Identifier: LGPL-3-or-later
* Licensed under GNU Lesser General Public License v3.
* IVM™ IS A FREE SOFTWARE.
*/
#include "../include/ivm.h" 

IVMContext *g_api_init()
{
    IVMContext *ctx = obj_create ();
    ctx->pc = 0;
    memset (ctx->bytecode, 0, sizeof (ctx->bytecode));
    obj_kick (ctx);
    return ctx;
}

void g_api_save (IVMContext *ctx, const char *filename)
{
    obj_savefile (ctx, filename);
}

void g_api_load (IVMContext *ctx, const char *filename)
{
    obj_loadfile (ctx, filename);
}

void g_push_int (Intcon_t *ctx, int value)
{
    emit_byte (ctx, G_PUSH_INT);
    emit_integer (ctx, value);
}

void g_push_double (Intcon_t *ctx, double value)
{
    emit_byte (ctx, G_PUSH_DOUBLE);
    emit_double (ctx, value);
}

void g_push_float (Intcon_t *ctx, float value)
{
    emit_byte (ctx, G_PUSH_FLOAT);
    emit_float (ctx, value);
}

void g_push_string (Intcon_t *ctx, const char *string)
{
    emit_byte (ctx, G_PUSH_STRING);
    emit_string (ctx, string);
}

void g_push_boolean (Intcon_t *ctx, bool value)
{
    emit_byte (ctx, G_PUSH_BOOLEAN);
    emit_integer (ctx, value);
}

int g_get_label (Intcon_t *ctx)
{
    return ctx->ip;
}

void g_push_uint8 (Intcon_t *ctx, uint8_t value)
{
    emit_byte (ctx, G_PUSH_UINT);
    emit_byte (ctx, value);
}

void g_push_char (Intcon_t *ctx, char character)
{
    emit_byte (ctx, G_PUSH_CHAR);
    emit_byte (ctx, (uint8_t)character);
}

void g_debug_patch_addr (Intcon_t *ctx, int addresToPatch, int targetValue)
{
    memcpy (&ctx->bytecode[addresToPatch], &targetValue, sizeof (int));
}

void g_malloc (Intcon_t *ctx, size_t size)
{
    emit_byte (ctx, G_PUSH_INT);
    emit_integer (ctx, (int)size);
    emit_byte (ctx, G_MEMORY_ALLOCATE);
}

void g_free (Intcon_t *ctx)
{
    emit_byte (ctx, G_MEMORY_FREE);
}

void g_mem_get_stats (Intcon_t *ctx)
{
    emit_byte (ctx, G_PUSH_INT);
    emit_integer (ctx, 1);
    emit_byte (ctx, G_MEMORY_TOOL);
}

void g_mem_map (Intcon_t *ctx)
{
    emit_byte (ctx, G_PUSH_INT);
    emit_integer (ctx, 2);
    emit_byte (ctx, G_MEMORY_TOOL);
}

void g_halt (Intcon_t *ctx)
{
    emit_byte (ctx, G_HALT);
}

void g_api_corrupt (Intcon_t *ctx)
{
    obj_corrupt (ctx);
}