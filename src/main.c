/* main.c, The main of IVM™.
* Copyright(C) 2026 Aleksandr Silaev.
* SPDX-License-Identifier: LGPL-3-or-later
* Licensed under GNU Lesser General Public License v3.
* IVM™ IS A FREE SOFTWARE.
*/

#include "../include/ivm.h"
#include "ivm-in.h"
#include "../include/arena.h"
#include "../external/src/IntMemoryManager.h"
#define PC ctx->pc
#define BYTECODE ctx->bytecode
#define SP ctx->sp
#define IP ctx->ip
#define STACK ctx->stack
#define RUN ctx->running

int obj_handle_alloc(IVMContext *ctx, size_t size)
{
	if (ctx->slot_count >= IVM_SANDBOX_MAX_ALLOCS) return -1;

	void *ptr = xalloc(&ctx->mem, size);
	if (!ptr) return -1;

	int idx = ctx->slot_count++;
	ctx->slots[idx].ptr = ptr;
	ctx->slots[idx].size = size;
	ctx->slots[idx].alive = true;
	return idx;
}

void obj_handle_free(IVMContext *ctx, int handle)
{
	if (handle < 0 || handle >= ctx->slot_count) return;
	ctx->slots[handle].alive = false;
}

void *obj_handle_deref(IVMContext *ctx, int handle)
{
	if (handle < 0 || handle >= ctx->slot_count) return NULL;
	if (!ctx->slots[handle].alive) return NULL;
	return ctx->slots[handle].ptr;
}

static void obj_emit_bytes(IVMContext *ctx, const void *src, size_t n)
{
	if ((size_t)PC + n > sizeof(BYTECODE))
	{
		fprintf(stderr, "\033[34m[IVM]\033[0m \033[31mFatal\033[0m: Bytecode buffer overflow(Stats: PC=%d, +%zu > %zu). Emit aborted, IVM going down.\n",
		PC, n, sizeof(BYTECODE));
		RUN = false;
		return;
	}
	memcpy(&BYTECODE[PC], src, n);
	PC += (int)n;
}

void obj_emit_byte (IVMContext *ctx, uint8_t byte) // Writing into byte-code.
{
	obj_emit_bytes(ctx, &byte, sizeof(byte));
}

IVMContext *obj_create (void)
{
	IVMContext *ctx = MemoryAllocate (sizeof(IVMContext));
	if (!ctx) return NULL;

	IP = 0;
	SP = -1;
	PC = 0;
	RUN = false;
	memset (STACK, 0, sizeof (STACK));
	memset (BYTECODE, 0, sizeof (BYTECODE));

	if (!xainit(&ctx->mem, IVM_SANDBOX_ARENA_LIMIT))
	{
		fprintf(stderr, "\033[36m[IVM]\033[0m \033[31mError\033[0m: Failed to reserve memory for sandbox. IVM can't run without Sandbox, shutting down.\n");
		cleanbit(ctx);
		return NULL;
	}
	ctx->slot_count = 0;
	memset(ctx->slots, 0, sizeof(ctx->slots));

	return ctx;
}

void obj_corrupt (IVMContext *ctx)
{
	if (ctx)
	{
		xdestroy(&ctx->mem);
		cleanbit(ctx);
	}
}

void obj_kick (IVMContext *ctx)
{
	if (!ctx) return;

	IP = 0;
	SP = -1;
	RUN = true;

	memset (STACK, 0, sizeof (STACK));

	printf ("\033[34m[IVM]\033[0m \033[32mInfo\033[0m: Reset complete. Ready to execute.\n");
}

int obj_verify_header (FILE *fd)
{
	IVMHeader header;

	if (fread (&header, sizeof (IVMHeader), 1, fd) != 1) return -1;

	if (header.magic != IVM_MNUM) {
		fprintf (stderr, "\033[36m[IVM]\033[0m \033[31mError\033[0m: Not found magic number, this file is not for IVM.\n");
		return -1;
	}

	if (header.version > 1) {
		printf ("\033[36m[IVM]\033[0m \033[31mError\033[0m: VM is too old for this file\n");
		return -1;
	}

	printf ("\033[36m[IVM]\033[0m \033[32mInfo\033[0m: Loading %u bytes of ivm...\n", header.code_size);
	return header.code_size;
}

