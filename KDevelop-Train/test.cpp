#include <gtest/gtest.h>

// 最简单的测试用例
TEST(MyTest, Test1)
{
    int a = 1;
    int b = 1;
    EXPECT_EQ(a, b);
}

TEST(MyTest, Test2)
{
    bool ok = true;
    ASSERT_TRUE(ok);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}