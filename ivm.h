#ifndef IVM_H
#define IVM_H
#if defined(__cplusplus)
    extern "C" {
#endif /* __cplusplus */
#define IVM_MNUM 0x49564D58
#define IVM_VERSION "0.2.1"
#define IVM_STACK_SIZE 256
#define IVM_BYTE_SIZE 1024
#define WIDTH 50
#define PUSH(ctx, val_type, field, value) \
        do { \
            if ((ctx)->sp >= IVM_STACK_SIZE - 1) \
            { \
                fprintf (stderr, "\033[31m[ERROR]\033[0m Stack Overflow!"); \
                (ctx)->running = false; \
            } \
            else { \
                (ctx)->sp++; \
                (ctx)->stack[(ctx)->sp].type = val_type; \
                (ctx)->stack[(ctx)->sp].data.field = value; \
            } \
        } while (0)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#define true 1
#define false 0

typedef struct {
    int startAddr;
    int endAddr;
}LoopWhile;

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
        G_SYS_STDIN_READ,       // read from stdin(0): G_SYS_STDIN_READ: buf/
        G_SYS_STDOUT_WRITE,     // write to stdout(1): G_SYS_STDOUT_WRITE: value/buf/
        G_SYS_STDERR_WRITE,     // write to stderr(2): G_SYS_STDOUT_WRITE: value/buf/
        
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
		G_HALT,
		G_SHOW,
		G_JUMP,
		G_JUMP_IF_FALSE,
        G_INSTRUCTION_COUNT     // total number of instructions
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

typedef struct {
    int ip; // Instruction pointer.
    int sp; // Stack pointer. 
    int pc; // Program counter.
    G_Value_t stack[IVM_STACK_SIZE];
    uint8_t bytecode[IVM_BYTE_SIZE];
    bool running;
} Intcon_t; 

//extern int ip; // Instruction pointer.
//extern int sp; // Stack pointer. 
//extern int pc; // Program counter.
//extern uint8_t bytecode[1024];

void emit_byte (Intcon_t *ctx, uint8_t byte);
int obj_verifyHeader (FILE *fd);
int obj_loadfile (Intcon_t *ctx, const char *filename);
void emit_float (Intcon_t *ctx, float f);
void obj_savefile (Intcon_t *ctx, const char *filename);
void emit_integer (Intcon_t *ctx, int i);
void emit_string (Intcon_t *ctx, const char *str);
void emit_double (Intcon_t *ctx, double d);
/* ================================= API ============================================= */
// Manage the runtime //
void obj_execute_runtime (Intcon_t *ctx);
Intcon_t *g_api_init();
void g_api_save (Intcon_t *ctx, const char *filename);
void g_api_load (Intcon_t *ctx, const char *filename);
void g_api_corrupt (Intcon_t *ctx);
Intcon_t *obj_create (void);
void obj_corrupt (Intcon_t *cxt);
void obj_kick (Intcon_t *ctx);

// Stack commands //
void g_push_int (Intcon_t *ctx, int value);
void g_push_float (Intcon_t *ctx, float value);
void g_push_double (Intcon_t *ctx, double value);
void g_push_string (Intcon_t *ctx, const char *string);
int g_get_label (Intcon_t *ctx);
void g_halt(Intcon_t *ctx);
void g_push_boolean (Intcon_t *ctx, bool value);
void g_push_uint8 (Intcon_t *ctx, uint8_t value);
void g_push_char (Intcon_t *ctx, char character);
void g_api_add();
void g_api_show();

// Memory management
void g_malloc (Intcon_t *ctx, size_t size);
void g_free (Intcon_t *ctx);

// Debug functions
void g_debug_patch_addr (Intcon_t *ctx, int addresToPatch, int targetValue);
void g_mem_get_stats (Intcon_t *ctx);
void g_mem_map(Intcon_t *ctx);

// loop's
LoopWhile g_while_start (Intcon_t *ctx);
LoopWhile g_while_end (Intcon_t *ctx);
void g_while_cond_check (Intcon_t *ctx, LoopWhile *w);

#endif /* IVM_H */

#if defined(__cplusplus)
    }
#endif /* __cplusplus */