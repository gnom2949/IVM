#ifndef IVM_H
#define IVM_H
#define IVM_MNUM 0x49564D58
#define PUSH(val_type, field, value) \
        do { \
            if (sp >= 255) { printf ("\033[31m[ERROR]\033[0m Stack Overflow!"); running = false; } \
            else { \
                sp++; \
                stack[sp].type = val_type; \
                stack[sp].data.field = value; \
            } \
        } while (0)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

typedef enum {
        /*------------------ Variables Declaration ------------------*/
        G_PUSH_INT,             // push integer: G_PUSH_INT: 42/  !!INFO!! int by default be signed
        G_PUSH_FLOAT,           // push float: G_PUSH_FLOAT: 3.14/
        G_PUSH_LONG,            // push long: G_PUSH_LONG: 123456789L/
        G_PUSH_STRING,          // push string: G_PUSH_STRING: hello/
        G_PUSH_UINT,            // push unsigned int: G_PUSH_UNSIGN: value/
        G_PUSH_DOUBLE,          // push double: G_PUSH_DOUBLE: 2.71828/
        G_PUSH_ARRAY,           // push array: G_PUSH_ARRAY: arr/10/
        G_PUSH_CHAR,            // push character: G_PUSH_CHAR: 'A'/
        G_PUSH_BOOLEAN,         // push boolean: G_PUSH_BOOLEAN: true/
        G_POP,                  // pop from stack: G_POP/

        /*------------------ Pointer Operations ------------------*/
        G_PTR_PUSH,             // create pointer: G_PTR_PUSH: ptr_name/
        G_PTR_POP,              // destroy pointer: G_PTR_POP: ptr_name/
        G_PTR_GRAB,             // grab value: G_PTR_GRAB: value/ptr/
        G_PTR_RELEASE,          // release value: G_PTR_RELEASE: ptr/
        G_PTR_PUT,              // put into pointer: G_PTR_PUT: value/ptr/
        G_PTR_SWAP,             // swap pointers: G_PTR_SWAP: ptr1/ptr2/

        /*------------------ String Operations ------------------*/
        G_STRING_VALUE_PUT,     // assign string value: G_STRING_VALUE_PUT: var/value/
        G_STRING_CONCAT,        // concatenate strings: G_STRING_CONCAT: str1/str2/result/
        G_STRING_SLICE,         // slice string: G_STRING_SLICE: str/start/end/result/
        G_STRING_FIND,          // find substring: G_STRING_FIND: str/substr/pos/
        G_SETUP_STRING_CHAR,    // setup as char array: G_SETUP_STRING_CHAR: str/

        /*------------------ Memory Operations ------------------*/
        G_MEMORY_TOOL,          // memory tool: G_MEMORY_TOOL: tool/params/
        G_MEMORY_ALLOCATE,      // allocate memory: G_MEMORY_ALLOCATE: size/ptr/
        G_MEMORY_FREE,          // free memory: G_MEMORY_FREE: ptr/
        G_MEMORY_COPY,          // copy memory: G_MEMORY_COPY: src/dst/size/
        G_MEMORY_PUT,           // put value: G_MEMORY_PUT: value/addr/
        G_MEMORY_MOVE,          // move block: G_MEMORY_MOVE: src/dst/size/

        /*------------------ Basic Operations ------------------*/
        G_OP_PLUS,              // addition: G_OP_PLUS: a/b/result/
        G_OP_MINUS,             // subtraction: G_OP_MINUS: a/b/result/
        G_OP_EQUALS,            // equality: G_OP_EQUALS: a/b/result/
        G_OP_COMMA,             // comma operator: G_OP_COMMA: a/b/
        G_OP_MULT,              // multiplication: G_OP_MULT: a/b/result/
        G_OP_DIV,               // division: G_OP_DIV: a/b/result/
        G_OP_AND,               // logical AND: G_OP_AND: a/b/result/
        G_OP_OR,                // logical OR: G_OP_OR: a/b/result/
        G_OP_PIPE,              // pipe operator: G_OP_PIPE: a/b/
        G_OP_EOF,               // end of file: G_OP_EOF/
        G_OP_NOT,               // logical NOT: G_OP_NOT: a/result/
        G_OP_TRUE,              // true constant: G_OP_TRUE: result/
        G_OP_FALSE,             // false constant: G_OP_FALSE: result/

        /*------------------ Comparison Operators ------------------*/
        G_OP_MOREQ,             // greater or equal (>=): G_OP_MOREQ: a/b/result/
        G_OP_LEQ,               // less or equal (<=): G_OP_LEQ: a/b/result/
        G_OP_LESS,              // less than (<): G_OP_LESS: a/b/result/
        G_OP_HIGHER,            // greater than (>): G_OP_HIGHER: a/b/result/

        /*------------------ Loop Constructs ------------------*/
        G_LOOP_FOR,             // for loop: G_LOOP_FOR: init/cond/inc/body/
        G_LOOP_WHILE,           // while loop: G_LOOP_WHILE: cond/body/
        G_LOOP_DO_WHILE,        // do-while loop: G_LOOP_DO_WHILE: body/cond/

        /*------------------ Debug Operations ------------------*/
        CHECK_DEF,              // check define: CHECK_DEF: macro/
        CHECK_VAR,              // check variable: CHECK_VAR: var/
        CHECK_TYPE,             // check type: CHECK_TYPE: var/expected_type/
        CHECK_BOUNDS,           // check bounds: CHECK_BOUNDS: array/index/
        CHECK_NULL,             // check null: CHECK_NULL: ptr/

        /*------------------ System Calls ------------------*/
        G_SYS_CALL,             // generic syscall: G_SYS_CALL: num/args/
        G_SYS_FILE_OPEN,        // open file: G_SYS_FILE_OPEN: path/mode/fd/
        G_SYS_FILE_READ,        // read file: G_SYS_FILE_READ: fd/buf/size/
        G_SYS_FILE_WRITE,       // write file: G_SYS_FILE_WRITE: fd/buf/size/
        G_SYS_FILE_CLOSE,       // close file: G_SYS_FILE_CLOSE: fd/
        
        /*------------------ Process Operations ------------------*/
        G_SYS_PROC_OPEN,        // open process: G_SYS_PROC_OPEN: cmd/pid/
        G_SYS_PROC_CLOSE,       // close process: G_SYS_PROC_CLOSE: pid/
        
        /*------------------ Standard I/O ------------------*/
        G_SYS_STDOUT_READ,      // read from stdout: G_SYS_STDOUT_READ: buf/
        G_SYS_STDOUT_CLOSE,     // close stdout: G_SYS_STDOUT_CLOSE/
        G_SYS_STDOUT_OPEN,      // open stdout: G_SYS_STDOUT_OPEN/
        G_SYS_STDIN_OPEN,       // open stdin: G_SYS_STDIN_OPEN/
        G_SYS_STDIN_READ,       // read from stdin: G_SYS_STDIN_READ: buf/
        G_SYS_STDIN_CLOSE,      // close stdin: G_SYS_STDIN_CLOSE/
        
        /*------------------ Network Operations ------------------*/
        G_SYS_NETSOCK_OPEN,     // open socket: G_SYS_NETSOCK_OPEN: host/port/sock/
        G_SYS_NETSOCK_READ,     // read socket: G_SYS_NETSOCK_READ: sock/buf/
        G_SYS_NETSOCK_ACCEPT,   // accept connection: G_SYS_NETSOCK_ACCEPT: sock/client/
        G_SYS_NETSOCK_CLOSE,    // close socket: G_SYS_NETSOCK_CLOSE: sock/
        
        /*------------------ Signal Handling ------------------*/
        G_SYS_SIGNAL_GET,       // get signal: G_SYS_SIGNAL_GET: signum/handler/
        G_SYS_SIGNAL_DO,        // send signal: G_SYS_SIGNAL_DO: pid/signum/

        /*------------------ Meta Instructions ------------------*/
        G_INCLUDE,              // include file: G_INCLUDE: filename/
        G_DEFINE,               // define macro: G_DEFINE: macro/value/
        G_IFDEF,                // if defined: G_IFDEF: macro/
        G_IFNOTDEF,            // if not defined: G_IFNOTDEF: macro/
        G_ELSE,                 // else: G_ELSE/
        G_ENDIF,                // endif: G_ENDIF/

        /*------------------ End of Instructions ------------------*/
        G_INSTRUCTION_COUNT,     // total number of instructions
		G_HALT = 47,
		G_SHOW,
		G_JUMP,
		G_JUMP_IF_FALSE
} gOpCode;

typedef struct {
	gOpCode type; 
	union {
		int i_val; // Int value.
        uint32_t u_val; // Unsigned Integer value.
		float f_val; // Float value.
		char *sPtr; // string pointer.
        long l_val; // Long value.
        double d_val; // Double value (lf).
        char c_val; // Charater value.
        bool b_val; // Bool value.
        char *aPtr; // for arrays/pointers.
    } data;
} G_Value_t;

typedef struct {
	uint32_t magic;
	uint16_t version;
	uint16_t flags;
	uint32_t code_size;
} G_Header_t;

extern int ip; // Instruction pointer.
extern int sp; // Stack pointer.
extern bool running = true; 
extern int pc; // Program counter.


void emitByte (uint8_t byte);
int verifyHeader (FILE *fd);
int openfiled (const char *filename);
void save_to_file (const char *filename);
void emitFloat (float f);
void emitInteger (int i);
void emitString (const char* str);
void eval();

#endif /* IVM_H */