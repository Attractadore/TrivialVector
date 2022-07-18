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

TEST(TestConstruct, MoveConstruct) {
    std::array arr = {1, 2, 3, 4, 5};
    TrivialVector<int> vec(arr);
    auto old_data = vec.data();
    auto old_capacity = vec.capacity();
    auto old_size = vec.size();

    auto vec2 = std::move(vec);

    EXPECT_EQ(vec.capacity(), 0);
    EXPECT_TRUE(vec.empty());

    EXPECT_EQ(vec2.data(), old_data);
    EXPECT_EQ(vec2.capacity(), old_capacity);
    EXPECT_EQ(vec2.size(), old_size);

    EXPECT_TRUE(std::ranges::equal(vec2, arr));
}

TEST(TestConstruct, ExtendedMoveConstructNoAllocAlwaysSuccessSteal) {
    std::array arr = {1, 2, 3, 4, 5};
    TrivialVector<int> vec(arr);
    auto old_capacity = vec.capacity();
    auto old_data = vec.data();
    auto old_size = vec.size();

    InlineTrivialVector<int, 4> vec2 = std::move(vec);

    EXPECT_EQ(vec.capacity(), 0);
    EXPECT_TRUE(vec.empty());

    EXPECT_EQ(vec2.data(), old_data);
    EXPECT_EQ(vec2.capacity(), old_capacity);
    EXPECT_EQ(vec2.size(), old_size);

    EXPECT_TRUE(std::ranges::equal(vec2, arr));
}

TEST(TestConstruct, ExtendedMoveConstructNoAllocAlwaysSuccessCopy) {
    std::array arr = {1, 2, 3, 4, 5};
    InlineTrivialVector<int, 5> vec(arr);
    auto old_data = vec.data();
    auto old_capacity = vec.capacity();
    auto old_size = vec.size();

    InlineTrivialVector<int, 6> vec2 = std::move(vec);

    EXPECT_EQ(vec.data(), old_data);
    EXPECT_EQ(vec.capacity(), old_capacity);
    EXPECT_TRUE(vec.empty());

    EXPECT_NE(vec2.data(), old_data);
    EXPECT_EQ(vec2.capacity(), vec2.max_inline_size());
    EXPECT_EQ(vec2.size(), old_size);

    EXPECT_TRUE(std::ranges::equal(vec2, arr));
}

TEST(TestConstruct, ExtendedMoveConstructNoAllocAlwaysSuccessStealOrCopy) {
    std::array arr = {1, 2, 3, 4, 5};
    TrivialVector<int> vec(arr);
    auto old_data = vec.data();
    auto old_capacity = vec.capacity();
    auto old_size = vec.size();

    InlineTrivialVector<int, 6> vec2 = std::move(vec);

    bool stolen = vec.capacity() == 0;
    if (stolen) {
        EXPECT_EQ(vec.capacity(), 0);
        EXPECT_TRUE(vec.empty());

        EXPECT_EQ(vec2.data(), old_data);
        EXPECT_EQ(vec2.capacity(), old_capacity);
        EXPECT_EQ(vec2.size(), old_size);
    } else {
        EXPECT_EQ(vec.data(), old_data);
        EXPECT_EQ(vec.capacity(), old_capacity);
        EXPECT_TRUE(vec.empty());

        EXPECT_NE(vec2.data(), old_data);
        EXPECT_EQ(vec2.capacity(), vec2.max_inline_size());
        EXPECT_EQ(vec2.size(), old_size);
    }

    EXPECT_TRUE(std::ranges::equal(vec2, arr));
}

TEST(TestConstruct, ExtendedMoveConstructNoAllocSuccessSteal) {
    std::array arr = {1, 2, 3, 4, 5};
    InlineTrivialVector<int, 4> vec(arr);
    auto old_capacity = vec.capacity();
    auto old_data = vec.data();
    auto old_size = vec.size();

    InlineTrivialVector<int, 3> vec2 = std::move(vec);

    EXPECT_EQ(vec.capacity(), vec.max_inline_size());
    EXPECT_TRUE(vec.empty());

    EXPECT_EQ(vec2.data(), old_data);
    EXPECT_EQ(vec2.capacity(), old_capacity);
    EXPECT_EQ(vec2.size(), old_size);

    EXPECT_TRUE(std::ranges::equal(vec2, arr));
}