int obj_load_file (IVMContext *ctx, const char *filename) // It is necessary to specify the file name WITHOUT .ivm
{
	char ff[128]; // ff -> fully filename
	snprintf (ff, sizeof (ff), "%s.ivm", filename);

	FILE *fd = fopen (ff, "r");
	if (!fd) {
		fprintf (stderr, "\033[31m[ALERT]\033[0m Cannot open file!\n");
		return 1;
	}

	uint32_t check_magic;
	if (fread (&check_magic, sizeof (uint32_t), 1, fd) != 1) {
		fprintf(stderr, "\033[31m[ERROR]\033[0m File is empty or too small!\n");
        fclose(fd);
        return 1;
	}

	if (check_magic != IVM_MNUM) {
		fprintf(stderr, "\033[31m[ERROR]\033[0m That's not mine file, don't try load invalid file moron, only .ivm .\n");z
        fclose(fd);
        return 1;
	}

	fseek (fd, 0, SEEK_END);
	long fSize = ftell (fd) - sizeof (uint32_t);
	fseek (fd, sizeof (uint32_t), SEEK_SET);

	if (fSize > (long) sizeof (BYTECODE)) {
		fprintf (stderr, "\033[31m[ALERT]\033[0m File too large!\n");
		fclose (fd);
		return 1;
	}

	fread (BYTECODE, 1, fSize, fd);
	PC = (int)fSize;

	fclose (fd);
	return 0;
}

void obj_save_file (IVMContext *ctx, const char *filename)
{
	char ff[128];
	snprintf (ff, sizeof (ff), "%s.ivm", filename);

	FILE *fd = fopen (ff, "wb");
	if (!fd) 
	{
		perror ("Failed to create file.\n");
		return;
	}	

	uint32_t magic = IVM_MNUM;
	fwrite (&magic, sizeof (uint32_t), 1, fd);

	fwrite (BYTECODE, 1, PC, fd);

	fclose (fd);
	printf ("\033[32m[INFO]\033[0m Created file with name %s.\n", ff);
}

void obj_emit_float (IVMContext *ctx, float f) // Writing Float type.
{
	obj_emit_bytes(ctx, &f, sizeof(f));
}

void obj_emit_double (IVMContext *ctx, double d)
{
	obj_emit_bytes(ctx, &d, sizeof(d));
}

void obj_emit_integer (IVMContext *ctx, int i) // Writing Integer type.
{
	obj_emit_bytes(ctx, &i, sizeof(i));
}

void obj_emit_string (IVMContext *ctx, const char* s) // Writing String type.
{
	obj_emit_bytes(ctx, &s, sizeof(s) + 1); // +1 for null terminator.
}

static bool obj_check_operand(IVMContext *ctx, size_t n)
{
	if ((size_t)IP + n > sizeof(BYTECODE))
	{
		fprintf(stderr, "\033[34m[IVM]\033[0m \033[41mOPFAULT\033[0m: Truncated/malformed operand at IP=%d (need %zu bytes) - shutting down.\n", IP, n);
		RUN = false;
		return false;
	}
	return true;
}

