/* test1.cpp The test for IVM™.
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


class TestOne : public ::testing::Test
{
protected:
    Intcon_t *ctx;

    void SetUp() override {
        ctx = obj_create();   
        obj_kick(ctx);  
    }

    
    void TearDown() override {
        obj_corrupt(ctx);     
    }
};


TEST_F (TestOne, AdditionTest)
{
    emit_byte (ctx, G_PUSH_INT);
    emit_integer (ctx, 2);
    emit_byte (ctx, G_PUSH_INT);
    emit_integer (ctx, 3);
    emit_byte (ctx, G_OP_PLUS);
    emit_byte (ctx, G_HALT);

    obj_execute_runtime (ctx);

    ASSERT_EQ (ctx->sp, 0);
    EXPECT_FLOAT_EQ (ctx->stack[ctx->sp].data.f_val, 5.0f);
}

TEST_F (TestOne, StackPushPopTest)
{
    emit_byte (ctx, G_PUSH_INT);
    emit_integer (ctx, 100);
    emit_byte (ctx, G_POP);
    emit_byte (ctx, G_HALT);

    obj_execute_runtime (ctx);

    EXPECT_EQ (ctx->sp, -1);
}

TEST_F (TestOne, StringTest)
{
    const char *testString = "Hello IVM!";
    emit_byte (ctx, G_PUSH_STRING);
    emit_string (ctx, testString);
    emit_byte (ctx, G_HALT);

    obj_execute_runtime (ctx);

    ASSERT_EQ (ctx->sp, 0);
    EXPECT_STREQ (ctx->stack[ctx->sp].data.sPtr, testString);
}

TEST_F (TestOne, JumpIfFalseTest)
{
    emit_byte (ctx, G_PUSH_BOOLEAN);
    emit_byte (ctx, 0);

    emit_byte (ctx, G_JUMP_IF_FALSE);
    int jumpTar = 7;
    emit_integer (ctx, jumpTar);

    emit_byte (ctx, G_HALT);

    obj_execute_runtime (ctx);

    EXPECT_EQ (ctx->ip, jumpTar + 1);
}