/* ivm-loop.c, The loop's realization, IVM™.
* Copyright(C) 2026 Aleksandr Silaev.
* SPDX-License-Identifier: LGPL-3-or-later
* Licensed under GNU Lesser General Public License v3.
* IVM™ IS A FREE SOFTWARE.
*/

#include "ivm.h"
#include "IntMemoryManager.h"

LoopWhile g_while_start (Intcon_t *ctx)
{
    LoopWhile w;
    w.startAddr = g_get_label(ctx);



    return w;
}

void g_while_cond_check (Intcon_t *ctx, LoopWhile *w)
{
    emit_byte (ctx, G_JUMP_IF_FALSE);

    w->endAddr = g_get_label (ctx);
    emit_integer (ctx, 0);
}

void g_while_end (Intcon_t *ctx, LoopWhile *w)
{
    emit_byte (ctx, G_JUMP);
    emit_integer (ctx, w->startAddr);

    int exitAddr = g_get_label (ctx);
    g_debug_patch_addr (ctx, w->endAddr, exitAddr);
}