TEST(TestConstruct, ExtendedMoveConstructNoAllocSuccessCopy) {
    std::array arr = {1, 2, 3, 4, 5};
    InlineTrivialVector<int, 6> vec(arr);
    auto old_data = vec.data();
    auto old_capacity = vec.capacity();
    auto old_size = vec.size();

    InlineTrivialVector<int, 5> vec2 = std::move(vec);

    EXPECT_EQ(vec.data(), old_data);
    EXPECT_EQ(vec.capacity(), old_capacity);
    EXPECT_TRUE(vec.empty());

    EXPECT_NE(vec2.data(), old_data);
    EXPECT_EQ(vec2.capacity(), vec2.max_inline_size());
    EXPECT_EQ(vec2.size(), old_size);

    EXPECT_TRUE(std::ranges::equal(vec2, arr));
}

TEST(TestConstruct, ExtendedMoveConstructNoAllocFail) {
    std::array arr = {1, 2, 3, 4, 5};
    InlineTrivialVector<int, 6> vec(arr);
    auto old_data = vec.data();
    auto old_capacity = vec.capacity();
    auto old_size = vec.size();

    InlineTrivialVector<int, 4> vec2 = std::move(vec);

    EXPECT_EQ(vec.data(), old_data);
    EXPECT_EQ(vec.capacity(), old_capacity);
    EXPECT_TRUE(vec.empty());

    EXPECT_GT(vec2.capacity(), vec2.max_inline_size());
    EXPECT_EQ(vec2.size(), old_size);

    EXPECT_TRUE(std::ranges::equal(vec2, arr));
}

TEST(TestConstruct, FromInitListNoAlloc) {
    std::array arr = {1, 2, 3, 4, 5};
    InlineTrivialVector<int, 5> vec = {1, 2, 3, 4, 5};
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(vec.capacity(), vec.max_inline_size());
    EXPECT_TRUE(std::ranges::equal(vec, arr));
}

TEST(TestConstruct, FromInitListAlloc) {
    std::array arr = {1, 2, 3, 4, 5};
    TrivialVector<int> vec = {1, 2, 3, 4, 5};
    EXPECT_EQ(vec.size(), 5);
    EXPECT_GT(vec.capacity(), vec.max_inline_size());
    EXPECT_TRUE(std::ranges::equal(vec, arr));
}

TEST(TestCopyAssign, Inline) {
    InlineTrivialVector<int, 5> vec;
    auto old_data = vec.data();
    const InlineTrivialVector<int, 5> vec2 = {1, 2, 3, 4, 5};
    vec = vec2;

    EXPECT_EQ(vec.size(), vec2.size());
    bool is_inline = vec.capacity() == vec.max_inline_size();
    if (is_inline) {
        EXPECT_EQ(vec.data(), old_data);
    } else {
        EXPECT_NE(vec.data(), old_data);
        EXPECT_GT(vec.capacity(), vec.max_inline_size());
    }

    EXPECT_TRUE(std::ranges::equal(vec, vec2));
}

TEST(TestCopyAssign, Heap) {
    TrivialVector<int> vec;
    auto old_data = vec.data();
    const TrivialVector<int> vec2 = {1, 2, 3, 4, 5};
    vec = vec2;

    EXPECT_NE(vec.data(), old_data);
    EXPECT_GT(vec.capacity(), vec.max_inline_size());
    EXPECT_EQ(vec.size(), vec2.size());

    EXPECT_TRUE(std::ranges::equal(vec, vec2));
}