void obj_execute_runtime (IVMContext *ctx)
{
	while (RUN) 
	{
		if (IP < 0 || (size_t)IP >= sizeof(BYTECODE))
		{
			fprintf(stderr, "\033[34m[IVM]\033[0m \033[41mINSTFAULT\033[0m: Instruction pointer out of bounds (IP=%d), shutting down.\n", IP);
			RUN = false;
			break;
		}
		uint8_t inst = BYTECODE[IP++]; // Reading the instruction (G_ prefix).

		switch (inst) 
		{
			case G_HALT:
				RUN = false;
				break;

			case G_PUSH_INT: 
			{
				if (!obj_check_operand(ctx, sizeof(int))) break;
				int val;

				memcpy (&val, &BYTECODE[IP], sizeof (int));
				IP += sizeof (int);

				PUSH (ctx, G_PUSH_INT, i_val, val);

				printf ("Pushed INTEGER: %d\n", val);
				break;
			}

			case G_PUSH_UINT: 
			{
				if (!obj_check_operand(ctx, sizeof(uint32_t))) break;
				uint32_t val;
				memcpy (&val, &BYTECODE[IP], sizeof (uint32_t));
				IP += sizeof (uint32_t);

				PUSH (ctx, G_PUSH_UINT, u_val, val);

				printf ("Pushed Unsigned INT: %u\n", val);
				break;
			}

			case G_PUSH_FLOAT: 
			{
				if (!obj_check_operand(ctx, sizeof(float))) break;
				float val;
				memcpy (&val, &BYTECODE[IP], sizeof (float));
				IP += sizeof (float);

				PUSH (ctx, G_PUSH_FLOAT, f_val, val);
				
				printf ("Pushed FLOAT: %f\n", val);
				break;
			}

			case G_PUSH_STRING: 
			{
				size_t maxlen = sizeof(BYTECODE) - (size_t)IP;
				char *sPtr = (char*) &BYTECODE[IP];
				size_t len = strnlen(sPtr, maxlen);
				if (len == maxlen)
				{
					fprintf(stderr, "\033[34m[IVM]\033[0m \033[41mSTRFAULT\033[0m: Unterminated string operand at IP=%d, shutting down.\n", IP);
					RUN = false;
					break;
				}
				IP += (int)(len + 1);
				PUSH (ctx, G_PUSH_STRING, sPtr, sPtr);
				printf("Pushed STRING: %s\n", sPtr);
				break;
			}

			case G_PUSH_BOOLEAN: 
			{
				if (!obj_check_operand(ctx, sizeof(bool))) break;
				bool val = (BYTECODE[IP++] != 0); // any value that non-equals 0 => true
				PUSH (ctx, G_PUSH_BOOLEAN, b_val, val);
				printf ("Pushed BOOLEAN with %s\n", val ? "true" : "false");
				break;
			}

			case G_PUSH_DOUBLE: 
			{
				if (!obj_check_operand(ctx, sizeof(double))) break;
				double val;
				memcpy (&val, &BYTECODE[IP], sizeof (double));
				IP += sizeof (double);
				PUSH (ctx, G_PUSH_DOUBLE, d_val, val);
				printf ("Pushed DOUBLE with value: %lf\n", val);
				break;
			}

			case G_PUSH_CHAR: 
			{
				if (!obj_check_operand(ctx, 1)) break;
				char val = (char)BYTECODE[IP++];
				PUSH (ctx, G_PUSH_CHAR, c_val, val);
				printf ("Pushed CHAR with value: %c\n", val);
				break;
			}

			case G_PUSH_LONG: 
			{
				if (!obj_check_operand(ctx, sizeof(long))) break;
				long val;
				memcpy (&val, &BYTECODE[IP], sizeof (long));
				IP += sizeof (long);
				PUSH (ctx, G_PUSH_LONG, l_val, val);
				printf ("Pushed LONG with value: %ld\n", val);
				break;
			}

            case G_MEMORY_ALLOCATE:
			{
				if (SP < 0) 
				{
					fprintf(stderr, "\033[34m[IVM]\033[0m \033[31mError\033[0m: Stack Underflow in MEMORY_ALLOCATE!\n");
					RUN = false;
					break;
				}
				IVMCell val_size = STACK[SP--];
				if (val_size.op != G_PUSH_INT && val_size.op != G_PUSH_UINT)
				{
					fprintf(stderr, "\033[34m[IVM]\033[0m \033[31mError\033[0m: MEMORY_ALLOCATE expects an int or uint size on stack.\n");
					RUN = false;
					break;
				}
				size_t size = (val_size.op == G_PUSH_INT)
								? (size_t) val_size.data.i_val
								: (size_t) val_size.data.u_val;

				int handle = obj_handle_alloc(ctx, size);

				if (handle < 0)
				{
					printf("\033[34m[IVM]\033[0m \033[31mError\033[0m Sandbox memory budget exhausted(or handle table is full)!\n");
					PUSH(ctx, G_PUSH_BOOLEAN, b_val, false);
				}
				else
				{
					printf("\033[34m[IVM]\033[0m \033[32mInfo\033[0m: Allocated handle #%d (%zu bytes)\n", handle, size);
				}
				break;
			}

			case G_SYS_STDIN_READ: 
			{
				if (SP < 0) {
					fprintf(stderr, "\033[34m[IVM]\033[0m \033[31mError\033[0m: Stack Underflow in SYS_STDIN_READ!\n");
					RUN = false;
					break;
				}
				// get buf size from stack
				IVMCell size_val = STACK[SP--];
				if (size_val.op != G_PUSH_INT || size_val.data.i_val <= 0)
				{
					PUSH(ctx, G_PUSH_BOOLEAN, b_val, false);
					break;
				}
				size_t limit = (size_t)size_val.data.i_val;

				int scratch = obj_handle_alloc(ctx, limit + 1);
				if (scratch < 0)
				{
					printf("\033[34m[IVM]\033[0m \033[31mError\033[0m: Sandbox memory budget exhausted!\n");
					PUSH (ctx, G_PUSH_BOOLEAN, b_val, false);
					break;
				}
				char *raw = (char*) obj_handle_deref(ctx, scratch);

				if (fgets (raw, (int)limit, stdin) != NULL)
				{
					size_t len = strcspn(raw, "\n");

					int handle = obj_string_new(ctx, raw, (uint32_t)len);
					obj_handle_free(ctx, scratch);
					if (handle < 0)
					{
						printf("\033[34m[IVM]\033[0m \033[31mError\033[0m: Sandbox memory budget exhausted!\n");
						PUSH(ctx, G_PUSH_BOOLEAN, b_val, false);
					}
					PUSH (ctx, G_PUSH_IVMSTRING, i_val, handle);
				}
				else
				{
					obj_handle_free(ctx, scratch);
					PUSH (ctx, G_PUSH_BOOLEAN, b_val, false);
				}
				break;
			}

			case G_SYS_STDOUT_WRITE:
			case G_SYS_STDERR_WRITE: 
			{
				FILE *tar = (inst == G_SYS_STDOUT_WRITE) ? stdout : stderr;
				if (SP < 0) break;

				IVMCell val = STACK[SP--];

				switch (val.op)
				{
					case G_PUSH_STRING:
						fprintf (tar, "%s", val.data.sPtr);
						break;
					case G_PUSH_INT:
						fprintf (tar, "%d", val.data.i_val);
						break;
					case G_PUSH_DOUBLE:
						fprintf (tar, "%lf", val.data.d_val);
						break;
					case G_PUSH_FLOAT:
						fprintf (tar, "%f", val.data.f_val);
						break;
					case G_PUSH_CHAR:
						fprintf (tar, "%c", val.data.c_val);
						break;
					case G_PUSH_BOOLEAN:
						fprintf (tar, "%s", val.data.b_val ? "true" : "false");
						break;
					case G_PUSH_LONG:
						fprintf (tar, "%ld", val.data.l_val);
						break;
					case G_PUSH_UINT:
						fprintf (tar, "%u", val.data.u_val);
						break;
				}
				fflush (tar);
				break;
			}

			case G_MEMORY_FREE: 
			{
				IVMCell ptr_val = STACK[SP--];
				if (ptr_val.op == G_PUSH_ARRAY)
				{
					cleanbit (ptr_val.data.aPtr);
					printf ("[IVM] Memory freed.\n");
				}
				break;
			}

			case G_MEMORY_TOOL: 
			{
				int toolid = STACK[SP--].data.i_val;

				switch (toolid)
				{
					case 1:
						IntMemoryStatistics stats;
						MemoryPullStats (&stats);
						printf ("\n===== [ IVM MEMORY STATISTICS ] =====\n");
						printf ("	Mapped:		%zu KB\n", stats.total_mapped / 1024);
						printf ("	Used:		%zu KB (%.2f%%)\n", stats.currently_used / 1024, (float)stats.currently_used / stats.total_mapped * 100);
						printf ("	Blocks: 	%d active\n", stats.active_blocks);
						printf ("\n======================================\n\n");
					break;

				}
				break;
			}

			case G_SYS_FILE_READ: 
			{
				char *path = STACK[SP--].data.sPtr;

				FILE *f = fopen (path, "rb");
				if (!f)
				{
					PUSH (ctx, G_PUSH_BOOLEAN, b_val, false);
					break;
				}

				fseek (f, 0, SEEK_END);
				size_t fsize = ftell (f);
				rewind (f);

				void *buffer = MemoryAllocate (fsize + 1);
				fread (buffer, 1, fsize, f);
				((char*)buffer)[fsize] = '\0';
				fclose (f);

				PUSH (ctx, G_PUSH_ARRAY, aPtr, buffer);
				printf ("\033[36m[FS]\033[0m File %s loaded.", path);
				break;
			}

			case G_PUSH_ARRAY: 
			{
				int size;
				memcpy (&size, &BYTECODE[IP], sizeof (int));
				IP += sizeof (int);

				int *arr = (int*) MemoryAllocate (size * sizeof (int));
				if (arr)
				{
					memset (arr, 0, size * sizeof (int));
					PUSH (ctx, G_PUSH_ARRAY, aPtr, (void*)arr);
					printf ("Pushed ARRAY of size %d\n", size);
				} 
				else 
				{
					fprintf (stderr, "\033[31m[ERROR]\033[0m");
					RUN = false;
				}
				break;
			}

			case G_POP: 
			{
				if (SP < 0) 
				{
					printf ("\033[31m[ERROR]\033[0m Stack Underflow!\n");
					RUN = false;
					break;
				}
				SP--;
				printf ("Popped value.\n");
				break;
			}

			case G_OP_PLUS: 
			{
				if (SP < 1) 
				{
					printf ("\033[31m[ERROR]\033[0m Stack Underflow in PLUS OPERATOR\n");
					RUN = false;
					break;
				}
				IVMCell b = STACK[SP--];
				IVMCell a = STACK[SP--];

				float val_a = (a.op == G_PUSH_INT) ? (float)a.data.i_val : a.data.f_val;
				float val_b = (b.op == G_PUSH_INT) ? (float)b.data.i_val : b.data.f_val;
				float res = val_a + val_b;

				PUSH (ctx, G_PUSH_FLOAT, f_val, res);

				printf ("Added: %f + %f EQUALS %f\n", val_a, val_b, STACK[SP].data.f_val);
				break;
			}

			case G_OP_MINUS: 
			{
				if (SP < 1) {
					printf ("\033[31m[ERROR]\033[0m Stack Underflow in MINUS OPERATOR");
					RUN = false;
					break;
				}
				IVMCell b = STACK[SP--];
				IVMCell a = STACK[SP--];

				float val_a = (a.op == G_PUSH_INT) ? (float)a.data.i_val : a.data.f_val;
				float val_b = (b.op == G_PUSH_INT) ? (float)b.data.i_val : b.data.f_val;
				float res = val_a - val_b;

				PUSH (ctx, G_PUSH_FLOAT, f_val, res);

				printf ("Excluded: %f - %f EQUALS %f\n", val_a, val_b, STACK[SP].data.f_val);
				break;
			}

			case G_JUMP: {
				int tAddr; // target address
				memcpy (&tAddr, &BYTECODE[IP], sizeof (int));

				IP = tAddr;

				printf ("Jumping to byte %d", tAddr);
				break;
			}

			case G_JUMP_IF_FALSE: {
				int tAddr;
				memcpy (&tAddr, &BYTECODE[IP], sizeof (int));
				IP += sizeof (int);

				IVMCell val = STACK[SP--];
				bool cond = false;

				if (val.op == G_PUSH_BOOLEAN) cond = val.data.b_val;
				else if (val.op == G_PUSH_INT) cond = (val.data.i_val != 0);

				if (!cond) {
					IP = tAddr;
					printf ("Condition false. Jumping to %d\n", tAddr);
				} else {
					printf ("Condition true. Continue...\n");
				}
				break;
			}

			case G_SHOW: {
				if (SP < 0) break;
				IVMCell val = STACK[SP];

				printf ("Top of stack: ");
				if (val.op == G_PUSH_INT) printf ("%d\n", val.data.i_val);
				else if (val.op == G_PUSH_FLOAT) printf ("%f\n", val.data.f_val);
				else if (val.op == G_PUSH_STRING) printf ("%s\n", val.data.sPtr);
				break;
			}
		}
	}
}

#ifndef NOMAIN
int main (int argc, char **argv) 
{
	IVMContext *ctx = obj_create ();
	if (!ctx) {
		fprintf (stderr, "\033[31m[ERROR]\033[0m Failed to create VM context!\n");
		obj_corrupt (ctx);
		return 1;
	}

	obj_emit_byte (ctx, G_PUSH_INT);
	obj_emit_integer (ctx, 2);
	obj_emit_byte (ctx, G_MEMORY_TOOL);
	obj_emit_byte (ctx, G_HALT);
	obj_execute_runtime (ctx);

	obj_corrupt (ctx);
	return 0;
}
#endif /* NOMAIN */