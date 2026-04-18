// main.c of IVM, runs GBC.
// to build use make.

#include "ivm.h"
#include "IntMemoryManager.h"
#define PC ctx->pc
#define BYTECODE ctx->bytecode
#define SP ctx->sp
#define IP ctx->ip
#define STACK ctx->stack
#define RUN ctx->running

void emit_byte (Intcon_t *ctx, uint8_t byte) // Writing into byte-code.
{
	BYTECODE[PC++] = byte;
}

Intcon_t *obj_create (void)
{
	Intcon_t *ctx = MemoryAllocate (sizeof(Intcon_t));
	if (!ctx) return NULL;

	IP = 0;
	SP = -1;
	PC = 0;
	RUN = false;
	memset (STACK, 0, sizeof (STACK));
	memset (BYTECODE, 0, sizeof (BYTECODE));

	return ctx;
}

void obj_corrupt (Intcon_t *ctx)
{
	if (ctx)
	{
		cleanbit (ctx);
	}
}

void obj_kick (Intcon_t *ctx)
{
	if (!ctx) return;

	IP = 0;
	SP = -1;
	RUN = true;

	memset (STACK, 0, sizeof (STACK));

	printf ("\033[36m[IVM]\033[0m Reset complete. Ready to execute.\n");
}

int obj_verifyheader (FILE *fd)
{
	G_Header_t header;

	if (fread (&header, sizeof (G_Header_t), 1, fd) != 1) return -1;

	if (header.magic != IVM_MNUM) {
		printf ("[ERROR] G_MN_NOT_FOUND: This is a stray file.\n");
		return -1;
	}

	if (header.version > 1) {
		printf ("[ERROR] G_VERSION_MISMATCH: VM is too old for this file\n");
		return -1;
	}

	printf ("[INFO] G_MN_PRESENT. Loading %u bytes of ivm...\n", header.code_size);
	return header.code_size;
}

int obj_loadfile (Intcon_t *ctx, const char *filename) // It is necessary to specify the file name WITHOUT .ivm
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
		fprintf(stderr, "\033[31m[ERROR]\033[0m That's not mine file, don't try load invalid file moron, only .ivm .\n");
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
	PC = (int) fSize;

	fclose (fd);
	return 0;
}

void obj_savefile (Intcon_t *ctx, const char *filename)
{
	char ff[128];
	snprintf (ff, sizeof (ff), "%s.ivm", filename);

	FILE *fd = fopen (ff, "wb");
	if (!fd) {
		perror ("Failed to create file.\n");
		return;
	}	

	uint32_t magic = IVM_MNUM;
	fwrite (&magic, sizeof (uint32_t), 1, fd);

	fwrite (BYTECODE, 1, PC, fd);

	fclose (fd);
	printf ("\033[32m[INFO]\033[0m Created file with name %s.\n", ff);
}

void emit_float (Intcon_t *ctx, float f) // Writing Float type.
{
	memcpy (&BYTECODE[PC], &f, sizeof (float));
	PC += sizeof (float);
}

void emit_double (Intcon_t *ctx, double d)
{
	memcpy (&BYTECODE[PC], &d, sizeof (double));
	PC += sizeof (double);
}

void emit_integer (Intcon_t *ctx, int i) // Writing Integer type.
{
	memcpy (&BYTECODE[PC], &i, sizeof (int));
	PC += sizeof (int);
}

void emit_string (Intcon_t *ctx, const char* s) // Writing String type.
{
	int len = strlen (s) + 1; // +1 for null terminator.
	memcpy (&BYTECODE[PC], s, len);
	PC += len;
}

