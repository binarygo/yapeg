#include <gtest/gtest.h>
#include <yapeg_any.h>
#include <memory>
#include <utility>

namespace yapeg {

struct Foo
{
    int d_i;
    Foo(int i): d_i(i) {}
    bool operator== (const Foo& rhs) const
    {
        return d_i == rhs.d_i;
    }
};
    
TEST(Any, ctor)
{
    Any a;
}

TEST(Any, copy_ctor)
{
    Any a;
    a.set<int>(10);

    Any b(a);
    EXPECT_EQ(b.get<int>(), 10);
}

TEST(Any, move_ctor)
{
    Any a;
    a.set<int>(10);

    Any b(std::move(a));
    EXPECT_EQ(b.get<int>(), 10);
    EXPECT_TRUE(a.isNone());
}

TEST(Any, copy_assign)
{
    Any a;
    a.set<int>(10);
    EXPECT_EQ(a.get<int>(), 10);
    
    Any b;
    b.set<char>('x');
    EXPECT_EQ(b.get<char>(), 'x');

    b = a;
    EXPECT_EQ(b.get<int>(), 10);
    EXPECT_EQ(a.get<int>(), 10);
}

TEST(Any, move_assign)
{
    Any a;
    a.set<int>(10);
    EXPECT_EQ(a.get<int>(), 10);
    
    Any b;
    b.set<char>('x');
    EXPECT_EQ(b.get<char>(), 'x');

    b = std::move(a);
    EXPECT_EQ(b.get<int>(), 10);
    EXPECT_TRUE(a.isNone());
}

TEST(Any, clear)
{
    Any a;
    a.set<int>(10);
    EXPECT_EQ(a.get<int>(), 10);

    a.clear();
    EXPECT_TRUE(a.isNone());
}
    
TEST(Any, set_get_1)
{
    Any a;
    a.set(3);
}
    
TEST(Any, set_get_2)
{
    Any a;
    a.set<int>(6);
    EXPECT_EQ(a.get<int>(), 6);
}

TEST(Any, set_get_3)
{
    Any a;
    a.set<int>(6);
    a.set<double>(2.0);
    a.set<Foo>(Foo(1));
    a.set<float>(6.18);
    a.set<int>(10);
    EXPECT_EQ(a.get<int>(), 10);
    a.set<Foo>(Foo(2));
    EXPECT_EQ(a.get<Foo>(), Foo(2));
    EXPECT_THROW(a.get<int>(), Any::TypeMismatch);
}

TEST(Any, set_get_4)
{
    const Foo& foo(3);

    Any a;
    a.set(foo);
}
    
} // close namespace yapeg

