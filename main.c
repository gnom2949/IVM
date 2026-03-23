// main.c of IVM, runs GBC.
// to build use make.

#include "ivm.h"

int ip = 0; // Instruction pointer.
int sp = -1; // Stack pointer.
G_Value_t stack[256]; // Stack of structures.
uint8_t bytecode[1024];
int pc = 0; // Program counter.

void emitByte (uint8_t byte) // Writing into byte-code.
{
	bool running = true;
	bytecode[pc++] = byte;
}

int verifyheader (FILE *fd)
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

int openfiled (const char *filename) // It is necessary to specify the file name WITHOUT .ivm
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

	if (fSize > (long) sizeof (bytecode)) {
		fprintf (stderr, "\033[31m[ALERT]\033[0m File too large!\n");
		fclose (fd);
		return 1;
	}

	fread (bytecode, 1, fSize, fd);
	pc = (int) fSize;

	fclose (fd);
	return 0;
}

void save_to_file (const char *filename)
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

	fwrite (bytecode, 1, pc, fd);

	fclose (fd);
	printf ("\033[32m[INFO] Created file with name %s.\n", ff);
}

void emitFloat (float f) // Writing Float type.
{
	memcpy (&bytecode[pc], &f, sizeof (float));
	pc += sizeof (float);
}

void emitInteger (int i) // Writing Integer type.
{
	memcpy (&bytecode[pc], &i, sizeof (int));
	pc += sizeof (int);
}

void emitString (const char* s) // Writing String type.
{
	int len = strlen (s) + 1; // +1 for null terminator.
	memcpy (&bytecode[pc], s, len);
	pc += len;
}