void obj_execute_runtime (Intcon_t *ctx)
{
	while (RUN) {
		uint8_t inst = BYTECODE[IP++]; // Reading the instruction (G_ prefix).

		switch (inst) 
		{
			case G_HALT:
				RUN = false;
				break;

			case G_PUSH_INT: 
			{
				int val;

				memcpy (&val, &BYTECODE[IP], sizeof (int));
				IP += sizeof (int);

				PUSH (ctx, G_PUSH_INT, i_val, val);

				printf ("Pushed INTEGER: %d\n", val);
				break;
			}

			case G_PUSH_UINT: 
			{
				uint32_t val;
				memcpy (&val, &BYTECODE[IP], sizeof (uint32_t));
				IP += sizeof (uint32_t);

				PUSH (ctx, G_PUSH_UINT, u_val, val);

				printf ("Pushed Unsigned INT: %u\n", val);
				break;
			}

			case G_PUSH_FLOAT: 
			{
				float val;
				memcpy (&val, &BYTECODE[IP], sizeof (float));
				IP += sizeof (float);

				PUSH (ctx, G_PUSH_FLOAT, f_val, val);
				
				printf ("Pushed FLOAT: %f\n", val);
				break;
			}

			case G_PUSH_STRING: 
			{
				char *sPtr = (char*) &BYTECODE[IP];
				IP += strlen (sPtr) + 1;

				PUSH (ctx, G_PUSH_STRING, sPtr, sPtr);

				printf ("Pushed STRING: %s\n", sPtr);
				break;
			}

			case G_PUSH_BOOLEAN: 
			{
				bool val = (BYTECODE[IP++] != 0); // any value that non-equals 0 => true
				PUSH (ctx, G_PUSH_BOOLEAN, b_val, val);
				printf ("Pushed BOOLEAN with %s\n", val ? "true" : "false");
				break;
			}

			case G_PUSH_DOUBLE: 
			{
				double val;
				memcpy (&val, &BYTECODE[IP], sizeof (double));
				IP += sizeof (double);
				PUSH (ctx, G_PUSH_DOUBLE, d_val, val);
				printf ("Pushed DOUBLE with value: %lf\n", val);
				break;
			}

			case G_PUSH_CHAR: 
			{
				char val = (char)BYTECODE[IP++];
				PUSH (ctx, G_PUSH_CHAR, c_val, val);
				printf ("Pushed CHAR with value: %c\n", val);
				break;
			}

			case G_PUSH_LONG: 
			{
				long val;
				memcpy (&val, &BYTECODE[IP], sizeof (long));
				IP += sizeof (long);
				PUSH (ctx, G_PUSH_LONG, l_val, val);
				printf ("Pushed LONG with value: %ld\n", val);
				break;
			}

            case G_MEMORY_ALLOCATE:
			{
				G_Value_t val_size = STACK[SP--];
				size_t size = (size_t) val_size.data.i_val;

				void* ptr = MemoryAllocate (size);

				if (ptr)
				{
					PUSH (ctx, G_PUSH_ARRAY, aPtr, ptr);
					printf ("[IVM] Allocated memory at %p\n", ptr);
				}
				else
				{
					printf ("\033[31m[ERROR]\033[0m Failed to allocate memory!\n");
					RUN = false;
				}
				break;
			}

			case G_SYS_STDIN_READ: 
			{
				// get buf size from stack
				G_Value_t size_val = STACK[SP--];
				size_t limit = (size_t)size_val.data.i_val;

				char *buffer = MemoryAllocate (limit + 1);
				if (!buffer)
				{
					RUN = false;
					break;
				}

				if (fgets (buffer, limit, stdin) != NULL)
				{
					buffer[strcspn (buffer, "\n")] = 0;

					PUSH (ctx, G_PUSH_STRING, sPtr, buffer);

				}
				else
				{
					cleanbit (buffer);
					PUSH (ctx, G_PUSH_BOOLEAN, b_val, false);
				}
				break;
			}

			case G_SYS_STDOUT_WRITE:
			case G_SYS_STDERR_WRITE: 
			{
				FILE *tar = (inst == G_SYS_STDOUT_WRITE) ? stdout : stderr;
				if (SP < 0) break;

				G_Value_t val = STACK[SP--];

				switch (val.type)
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
				G_Value_t ptr_val = STACK[SP--];
				if (ptr_val.type == G_PUSH_ARRAY)
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

					case 2:
						MemoryRenderMap ("IVM");
						break;

					case 1488:
						printf ("\033[1;33m[GITLER] You enter me...... Matrix mode engaged...\033[0m\n");
						int matrix[3][3] = { {1, 4, 2}, {3, 6, 8} };

						for (int i = 0; i < 3; i++)
						{
							for (int j = 0; j < 3; j++) 
							{
								printf ("%d ", matrix[i][j]);
							}
							printf ("\n");
						}
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
				G_Value_t b = STACK[SP--];
				G_Value_t a = STACK[SP--];

				float val_a = (a.type == G_PUSH_INT) ? (float)a.data.i_val : a.data.f_val;
				float val_b = (b.type == G_PUSH_INT) ? (float)b.data.i_val : b.data.f_val;
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
				G_Value_t b = STACK[SP--];
				G_Value_t a = STACK[SP--];

				float val_a = (a.type == G_PUSH_INT) ? (float)a.data.i_val : a.data.f_val;
				float val_b = (b.type == G_PUSH_INT) ? (float)b.data.i_val : b.data.f_val;
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

				G_Value_t val = STACK[SP--];
				bool cond = false;

				if (val.type == G_PUSH_BOOLEAN) cond = val.data.b_val;
				else if (val.type == G_PUSH_INT) cond = (val.data.i_val != 0);

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
				G_Value_t val = STACK[SP];

				printf ("Top of stack: ");
				if (val.type == G_PUSH_INT) printf ("%d\n", val.data.i_val);
				else if (val.type == G_PUSH_FLOAT) printf ("%f\n", val.data.f_val);
				else if (val.type == G_PUSH_STRING) printf ("%s\n", val.data.sPtr);
				break;
			}
		}
	}
}

#ifndef NOMAIN
int main (int argc, char **argv) 
{
	Intcon_t *ctx = obj_create ();
	if (!ctx) {
		fprintf (stderr, "\033[31m[ERROR]\033[0m Failed to create VM context!\n");
		obj_corrupt (ctx);
		return 1;
	}

	emit_byte (ctx, G_PUSH_INT);
	emit_integer (ctx, 2);
	emit_byte (ctx, G_MEMORY_TOOL);
	emit_byte (ctx, G_HALT);
	obj_execute_runtime (ctx);

	obj_corrupt (ctx);
	return 0;
}
#endif /* NOMAIN */