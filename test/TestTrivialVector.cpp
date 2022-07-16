#include "Attractadore/TrivialVector.hpp"

#include <gtest/gtest.h>

#include <list>

using Attractadore::InlineTrivialVector;
using Attractadore::TrivialVector;

TEST(TestConstruct, Default) {
    TrivialVector<int> vec;
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.capacity(), 0);
}

TEST(TestConstruct, FromAllocator) {
    using Vec = TrivialVector<int>;
    auto alloc = Vec::allocator_type{};
    Vec vec(alloc);
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.get_allocator(), alloc);
    EXPECT_EQ(vec.capacity(), 0);
}

TEST(TestConstruct, WithSize) {
    auto sz = 16;
    TrivialVector<int> vec(sz);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(vec.size(), sz);
    EXPECT_GE(vec.capacity(), sz);
}

TEST(TestConstruct, WithFill) {
    int val = 55;
    auto sz = 16;
    TrivialVector<int> vec(sz, val);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(vec.size(), sz);
    EXPECT_GE(vec.capacity(), sz);
    EXPECT_EQ(std::ranges::count(vec, val), vec.size());
}

TEST(TestConstruct, FromFwdIter) {
    std::list lst = {1, 2, 3, 4, 5};
    TrivialVector<int> vec(lst.begin(), lst.end());
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(vec.size(), lst.size());
    EXPECT_TRUE(std::ranges::equal(vec, lst));
}

TEST(TestConstruct, FromRAIter) {
    std::array arr = {1, 2, 3, 4, 5};
    TrivialVector<int> vec(arr.begin(), arr.end());
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(vec.size(), arr.size());
    EXPECT_TRUE(std::ranges::equal(vec, arr));
}

TEST(TestConstruct, FromUnsizedRange) {
    std::list lst = {1, 2, 3, 4, 5};
    auto r = std::ranges::subrange(lst.begin(), lst.end());
    static_assert(not std::ranges::sized_range<decltype(r)>);
    TrivialVector<int> vec(r);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(vec.size(), lst.size());

    EXPECT_TRUE(std::ranges::equal(vec, lst));
}

TEST(TestConstruct, FromSizedRange) {
    std::array arr = {1, 2, 3, 4, 5};
    TrivialVector<int> vec(arr);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(vec.size(), arr.size());
    EXPECT_TRUE(std::ranges::equal(vec, arr));
}

TEST(TestConstruct, CopyConstruct) {
    std::array arr = {1, 2, 3, 4, 5};
    TrivialVector<int> vec(arr);
    auto vec2 = vec;
    EXPECT_EQ(vec.size(), vec2.size());
    EXPECT_TRUE(std::ranges::equal(vec, vec2));
}

TEST(TestConstruct, ExtendedCopyConstruct) {
    std::array arr = {1, 2, 3, 4, 5};
    {
        InlineTrivialVector<int, 5> vec(arr);
        TrivialVector<int> vec2 = vec;
        EXPECT_EQ(vec.size(), vec2.size());
        EXPECT_TRUE(std::ranges::equal(vec, vec2));
    } {
        TrivialVector<int> vec(arr);
        InlineTrivialVector<int, 3> vec2 = vec;
        EXPECT_EQ(vec.size(), vec2.size());
        EXPECT_TRUE(std::ranges::equal(vec, vec2));
    }
}