void eval ()
{
	while (running) {
		uint8_t inst = bytecode[ip++]; // Reading the instruction (G_ prefix).

		switch (inst) 
		{
			case G_HALT:
				running = false;
				break;

			case G_PUSH_INT: {
				int val;

				memcpy (&val, &bytecode[ip], sizeof (int));
				ip += sizeof (int);

				PUSH (G_PUSH_INT, i_val, val);

				printf ("Pushed INTEGER: %d\n", val);
				break;
			}

			case G_PUSH_UINT: {
				uint32_t val;
				memcpy (&val, &bytecode[ip], sizeof (uint32_t));
				ip += sizeof (uint32_t);

				PUSH (G_PUSH_UINT, u_val, val);

				printf ("Pushed Unsigned INT: %u\n", val);
				break;
			}

			case G_PUSH_FLOAT: {
				float val;
				memcpy (&val, &bytecode[ip], sizeof (float));
				ip += sizeof (float);

				PUSH (G_PUSH_FLOAT, f_val, val);
				
				printf ("Pushed FLOAT: %f\n", val);
				break;
			}

			case G_PUSH_STRING: {
				char *sPtr = (char*) &bytecode[ip];
				ip += strlen (sPtr) + 1;

				PUSH (G_PUSH_STRING, sPtr, sPtr);

				printf ("Pushed STRING: %s\n", sPtr);
				break;
			}

			case G_PUSH_BOOLEAN: {
				bool val = (bytecode[ip++] != 0); // any value that non-equals 0 => true
				PUSH (G_PUSH_BOOLEAN, b_val, val);
				printf ("Pushed BOOLEAN with %s\n", val ? "true" : "false");
				break;
			}

			case G_PUSH_DOUBLE: {
				double val;
				memcpy (&val, &bytecode[ip], sizeof (double));
				ip += sizeof (double);
				PUSH (G_PUSH_DOUBLE, d_val, val);
				printf ("Pushed DOUBLE with value: %lf\n", val);
				break;
			}

			case G_PUSH_CHAR: {
				char val = (char)bytecode[ip++];
				PUSH (G_PUSH_CHAR, c_val, val);
				printf ("Pushed CHAR with value: %c\n", val);
				break;
			}

			case G_PUSH_LONG: {
				long val;
				memcpy (&val, &bytecode[ip], sizeof (long));
				ip += sizeof (long);
				PUSH (G_PUSH_LONG, l_val, val);
				printf ("Pushed LONG with value: %ld\n", val);
				break;
			}

			case G_PUSH_ARRAY: {
				int size;
				memcpy (&size, &bytecode[ip], sizeof (int));
				ip += sizeof (int);

				int *arr = (int*) calloc (size, sizeof (int));

				PUSH (G_PUSH_ARRAY, aPtr, (void*)arr);
				printf ("Pushed ARRAY of size %d\n", size);
				break;
			}

			case G_POP: {
				if (sp < 0) {
					printf ("\033[31m[ERROR]\033[0m Stack Underflow!\n");
					running = false;
					break;
				}
				sp--;
				printf ("Popped value.\n");
				break;
			}

			case G_OP_PLUS: {
				if (sp < 1) {
					printf ("\033[31m[ERROR]\033[0m Stack Underflow in PLUS OPERATOR\n");
					running = false;
					break;
				}
				G_Value_t b = stack[sp--];
				G_Value_t a = stack[sp--];

				float val_a = (a.type == G_PUSH_INT) ? (float)a.data.i_val : a.data.f_val;
				float val_b = (b.type == G_PUSH_INT) ? (float)b.data.i_val : b.data.f_val;
				float res = val_a + val_b;

				PUSH (G_PUSH_FLOAT, f_val, res);

				printf ("Added: %f + %f EQUALS %f\n", val_a, val_b,stack[sp].data.f_val);
				break;
			}

			case G_OP_MINUS: {
				if (sp < 1) {
					printf ("\033[31m[ERROR]\033[0m Stack Underflow in MINUS OPERATOR");
					running = false;
					break;
				}
				G_Value_t b = stack[sp--];
				G_Value_t a = stack[sp--];

				float val_a = (a.type == G_PUSH_INT) ? (float)a.data.i_val : a.data.f_val;
				float val_b = (b.type == G_PUSH_INT) ? (float)b.data.i_val : b.data.f_val;
				float res = val_a - val_b;

				PUSH (G_PUSH_FLOAT, f_val, res);

				printf ("Excluded: %f - %f EQUALS %f\n", val_a, val_b, stack[sp].data.f_val);
				break;
			}

			case G_JUMP: {
				int tAddr; // target address
				memcpy (&tAddr, &bytecode[ip], sizeof (int));

				ip = tAddr;

				printf ("Jumping to byte %d", tAddr);
				break;
			}

			case G_JUMP_IF_FALSE: {
				int tAddr;
				memcpy (&tAddr, &bytecode[ip], sizeof (int));
				ip += sizeof (int);

				G_Value_t val = stack[sp--];
				bool cond = false;

				if (val.type == G_PUSH_BOOLEAN) cond = val.data.b_val;
				else if (val.type == G_PUSH_INT) cond = (val.data.i_val != 0);

				if (!cond) {
					ip = tAddr;
					printf ("Condition false. Jumping to %d\n", tAddr);
				} else {
					printf ("Condition true. Continue...\n");
				}
				break;
			}

			case G_SHOW: {
				if (sp < 0) break;
				G_Value_t val = stack[sp];

				printf ("Top of stack: ");
				if (val.type == G_PUSH_INT) printf ("%d\n", val.data.i_val);
				else if (val.type == G_PUSH_FLOAT) printf ("%f\n", val.data.f_val);
				else if (val.type == G_PUSH_STRING) printf ("%s\n", val.data.sPtr);
				break;
			}
		}
	}
}


int main (int argc, char **argv) 
{
	if (argc < 2) {
		printf("No file provided. Generating 'test.ivm'...\n");
        
        emitByte(G_PUSH_INT);
        emitInteger(42);
        emitByte(G_PUSH_INT);
        emitInteger(213);
        emitByte(G_OP_PLUS);
        emitByte(G_SHOW);
        emitByte(G_HALT);
        
        save_to_file("test"); 
        return 0;
	}

	if (openfiled (argv[1]) == 0) eval();
	return 0;
}