TEST(TestCopyAssign, SelfAssignInline) {
    std::array arr = {1, 2, 3, 4, 5};
    InlineTrivialVector<int, 5> vec(arr);
    auto old_data = vec.data();
    auto old_capacity = vec.capacity();
    auto old_size = vec.size();
    vec = const_cast<const decltype(vec)&>(vec);

    EXPECT_EQ(vec.data(), old_data);
    EXPECT_EQ(vec.capacity(), old_capacity);
    EXPECT_EQ(vec.size(), old_size);

    EXPECT_TRUE(std::ranges::equal(vec, arr));
}

TEST(TestCopyAssign, SelfAssignHeap) {
    std::array arr = {1, 2, 3, 4, 5};
    TrivialVector<int> vec(arr);
    auto old_data = vec.data();
    auto old_capacity = vec.capacity();
    auto old_size = vec.size();
    vec = const_cast<const decltype(vec)&>(vec);

    EXPECT_EQ(vec.data(), old_data);
    EXPECT_EQ(vec.capacity(), old_capacity);
    EXPECT_EQ(vec.size(), old_size);

    EXPECT_TRUE(std::ranges::equal(vec, arr));
}

TEST(TestMoveAssign, Copy) {
    std::array arr = {1, 2, 3, 4, 5};
    InlineTrivialVector<int, 5> vec;
    auto old_data = vec.data();
    InlineTrivialVector<int, 5> vec2(arr);
    auto old_data2 = vec2.data();
    auto old_size = vec2.size();
    vec = std::move(vec2);

    EXPECT_EQ(vec2.data(), old_data2);
    EXPECT_EQ(vec2.capacity(), vec2.max_inline_size());
    EXPECT_TRUE(vec2.empty());

    EXPECT_EQ(vec.data(), old_data);
    EXPECT_EQ(vec.capacity(), vec.max_inline_size());
    EXPECT_EQ(vec.size(), old_size);

    EXPECT_TRUE(std::ranges::equal(vec, arr));
}

TEST(TestMoveAssign, Steal) {
    std::array arr = {1, 2, 3, 4, 5};
    TrivialVector<int> vec;
    TrivialVector<int> vec2(arr);
    auto old_data = vec2.data();
    auto old_capacity = vec2.capacity();
    auto old_size = vec2.size();
    vec = std::move(vec2);

    EXPECT_EQ(vec2.capacity(), vec2.max_inline_size());
    EXPECT_TRUE(vec2.empty());

    EXPECT_EQ(vec.data(), old_data);
    EXPECT_EQ(vec.capacity(), old_capacity);
    EXPECT_EQ(vec.size(), old_size);

    EXPECT_TRUE(std::ranges::equal(vec, arr));
}

TEST(TestMoveAssign, SelAssignCopy) {
    std::array arr = {1, 2, 3, 4, 5};
    InlineTrivialVector<int, 5> vec(arr);
    auto old_data = vec.data();
    auto old_capacity = vec.capacity();
    vec = std::move(vec);

    EXPECT_EQ(vec.data(), old_data);
    EXPECT_EQ(vec.capacity(), vec.max_inline_size());
    EXPECT_TRUE(vec.empty());
}

TEST(TestMoveAssign, SelfAssignSteal) {
    std::array arr = {1, 2, 3, 4, 5};
    TrivialVector<int> vec(arr);
    auto old_capacity = vec.capacity();
    vec = std::move(vec);

    EXPECT_EQ(vec.capacity(), vec.max_inline_size());
    EXPECT_TRUE(vec.empty());
}

TEST(TestInitListAssign, InitList) {
    std::array arr = {1, 2, 3, 4, 5};
    TrivialVector<int> vec;
    vec = {1, 2, 3, 4, 5};
    EXPECT_TRUE(std::ranges::equal(vec, arr));
}

struct TestSwap: testing::Test {
    std::array<int, 5> arr1 = {1, 2, 3, 4, 5};
    std::array<int, 4> arr2 = {5, 4, 3, 2};
    std::array<int, 5> arr3 = {5, 4, 3, 2, 1};
    static constexpr auto sz1 = std::tuple_size_v<decltype(arr1)>;
    static constexpr auto sz2 = std::tuple_size_v<decltype(arr2)>;
    static constexpr auto sz3 = std::tuple_size_v<decltype(arr2)>;
};

