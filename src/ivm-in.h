/* ivm-in.c, The internal IVM header, IVM™.
* Copyright(C) 2026 Aleksandr Silaev.
* SPDX-License-Identifier: LGPL-3-or-later
* Licensed under GNU Lesser General Public License v3.
* IVM™ IS A FREE SOFTWARE.
*/
#ifndef __IVM_IN_H__
#define __IVM_IN_H__

#include <stdint.h>
#include <stdbool.h>
#include "../include/arena.h"
#include "../include/ivm-string.h"

#define IVM_MNUM 0x49564D58
#define IVM_VERSION "0.3.0"
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
                (ctx)->stack[(ctx)->sp].op = val_type; \
                (ctx)->stack[(ctx)->sp].data.field = value; \
            } \
        } while (0)

#define IVM_SANDBOX_MAX_ALLOCS 128
#define IVM_SANDBOX_ARENA_LIMIT (4 * 1024 * 1024)

typedef struct {
    void *ptr;
    __SIZE_TYPE__ size;
    bool alive;
} IVMAllocSlot;

typedef struct {
    int startAddr;
    int endAddr;
}IVMLoopWhile;

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
        G_PUSH_HANDLE,          // push sandbox memory handle(opaque index, NEVER a raw pointer), generic byte buffer, no len header.
        G_PUSH_IVMSTRING,       // push an safe IVMString handle
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
        
        /*------------------ End of Instructions ------------------*/
		G_HALT,
		G_SHOW,
		G_JUMP,
		G_JUMP_IF_FALSE,
        G_INSTRUCTION_COUNT     // total number of instructions
} IVMOperation;

typedef struct {
	IVMOperation op; 
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
} IVMCell;

typedef struct {
	uint32_t magic;
	uint16_t version;
	uint16_t flags;
	uint32_t code_size;
} IVMHeader;

typedef struct {
    int ip; // Instruction pointer.
    int sp; // Stack pointer. 
    int pc; // Program counter.
    IVMCell stack[IVM_STACK_SIZE];
    uint8_t bytecode[IVM_BYTE_SIZE];
    bool running;
    IvsArena mem;
    IVMAllocSlot slots[IVM_SANDBOX_MAX_ALLOCS];
    int slot_count;
} IVMContext;

#endif