/* testapi.cpp The test of ivm-api for IVM™.
Google Test (Gtest) as Dependency.
Copyright (C) 2026 Aleksandr Silaev.
C/C++/Zig.
IVM TEST ONLY.
*/

#include <gtest/gtest.h>

extern "C" {
    #include <ivm/ivm.h>
    #include <imm/IntMemoryManager.h>
}
class TestAPI : public ::testing::Test
{
protected:
    Intcon_t *ctx;

    void SetUp() override {
        ctx = g_api_init();
    }

    
    void TearDown() override {
        g_api_corrupt(ctx);     
    }
};

TEST_F (TestAPI, PushBC)
{
    g_push_int (ctx, 1337);

    EXPECT_EQ (ctx->bytecode[0], G_PUSH_INT);

    int val;
    memcpy (&val, &ctx->bytecode[1], sizeof (int));
    EXPECT_EQ (val, 1337);
    EXPECT_EQ (ctx->pc, 1 + sizeof (int));
}

TEST_F (TestAPI, VmAlloc)
{
    g_malloc (ctx, 256);

    EXPECT_EQ (ctx->bytecode[0], G_PUSH_INT);
    EXPECT_EQ (ctx->bytecode[1 + sizeof(int)], G_MEMORY_ALLOCATE);
}

TEST_F (TestAPI, ExecuteAddSequence)
{
    g_push_int (ctx, 10);
    g_push_int (ctx, 20);
    emit_byte (ctx, G_OP_PLUS);
    g_halt (ctx);

    obj_execute_runtime (ctx);

    ASSERT_EQ (ctx->sp, 0);
    EXPECT_FLOAT_EQ (ctx->stack[ctx->sp].data.f_val, 30.0f);
} 