TEST_F(TestSwap, PointerSwap) {
    TrivialVector<int> vec1(arr1);
    TrivialVector<int> vec2(arr2);
    vec1.swap(vec2);
    EXPECT_TRUE(std::ranges::equal(vec1, arr2));
    EXPECT_TRUE(std::ranges::equal(vec2, arr1));
}

TEST_F(TestSwap, InlineCopySwap) {
    static constexpr auto max_sz = std::max(sz1, sz2);
    InlineTrivialVector<int, max_sz> vec1(arr1);
    InlineTrivialVector<int, max_sz> vec2(arr2);
    vec1.swap(vec2);
    EXPECT_TRUE(std::ranges::equal(vec1, arr2));
    EXPECT_TRUE(std::ranges::equal(vec2, arr1));
}

TEST_F(TestSwap, InlineReallocCopySwap) {
    InlineTrivialVector<int, sz1> vec1(arr1);
    InlineTrivialVector<int, sz2> vec2(arr2);
    vec1.swap(vec2);
    EXPECT_TRUE(std::ranges::equal(vec1, arr2));
    EXPECT_TRUE(std::ranges::equal(vec2, arr1));
}

TEST_F(TestSwap, InlineHeapHybridSwap) {
    InlineTrivialVector<int, sz2> vec1(arr2);
    InlineTrivialVector<int, sz2> vec2(arr3);
    vec1.swap(vec2);
    EXPECT_TRUE(std::ranges::equal(vec1, arr3));
    EXPECT_TRUE(std::ranges::equal(vec2, arr2));
    vec2.swap(vec1);
    EXPECT_TRUE(std::ranges::equal(vec1, arr2));
    EXPECT_TRUE(std::ranges::equal(vec2, arr3));
}

TEST_F(TestSwap, InlineHeapCopySwap) {
    InlineTrivialVector<int, sz1> vec1(arr1);
    InlineTrivialVector<int, sz2> vec2(arr3);
    vec1.swap(vec2);
    EXPECT_TRUE(std::ranges::equal(vec1, arr3));
    EXPECT_TRUE(std::ranges::equal(vec2, arr1));
}

TEST_F(TestSwap, InlineHeapReallocPointerSwap) {
    InlineTrivialVector<int, sz2> vec1(arr2);
    TrivialVector<int> vec2(arr3);
    vec1.swap(vec2);
    EXPECT_TRUE(std::ranges::equal(vec1, arr3));
    EXPECT_TRUE(std::ranges::equal(vec2, arr2));
}

TEST(TestAssign, FillWithValueRealloc) {
    TrivialVector<int> vec;
    int val = 0;
    auto sz = 5;
    vec.assign(sz, val);
    ASSERT_EQ(vec.size(), sz);
    for (size_t i = 0; i < sz; i++) {
        EXPECT_EQ(vec.data()[i], val);
    }
}

TEST(TestAssign, FillWithValueNoReallocInline) {
    InlineTrivialVector<int, 6> vec;
    auto old_data = vec.data();
    auto old_capacity = vec.capacity();

    int val = 0;
    auto sz = 5;
    vec.assign(sz, val);

    ASSERT_EQ(vec.data(), old_data);
    EXPECT_EQ(vec.capacity(), old_capacity);
    ASSERT_EQ(vec.size(), sz);
    for (size_t i = 0; i < sz; i++) {
        EXPECT_EQ(vec.data()[i], val);
    }
}

TEST(TestAssign, FillWithValueNoReallocHeap) {
    constexpr int val = 0;
    constexpr auto sz = 5;

    TrivialVector<int> vec(sz);
    auto old_data = vec.data();
    auto old_capacity = vec.capacity();

    vec.assign(sz, val);

    ASSERT_EQ(vec.data(), old_data);
    EXPECT_EQ(vec.capacity(), old_capacity);
    ASSERT_EQ(vec.size(), sz);
    for (size_t i = 0; i < sz; i++) {
        EXPECT_EQ(vec.data()[i], val);
    }
}
