// The API of IVM, uses IMM, to compile use make.
#include "ivm.h" 

Intcon_t *g_api_init()
{
    Intcon_t *ctx = obj_create ();
    ctx->pc = 0;
    memset (ctx->bytecode, 0, sizeof (ctx->bytecode));
    return ctx;
}

void g_api_save (Intcon_t *ctx, const char *filename)
{
    obj_savefile (ctx, filename);
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