#include <ivm/ivm.h>
#include <ivm/IntMemoryManager.h>

int main()
{
	Intcon_t *ctx = g_api_init();
	g_push_int (ctx, 100);
	g_malloc (ctx, 5 * 1024 * 1024); // 5MB
	g_mem_get_stats(ctx);
	g_free (ctx);
	g_halt(ctx);
	obj_kick (ctx);
	obj_execute_runtime (ctx);
	obj_corrupt (ctx);
	return 0;
}
