# IVM
What's is IVM? 
IVM is a Int Virtual Machine a virtual machine that uses IIBC an bytecode designed to be user-friendly.

## Dependencies

For IVM, you need:
    * POSIX-compactable system like linux/BSD.
    * GoogleTest Framework Developer.
So let's install them:
**Fedora** `sudo dnf install gtest-devel`
**Debian** `sudo apt install libgtest-dev`
**Arch** `sudo pacman -S gtest`
**Alpine** `apk add gtest-dev`

## Install
To install ivm, do this commands:
first ` sudo make install `.
second ` sudo make test `.
third ` make clean `.

## Files
Ivm can use only .ivm file with magic number.
So how to create an files???

For this we need an API. 
# API
IVM provides an library and specification, but to manage this library you need a API, so here it is:
|Function|Does|
|:---|---:|
| g_api_init | Initializes api |
| g_api_save| Saves file |
| g_push_int| Pushes int to the stack|
| g_push_double| Pushes double to the stack |
| g_push_float | Pushes float to the stack |
| g_push_string | Pushes string to the stack |
| g_malloc | Allocates memory for IVM|
| g_free | Freed memory for IVM|
| g_mem_get_stats | Getting stats from IVM's Memory Manager|
| g_mem_map | Renders Memory Map by the data of IVM's Memory Manager|
| g_halt | Stopping the instructions stack input |
| g_api_corrupt | Stop's api |
| g_api_load | Load's file |

But IVM has a low-level api

|Function|Does|
|:---|---:|
| emit_byte | Emit's byte, first step before pushing any value to the stack|
| obj_verifyheader | verifies a magic number that's creating when user creates file |
| obj_loadfile | loades file |
| obj_kick | Does 'Kick', set's sp on -1, ip on 0 and run var on true |
| obj_execute_runtime | The heart of IVM, run's runtime |
| obj_create | Creates object env |
| obj_corrupt | Corrupt this object |
| emit_integer | Emit integer, DO AFTER 'emit_byte' |
| emit_float | Emit float, DO AFTER 'emit_byte' |
| emit_double | Emit double, DO AFTER 'emit_byte' |
| emit_string | Emit string, DO AFTER 'emit_byte' |



## Examples

First C api.
```C
#include <ivm/ivm.h> // includes a main header.
#include <imm/IntMemoryManager.h> // imm.

int main()
{
	Intcon_t *ctx = g_api_init(); // There we create a context with api init.
	g_push_int (ctx, 100); // pushes int, in api we don't need an emit's, api do this automaticly. 
	g_malloc (ctx, 5 * 1024 * 1024); // allocate memory in size of 5MB.
	g_mem_get_stats(ctx); // getting statistics.
	g_free (ctx); // freed memory.
	g_halt(ctx); // stop instructions.
	obj_execute_runtime (ctx); // executing.
	obj_corrupt (ctx); // corrupting ctx.
	return 0;
}
```

Let's compile it! (but ran make install first).
`cc -livm -o example example.c`

