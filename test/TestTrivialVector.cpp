#include "Attractadore/TrivialVector.hpp"

#include <gtest/gtest.h>

#include <list>

using Attractadore::InlineTrivialVector;
using Attractadore::TrivialVector;

namespace Attractadore::TrivialVectorNameSpace {
template<typename T, unsigned InlineCapacity>
std::ostream& operator<<(
    std::ostream& os,
    const InlineTrivialVector<T, InlineCapacity>& vec
) {
    os << "{";
    std::ranges::copy(vec, std::ostream_iterator<T>{os, ", "});
    os << "}";
    return os;
}
}

#ifndef NDEBUG
#define EXPECT_ASSERT(statement) EXPECT_DEATH(statement, "")
#else
#define EXPECT_ASSERT(statement) EXPECT_DEATH({}, "")
#endif
#define EXPECT_NO_ASSERT(statement) EXPECT_TRUE(true)

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

TEST(TestAssign, FillWithValueEmpty) {
    TrivialVector<int> vec;
    vec.assign(0, 0);
    EXPECT_TRUE(vec.empty());
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

TEST(TestAssign, RAIterEmpty) {
    std::vector<int> data;
    TrivialVector<int> vec;
    vec.assign(data.end(), data.end());
    EXPECT_TRUE(vec.empty());
}

TEST(TestAssign, RAIterRealloc) {
    int val = 0;
    auto sz = 5;
    std::vector<int> data(sz, val);
    TrivialVector<int> vec;
    vec.assign(data.begin(), data.end());
    EXPECT_TRUE(std::ranges::equal(vec, data));
}

TEST(TestAssign, RAIterNoReallocInline) {
    int val = 0;
    auto sz = 5;
    std::vector<int> data(sz, val);
    InlineTrivialVector<int, 5> vec;
    vec.assign(data.begin(), data.end());
    EXPECT_TRUE(std::ranges::equal(vec, data));
}

TEST(TestAssign, RAIterNoReallocHeap) {
    int val = 0;
    auto sz = 5;
    std::vector<int> data(sz, val);
    TrivialVector<int> vec(data.size());
    vec.assign(data.begin(), data.end());
    EXPECT_TRUE(std::ranges::equal(vec, data));
}

TEST(TestAssign, FwdIterEmpty) {
    std::list<int> data;
    TrivialVector<int> vec;
    vec.assign(data.end(), data.end());
    EXPECT_TRUE(vec.empty());
}

TEST(TestAssign, FwdIterRealloc) {
    int val = 0;
    auto sz = 5;
    std::list<int> data(sz, val);
    TrivialVector<int> vec;
    vec.assign(data.begin(), data.end());
    EXPECT_TRUE(std::ranges::equal(vec, data));
}

TEST(TestAssign, FwdIterNoReallocInline) {
    int val = 0;
    auto sz = 5;
    std::list<int> data(sz, val);
    InlineTrivialVector<int, 5> vec;
    vec.assign(data.begin(), data.end());
    EXPECT_TRUE(std::ranges::equal(vec, data));
}

TEST(TestAssign, FwdIterNoReallocHeap) {
    int val = 0;
    auto sz = 5;
    std::list<int> data(sz, val);
    TrivialVector<int> vec(data.size());
    vec.assign(data.begin(), data.end());
    EXPECT_TRUE(std::ranges::equal(vec, data));
}

TEST(TestAssign, EmptySizedRange) {
    std::vector<int> data;
    TrivialVector<int> vec;
    vec.assign(data);
    EXPECT_TRUE(vec.empty());
}

TEST(TestAssign, SizedRangeRealloc) {
    int val = 0;
    auto sz = 5;
    std::vector<int> data(sz, val);
    TrivialVector<int> vec;
    vec.assign(data);
    EXPECT_TRUE(std::ranges::equal(vec, data));
}

TEST(TestAssign, SizedRangeNoReallocInline) {
    int val = 0;
    auto sz = 5;
    std::vector<int> data(sz, val);
    InlineTrivialVector<int, 5> vec;
    vec.assign(data);
    EXPECT_TRUE(std::ranges::equal(vec, data));
}

TEST(TestAssign, SizedRangeNoReallocHeap) {
    int val = 0;
    auto sz = 5;
    std::vector<int> data(sz, val);
    TrivialVector<int> vec(data.size());
    vec.assign(data);
    EXPECT_TRUE(std::ranges::equal(vec, data));
}

TEST(TestAssign, EmptyUnsizedRange) {
    std::list<int> data;
    TrivialVector<int> vec;
    vec.assign(std::ranges::subrange(data));
    EXPECT_TRUE(vec.empty());
}

TEST(TestAssign, UnsizedRangeRealloc) {
    int val = 0;
    auto sz = 5;
    std::list<int> data(sz, val);
    TrivialVector<int> vec;
    vec.assign(std::ranges::subrange(data));
    EXPECT_TRUE(std::ranges::equal(vec, data));
}

TEST(TestAssign, UnsizedRangeNoReallocInline) {
    int val = 0;
    auto sz = 5;
    std::list<int> data(sz, val);
    InlineTrivialVector<int, 5> vec;
    vec.assign(std::ranges::subrange(data));
    EXPECT_TRUE(std::ranges::equal(vec, data));
}

TEST(TestAssign, UnsizedRangeNoReallocHeap) {
    int val = 0;
    auto sz = 5;
    std::list<int> data(sz, val);
    TrivialVector<int> vec(data.size());
    vec.assign(std::ranges::subrange(data));
    EXPECT_TRUE(std::ranges::equal(vec, data));
}

TEST(TestAssign, EmptyInitList) {
    TrivialVector<int> vec;
    vec.assign({});
    EXPECT_TRUE(vec.empty());
}

TEST(TestAssign, InitListRealloc) {
    std::array data = {1, 2, 3};
    TrivialVector<int> vec;
    vec.assign({1, 2, 3});
    EXPECT_TRUE(std::ranges::equal(vec, data));
}

TEST(TestAssign, InitListNoReallocInline) {
    std::array data = {1, 2, 3};
    InlineTrivialVector<int, 3> vec;
    vec.assign({1, 2, 3});
    EXPECT_TRUE(std::ranges::equal(vec, data));
}

TEST(TestAssign, InitListNoReallocHeap) {
    std::array data = {1, 2, 3};
    TrivialVector<int> vec(data.size());
    vec.assign({1, 2, 3});
    EXPECT_TRUE(std::ranges::equal(vec, data));
}

TEST(TestAt, InRange) {
    TrivialVector<int> vec = {1, 2, 3};
    EXPECT_NO_THROW(vec.at(0));
    EXPECT_EQ(vec.at(0), 1);
    vec.at(0) = 5;
    EXPECT_EQ(vec.at(0), 5);
}

TEST(TestAt, OutOfRange) {
    TrivialVector<int> vec;
    EXPECT_THROW(vec.at(0), std::out_of_range);
}

TEST(TestAccess, InRange) {
    TrivialVector<int> vec = {1, 2, 3};
    EXPECT_NO_ASSERT(vec[0]);
    EXPECT_EQ(vec[0], 1);
    vec[0] = 5;
    EXPECT_EQ(vec[0], 5);
}

TEST(TestAccess, OutOfRange) {
    TrivialVector<int> vec;
    EXPECT_ASSERT(vec[0]);
}

TEST(TestFront, InRange) {
    TrivialVector<int> vec = {1, 2, 3};
    EXPECT_NO_ASSERT(vec.front());
    EXPECT_EQ(vec.front(), 1);
    vec.front() = 5;
    EXPECT_EQ(vec.front(), 5);
}

TEST(TestFront, OutOfRange) {
    TrivialVector<int> vec;
    EXPECT_ASSERT(vec.front());
}

TEST(TestBack, InRange) {
    TrivialVector<int> vec = {1, 2, 3};
    EXPECT_NO_ASSERT(vec.back());
    EXPECT_EQ(vec.back(), 3);
    vec.back() = 5;
    EXPECT_EQ(vec.back(), 5);
}

TEST(TestBack, OutOfRange) {
    TrivialVector<int> vec;
    EXPECT_ASSERT(vec.back());
}

TEST(TestData, CData) {
    std::array arr = {1, 2, 3};
    TrivialVector<int> vec(arr);
    for (size_t i = 0; i < arr.size(); i++) {
        EXPECT_EQ(vec.cdata()[i], arr[i]);
    }
}

TEST(TestData, Data) {
    std::array arr = {1, 2, 3};
    TrivialVector<int> vec(arr);
    for (size_t i = 0; i < arr.size(); i++) {
        EXPECT_EQ(vec.data()[i], arr[i]);
    }
    std::array arr2 = {3, 1, 2};
    for (size_t i = 0; i < arr2.size(); i++) {
        vec.data()[i] = arr2[i];
    }
    EXPECT_TRUE(std::ranges::equal(vec, arr2));
}

TEST(TestIterators, CBegin) {
    std::array arr = {1, 2, 3};
    TrivialVector<int> vec(arr);
    EXPECT_EQ(*vec.cbegin(), vec[0]);
}

TEST(TestIterators, CEnd) {
    std::array arr = {1, 2, 3};
    TrivialVector<int> vec(arr);
    EXPECT_EQ(&*vec.cend(), vec.data() + vec.size());
}

TEST(TestIterators, Begin) {
    std::array arr = {1, 2, 3};
    TrivialVector<int> vec(arr);
    EXPECT_EQ(*vec.begin(), vec[0]);
    *vec.begin() = 5;
    EXPECT_EQ(vec[0], 5);
    EXPECT_EQ(*vec.begin(), vec[0]);
}

TEST(TestIterators, End) {
    std::array arr = {1, 2, 3};
    TrivialVector<int> vec(arr);
    EXPECT_EQ(&*vec.end(), vec.data() + vec.size());
}

TEST(TestIterators, ChangeAll) {
    std::array arr = {1, 2, 3};
    TrivialVector<int> vec(arr);
    EXPECT_TRUE(std::ranges::equal(
        vec.begin(), vec.end(),
        arr.begin(), arr.end()));
    std::array arr2 = {2, 3, 1};
    std::ranges::copy(arr2, vec.begin());
    EXPECT_TRUE(std::ranges::equal(
        vec.begin(), vec.end(),
        arr2.begin(), arr2.end()));
}

TEST(TestIterators, CRBegin) {
    std::array arr = {1, 2, 3};
    TrivialVector<int> vec(arr);
    EXPECT_EQ(*vec.crbegin(), vec[2]);
}

TEST(TestIterators, CREnd) {
    std::array arr = {1, 2, 3};
    TrivialVector<int> vec(arr);
    EXPECT_EQ(&*vec.crend(), vec.data() - 1);
}

TEST(TestIterators, RBegin) {
    std::array arr = {1, 2, 3};
    TrivialVector<int> vec(arr);
    EXPECT_EQ(*vec.rbegin(), vec[2]);
    *vec.rbegin() = 5;
    EXPECT_EQ(vec[2], 5);
    EXPECT_EQ(*vec.rbegin(), vec[2]);
}

TEST(TestIterators, REnd) {
    std::array arr = {1, 2, 3};
    TrivialVector<int> vec(arr);
    EXPECT_EQ(&*vec.rend(), vec.data() - 1);
}

TEST(TestIterators, ChangeAllReverse) {
    std::array arr = {1, 2, 3};
    TrivialVector<int> vec(arr);
    EXPECT_TRUE(std::ranges::equal(
        vec.rbegin(), vec.rend(),
        arr.rbegin(), arr.rend()));
    std::array arr2 = {2, 3, 1};
    std::ranges::copy(arr2, vec.rbegin());
    EXPECT_TRUE(std::ranges::equal(
        vec.begin(), vec.end(),
        arr2.rbegin(), arr2.rend()));
}

TEST(TestEmpty, Empty) {
    TrivialVector<int> vec;
    EXPECT_TRUE(vec.empty());
}

TEST(TestEmpty, NotEmpty) {
    TrivialVector<int> vec(1);
    EXPECT_FALSE(vec.empty());
}

TEST(TestSize, Empty) {
    TrivialVector<int> vec;
    EXPECT_EQ(vec.size(), 0);
}

TEST(TestSize, NotEmpty) {
    auto sz = 2;
    TrivialVector<int> vec(sz);
    EXPECT_EQ(vec.size(), sz);
    EXPECT_EQ(vec.size(), std::ranges::distance(vec.begin(), vec.end()));
}

TEST(TestCapacity, Empty) {
    TrivialVector<int> vec;
    EXPECT_EQ(vec.capacity(), 0);
}

TEST(TestCapacity, Inline) {
    InlineTrivialVector<int, 5> vec;
    EXPECT_EQ(vec.capacity(), vec.max_inline_size());
}

TEST(TestCapacity, NotEmpty) {
    TrivialVector<int> vec(5);
    EXPECT_GE(vec.capacity(), vec.size());
}

TEST(TestShrinkToFit, Empty) {
    TrivialVector<int> vec;
    EXPECT_EQ(vec.capacity(), 0);
    vec.shrink_to_fit();
    EXPECT_EQ(vec.capacity(), 0);
}

TEST(TestShrinkToFit, EmptyInline) {
    InlineTrivialVector<int, 5> vec;
    EXPECT_EQ(vec.capacity(), vec.max_inline_size());
    vec.shrink_to_fit();
    EXPECT_EQ(vec.capacity(), vec.max_inline_size());
}

TEST(TestShrinkToFit, NonEmpty) {
    TrivialVector<int> vec(5);
    auto old_capacity = vec.capacity();
    vec.shrink_to_fit();
    EXPECT_LE(vec.capacity(), old_capacity);
}

TEST(TestShrinkToFit, NonEmptyInline) {
    InlineTrivialVector<int, 5> vec(4);
    EXPECT_EQ(vec.capacity(), vec.max_inline_size());
    vec.shrink_to_fit();
    EXPECT_EQ(vec.capacity(), vec.max_inline_size());
}

TEST(TestShrinkToFit, NonEmptyHeap) {
    InlineTrivialVector<int, 3> vec(5);
    EXPECT_GT(vec.capacity(), vec.max_inline_size());
    vec.assign(vec.max_inline_size() - 1, 0);
    vec.shrink_to_fit();
    EXPECT_EQ(vec.capacity(), vec.max_inline_size());
}

TEST(TestClear, Empty) {
    TrivialVector<int> vec;
    vec.clear();
    EXPECT_TRUE(vec.empty());
}

TEST(TestClear, NonEmpty) {
    TrivialVector<int> vec(5);
    vec.clear();
    EXPECT_TRUE(vec.empty());
}

TEST(TestEmplace, ValueEmpty) {
    TrivialVector<int> vec;
    int val = 5;
    auto it = vec.emplace(vec.end(), val);
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, std::array{val}))
        << "Vec is " << vec;
}

TEST(TestEmplace, ValueFrontWithRealloc) {
    InlineTrivialVector<int, 4> vec = {1, 2, 3, 4};
    int val = 5;
    auto it = vec.emplace(vec.begin(), val);
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, std::array{val, 1, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestEmplace, ValueMidWithRealloc) {
    InlineTrivialVector<int, 4> vec = {1, 2, 3, 4};
    auto iit = std::ranges::find(vec, 2);
    auto idx = std::ranges::distance(vec.begin(), iit);
    int val = 5;
    auto it = vec.emplace(iit, val);
    EXPECT_EQ(it, vec.begin() + idx);
    EXPECT_TRUE(std::ranges::equal(vec, std::array{1, val, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestEmplace, ValueEndWithRealloc) {
    InlineTrivialVector<int, 4> vec = {1, 2, 3, 4};
    int val = 5;
    auto it = vec.emplace(vec.end(), val);
    EXPECT_EQ(it, vec.end() - 1);
    EXPECT_TRUE(std::ranges::equal(vec, std::array{1, 2, 3, 4, val}))
        << "Vec is " << vec;
}

TEST(TestInsert, ValueEmpty) {
    TrivialVector<int> vec;
    int val = 5;
    auto it = vec.insert(vec.end(), val);
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, std::array{val}))
        << "Vec is " << vec;
}

TEST(TestInsert, ValueFrontWithRealloc) {
    InlineTrivialVector<int, 4> vec = {1, 2, 3, 4};
    int val = 5;
    auto it = vec.insert(vec.begin(), val);
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, std::array{val, 1, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestInsert, ValueMidWithRealloc) {
    InlineTrivialVector<int, 4> vec = {1, 2, 3, 4};
    auto iit = std::ranges::find(vec, 2);
    auto idx = std::ranges::distance(vec.begin(), iit);
    int val = 5;
    auto it = vec.insert(iit, val);
    EXPECT_EQ(it, vec.begin() + idx);
    EXPECT_TRUE(std::ranges::equal(vec, std::array{1, val, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestInsert, ValueEndWithRealloc) {
    InlineTrivialVector<int, 4> vec = {1, 2, 3, 4};
    int val = 5;
    auto it = vec.insert(vec.end(), val);
    EXPECT_EQ(it, vec.end() - 1);
    EXPECT_TRUE(std::ranges::equal(vec, std::array{1, 2, 3, 4, val}))
        << "Vec is " << vec;
}

TEST(TestInsert, SpaceEmpty) {
    TrivialVector<int> vec;
    auto cnt = 5;
    auto it = vec.insert_space(vec.end(), cnt);
    EXPECT_EQ(it, vec.begin());
    std::ranges::fill_n(it, cnt, 0);
    EXPECT_TRUE(std::ranges::equal(vec, std::array{0, 0, 0, 0, 0}))
        << "Vec is " << vec;
}

TEST(TestInsert, NoSpaceEmpty) {
    TrivialVector<int> vec;
    auto it = vec.insert_space(vec.end(), 0);
    EXPECT_EQ(it, vec.end());
    EXPECT_TRUE(vec.empty())
        << "Vec is " << vec;
}

TEST(TestInsert, SpaceFrontWithRealloc) {
    InlineTrivialVector<int, 4> vec = {1, 2, 3, 4};
    auto cnt = 2;
    auto it = vec.insert_space(vec.begin(), cnt);
    EXPECT_EQ(it, vec.begin());
    std::ranges::fill_n(it, cnt, 0);
    EXPECT_TRUE(std::ranges::equal(vec, std::array{0, 0, 1, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestInsert, SpaceMidWithRealloc) {
    InlineTrivialVector<int, 4> vec = {1, 2, 3, 4};
    auto iit = std::ranges::find(vec, 2);
    auto idx = std::ranges::distance(vec.begin(), iit);
    auto cnt = 2;
    auto it = vec.insert_space(iit, cnt);
    EXPECT_EQ(it, vec.begin() + idx);
    std::ranges::fill_n(it, cnt, 0);
    EXPECT_TRUE(std::ranges::equal(vec, std::array{1, 0, 0, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestInsert, SpaceEndWithRealloc) {
    InlineTrivialVector<int, 4> vec = {1, 2, 3, 4};
    auto cnt = 2;
    auto it = vec.insert_space(vec.end(), cnt);
    EXPECT_EQ(it, vec.end() - cnt);
    std::ranges::fill_n(it, cnt, 0);
    EXPECT_TRUE(std::ranges::equal(vec, std::array{1, 2, 3, 4, 0, 0}))
        << "Vec is " << vec;
}

TEST(TestInsert, ValuesEmpty) {
    TrivialVector<int> vec;
    auto cnt = 5;
    auto val = 5;
    auto it = vec.insert(vec.end(), cnt, val);
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{val, val, val, val, val}))
        << "Vec is " << vec;
}

TEST(TestInsert, ZeroValuesEmpty) {
    TrivialVector<int> vec;
    auto val = 5;
    auto it = vec.insert(vec.end(), 0, val);
    EXPECT_EQ(it, vec.end());
    EXPECT_TRUE(vec.empty())
        << "Vec is " << vec;
}

TEST(TestInsert, ValuesFrontWithRealloc) {
    InlineTrivialVector<int, 4> vec = {1, 2, 3, 4};
    auto val = 5;
    auto cnt = 2;
    auto it = vec.insert(vec.begin(), cnt, val);
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, std::array{val, val, 1, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestInsert, ValuesMidWithRealloc) {
    InlineTrivialVector<int, 4> vec = {1, 2, 3, 4};
    auto iit = std::ranges::find(vec, 2);
    auto idx = std::ranges::distance(vec.begin(), iit);
    auto val = 5;
    auto cnt = 2;
    auto it = vec.insert(iit, cnt, 5);
    EXPECT_EQ(it, vec.begin() + idx);
    EXPECT_TRUE(std::ranges::equal(vec, std::array{1, val, val, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestInsert, ValuesEndWithRealloc) {
    InlineTrivialVector<int, 4> vec = {1, 2, 3, 4};
    auto val = 5;
    auto cnt = 2;
    auto it = vec.insert(vec.end(), cnt, val);
    EXPECT_EQ(it, vec.end() - cnt);
    EXPECT_TRUE(std::ranges::equal(vec, std::array{1, 2, 3, 4, val, val}))
        << "Vec is " << vec;
}

TEST(TestInsert, FwdIterEmpty) {
    std::list lst = {1, 2, 3, 4};
    TrivialVector<int> vec;
    auto it = vec.insert(vec.end(), lst.begin(), lst.end());
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, lst))
        << "Vec is " << vec;
}

TEST(TestInsert, FwdIterEmptyRange) {
    std::list<int> lst;
    TrivialVector<int> vec;
    auto it = vec.insert(vec.end(), lst.begin(), lst.end());
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(vec.empty());
}

TEST(TestInsert, FwdIterFront) {
    std::list lst = {4, 3, 2, 1};
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto it = vec.insert(vec.begin(), lst.begin(), lst.end());
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{4, 3, 2, 1, 1, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestInsert, FwdIterFrontEmptyRange) {
    std::list<int> lst;
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto it = vec.insert(vec.begin(), lst.begin(), lst.end());
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestInsert, FwdIterMid) {
    std::list lst = {1, 2, 3, 4};
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto iit = std::ranges::find(vec, 2);
    auto idx = std::ranges::distance(vec.begin(), iit);
    auto it = vec.insert(iit, lst.begin(), lst.end());
    EXPECT_EQ(it, vec.begin() + idx);
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{1, 1, 2, 3, 4, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestInsert, FwdIterMidEmptyRange) {
    std::list<int> lst;
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto iit = std::ranges::find(vec, 2);
    auto idx = std::ranges::distance(vec.begin(), iit);
    auto it = vec.insert(iit, lst.begin(), lst.end());
    EXPECT_EQ(it, vec.begin() + idx);
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestInsert, FwdIterBack) {
    std::list lst = {4, 3, 2, 1};
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto it = vec.insert(vec.end(), lst.begin(), lst.end());
    EXPECT_EQ(it, vec.end() - lst.size());
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{1, 2, 3, 4, 4, 3, 2, 1}))
        << "Vec is " << vec;
}

TEST(TestInsert, FwdIterBackEmptyRange) {
    std::list<int> lst;
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto it = vec.insert(vec.end(), lst.begin(), lst.end());
    EXPECT_EQ(it, vec.end());
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestInsert, RAIterEmpty) {
    std::vector data = {1, 2, 3, 4};
    TrivialVector<int> vec;
    auto it = vec.insert(vec.end(), data.begin(), data.end());
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, data))
        << "Vec is " << vec;
}

TEST(TestInsert, RAIterEmptyRange) {
    std::vector<int> data;
    TrivialVector<int> vec;
    auto it = vec.insert(vec.end(), data.begin(), data.end());
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(vec.empty());
}

TEST(TestInsert, RAIterFront) {
    std::vector data = {4, 3, 2, 1};
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto it = vec.insert(vec.begin(), data.begin(), data.end());
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{4, 3, 2, 1, 1, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestInsert, RAIterFrontEmptyRange) {
    std::vector<int> data;
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto it = vec.insert(vec.begin(), data.begin(), data.end());
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestInsert, RAIterMid) {
    std::vector data = {1, 2, 3, 4};
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto iit = std::ranges::find(vec, 2);
    auto idx = std::ranges::distance(vec.begin(), iit);
    auto it = vec.insert(iit, data.begin(), data.end());
    EXPECT_EQ(it, vec.begin() + idx);
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{1, 1, 2, 3, 4, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestInsert, RAIterMidEmptyRange) {
    std::vector<int> data;
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto iit = std::ranges::find(vec, 2);
    auto idx = std::ranges::distance(vec.begin(), iit);
    auto it = vec.insert(iit, data.begin(), data.end());
    EXPECT_EQ(it, vec.begin() + idx);
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestInsert, RAIterBack) {
    std::vector data = {4, 3, 2, 1};
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto it = vec.insert(vec.end(), data.begin(), data.end());
    EXPECT_EQ(it, vec.end() - data.size());
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{1, 2, 3, 4, 4, 3, 2, 1}))
        << "Vec is " << vec;
}

TEST(TestInsert, RAIterBackEmptyRange) {
    std::vector<int> data;
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto it = vec.insert(vec.end(), data.begin(), data.end());
    EXPECT_EQ(it, vec.end());
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestInsert, UnsizedRangeEmpty) {
    std::list lst = {1, 2, 3, 4};
    TrivialVector<int> vec;
    auto it = vec.insert(vec.end(), std::ranges::subrange(lst));
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, lst))
        << "Vec is " << vec;
}

TEST(TestInsert, UnsizedRangeEmptyRange) {
    std::list<int> lst;
    TrivialVector<int> vec;
    auto it = vec.insert(vec.end(), std::ranges::subrange(lst));
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(vec.empty());
}

TEST(TestInsert, UnsizedRangeFront) {
    std::list lst = {4, 3, 2, 1};
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto it = vec.insert(vec.begin(), std::ranges::subrange(lst));
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{4, 3, 2, 1, 1, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestInsert, UnsizedRangeFrontEmptyRange) {
    std::list<int> lst;
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto it = vec.insert(vec.begin(), std::ranges::subrange(lst));
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestInsert, UnsizedRangeMid) {
    std::list lst = {1, 2, 3, 4};
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto iit = std::ranges::find(vec, 2);
    auto idx = std::ranges::distance(vec.begin(), iit);
    auto it = vec.insert(iit, std::ranges::subrange(lst));
    EXPECT_EQ(it, vec.begin() + idx);
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{1, 1, 2, 3, 4, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestInsert, UnsizedRangeMidEmptyRange) {
    std::list<int> lst;
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto iit = std::ranges::find(vec, 2);
    auto idx = std::ranges::distance(vec.begin(), iit);
    auto it = vec.insert(iit, std::ranges::subrange(lst));
    EXPECT_EQ(it, vec.begin() + idx);
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestInsert, UnsizedRangeBack) {
    std::list lst = {4, 3, 2, 1};
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto it = vec.insert(vec.end(), std::ranges::subrange(lst));
    EXPECT_EQ(it, vec.end() - lst.size());
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{1, 2, 3, 4, 4, 3, 2, 1}))
        << "Vec is " << vec;
}

TEST(TestInsert, UnsizedRangeBackEmptyRange) {
    std::list<int> lst;
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto it = vec.insert(vec.end(), std::ranges::subrange(lst));
    EXPECT_EQ(it, vec.end());
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestInsert, SizedRangeEmpty) {
    std::vector data = {1, 2, 3, 4};
    TrivialVector<int> vec;
    auto it = vec.insert(vec.end(), data);
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, data))
        << "Vec is " << vec;
}

TEST(TestInsert, SizedRangeEmptyRange) {
    std::vector<int> data;
    TrivialVector<int> vec;
    auto it = vec.insert(vec.end(), data);
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(vec.empty());
}

TEST(TestInsert, SizedRangeFront) {
    std::vector data = {4, 3, 2, 1};
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto it = vec.insert(vec.begin(), data);
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{4, 3, 2, 1, 1, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestInsert, SizedRangeFrontEmptyRange) {
    std::vector<int> data;
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto it = vec.insert(vec.begin(), data);
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestInsert, SizedRangeMid) {
    std::vector data = {1, 2, 3, 4};
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto iit = std::ranges::find(vec, 2);
    auto idx = std::ranges::distance(vec.begin(), iit);
    auto it = vec.insert(iit, data);
    EXPECT_EQ(it, vec.begin() + idx);
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{1, 1, 2, 3, 4, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestInsert, SizedRangeMidEmptyRange) {
    std::vector<int> data;
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto iit = std::ranges::find(vec, 2);
    auto idx = std::ranges::distance(vec.begin(), iit);
    auto it = vec.insert(iit, data);
    EXPECT_EQ(it, vec.begin() + idx);
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestInsert, SizedRangeBack) {
    std::vector data = {4, 3, 2, 1};
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto it = vec.insert(vec.end(), data);
    EXPECT_EQ(it, vec.end() - data.size());
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{1, 2, 3, 4, 4, 3, 2, 1}))
        << "Vec is " << vec;
}

TEST(TestInsert, SizedRangeBackEmptyRange) {
    std::vector<int> data;
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto it = vec.insert(vec.end(), data);
    EXPECT_EQ(it, vec.end());
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestInsert, InitListEmpty) {
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec;
    auto it = vec.insert(vec.end(), {1, 2, 3, 4});
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestInsert, InitListEmptyRange) {
    TrivialVector<int> vec;
    auto it = vec.insert(vec.end(), {});
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(vec.empty());
}

TEST(TestInsert, InitListFront) {
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto it = vec.insert(vec.begin(), {4, 3, 2, 1});
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{4, 3, 2, 1, 1, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestInsert, InitListFrontEmptyRange) {
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto it = vec.insert(vec.begin(), {});
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestInsert, InitListMid) {
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto iit = std::ranges::find(vec, 2);
    auto idx = std::ranges::distance(vec.begin(), iit);
    auto it = vec.insert(iit, {1, 2, 3, 4});
    EXPECT_EQ(it, vec.begin() + idx);
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{1, 1, 2, 3, 4, 2, 3, 4}))
        << "Vec is " << vec;
}

TEST(TestInsert, InitListMidEmptyRange) {
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto iit = std::ranges::find(vec, 2);
    auto idx = std::ranges::distance(vec.begin(), iit);
    auto it = vec.insert(iit, {});
    EXPECT_EQ(it, vec.begin() + idx);
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestInsert, InitListBack) {
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto it = vec.insert(vec.end(), {4, 3, 2, 1});
    EXPECT_EQ(it, vec.end() - 4);
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{1, 2, 3, 4, 4, 3, 2, 1}))
        << "Vec is " << vec;
}

TEST(TestInsert, InitListBackEmptyRange) {
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto it = vec.insert(vec.end(), {});
    EXPECT_EQ(it, vec.end());
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestAppend, ValuesEmpty) {
    TrivialVector<int> vec;
    auto cnt = 5;
    auto val = 5;
    auto it = vec.append(cnt, val);
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{val, val, val, val, val}))
        << "Vec is " << vec;
}

TEST(TestAppend, ZeroValuesEmpty) {
    TrivialVector<int> vec;
    auto val = 5;
    auto it = vec.append(0, val);
    EXPECT_EQ(it, vec.end());
    EXPECT_TRUE(vec.empty())
        << "Vec is " << vec;
}

TEST(TestAppend, ValuesWithRealloc) {
    InlineTrivialVector<int, 4> vec = {1, 2, 3, 4};
    auto val = 5;
    auto cnt = 2;
    auto it = vec.append(cnt, val);
    EXPECT_EQ(it, vec.end() - cnt);
    EXPECT_TRUE(std::ranges::equal(vec, std::array{1, 2, 3, 4, val, val}))
        << "Vec is " << vec;
}

TEST(TestAppend, FwdIterEmpty) {
    std::list lst = {1, 2, 3, 4};
    TrivialVector<int> vec;
    auto it = vec.append(lst.begin(), lst.end());
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, lst))
        << "Vec is " << vec;
}

TEST(TestAppend, FwdIterEmptyEmptyRange) {
    std::list<int> lst;
    TrivialVector<int> vec;
    auto it = vec.append(lst.begin(), lst.end());
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(vec.empty());
}

TEST(TestAppend, FwdIter) {
    std::list lst = {4, 3, 2, 1};
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto it = vec.append(lst.begin(), lst.end());
    EXPECT_EQ(it, vec.end() - lst.size());
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{1, 2, 3, 4, 4, 3, 2, 1}))
        << "Vec is " << vec;
}

TEST(TestAppend, FwdIterEmptyRange) {
    std::list<int> lst;
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto it = vec.append(lst.begin(), lst.end());
    EXPECT_EQ(it, vec.end());
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestAppend, RAIterEmpty) {
    std::vector data = {1, 2, 3, 4};
    TrivialVector<int> vec;
    auto it = vec.append(data.begin(), data.end());
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, data))
        << "Vec is " << vec;
}

TEST(TestAppend, RAIterEmptyEmptyRange) {
    std::vector<int> data;
    TrivialVector<int> vec;
    auto it = vec.append(data.begin(), data.end());
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(vec.empty());
}

TEST(TestAppend, RAIter) {
    std::vector data = {4, 3, 2, 1};
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto it = vec.append(data.begin(), data.end());
    EXPECT_EQ(it, vec.end() - data.size());
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{1, 2, 3, 4, 4, 3, 2, 1}))
        << "Vec is " << vec;
}

TEST(TestAppend, RAIterEmptyRange) {
    std::vector<int> data;
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto it = vec.append(data.begin(), data.end());
    EXPECT_EQ(it, vec.end());
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestAppend, UnsizedRangeEmpty) {
    std::list lst = {1, 2, 3, 4};
    TrivialVector<int> vec;
    auto it = vec.append(std::ranges::subrange(lst));
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, lst))
        << "Vec is " << vec;
}

TEST(TestAppend, EmptyUnsizedRangeEmpty) {
    std::list<int> lst;
    TrivialVector<int> vec;
    auto it = vec.append(std::ranges::subrange(lst));
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(vec.empty());
}

TEST(TestAppend, UnsizedRange) {
    std::list lst = {4, 3, 2, 1};
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto it = vec.append(std::ranges::subrange(lst));
    EXPECT_EQ(it, vec.end() - lst.size());
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{1, 2, 3, 4, 4, 3, 2, 1}))
        << "Vec is " << vec;
}

TEST(TestAppend, EmptyUnsizedRange) {
    std::list<int> lst;
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto it = vec.append(std::ranges::subrange(lst));
    EXPECT_EQ(it, vec.end());
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestAppend, SizedRangeEmpty) {
    std::vector data = {1, 2, 3, 4};
    TrivialVector<int> vec;
    auto it = vec.append(data);
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, data))
        << "Vec is " << vec;
}

TEST(TestAppend, EmptySizedRangeEmpty) {
    std::vector<int> data;
    TrivialVector<int> vec;
    auto it = vec.append(data);
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(vec.empty());
}

TEST(TestAppend, SizedRange) {
    std::vector data = {4, 3, 2, 1};
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto it = vec.append(data);
    EXPECT_EQ(it, vec.end() - data.size());
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{1, 2, 3, 4, 4, 3, 2, 1}))
        << "Vec is " << vec;
}

TEST(TestAppend, EmptySizedRange) {
    std::vector<int> data;
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto it = vec.append(data);
    EXPECT_EQ(it, vec.end());
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestAppend, InitListEmpty) {
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec;
    auto it = vec.append({1, 2, 3, 4});
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestAppend, EmptyInitListEmpty) {
    TrivialVector<int> vec;
    auto it = vec.append({});
    EXPECT_EQ(it, vec.begin());
    EXPECT_TRUE(vec.empty());
}

TEST(TestAppend, InitList) {
    TrivialVector<int> vec = {1, 2, 3, 4};
    auto it = vec.append({4, 3, 2, 1});
    EXPECT_EQ(it, vec.end() - 4);
    EXPECT_TRUE(std::ranges::equal(
        vec, std::array{1, 2, 3, 4, 4, 3, 2, 1}))
        << "Vec is " << vec;
}

TEST(TestAppend, EmptyInitList) {
    std::array arr = {1, 2, 3, 4};
    TrivialVector<int> vec(arr);
    auto it = vec.append({});
    EXPECT_EQ(it, vec.end());
    EXPECT_TRUE(std::ranges::equal(vec, arr))
        << "Vec is " << vec;
}

TEST(TestEmplaceBack, Empty) {
    TrivialVector<int> vec;
    auto val = 5;
    vec.emplace_back() = val;
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], val);
}

TEST(TestEmplaceBack, NoRealloc) {
    InlineTrivialVector<int, 1> vec;
    auto val = 5;
    auto old_data = vec.data();
    vec.emplace_back() = val;
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], val);
    EXPECT_EQ(vec.data(), old_data);
}

TEST(TestEmplaceBack, Realloc) {
    InlineTrivialVector<int, 4> vec;
    vec.assign(vec.max_inline_size(), 0);
    auto val = 5;
    auto old_data = vec.data();
    auto old_capacity = vec.capacity();
    auto old_size = vec.size();
    vec.emplace_back() = val;
    EXPECT_EQ(vec.size(), old_size + 1);
    EXPECT_EQ(vec[old_size], val);
    EXPECT_NE(vec.data(), old_data);
}

TEST(TestPushBack, Empty) {
    TrivialVector<int> vec;
    auto val = 5;
    vec.push_back(val);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], val);
}

TEST(TestPushBack, NoRealloc) {
    InlineTrivialVector<int, 1> vec;
    auto val = 5;
    auto old_data = vec.data();
    vec.push_back(val);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], val);
    EXPECT_EQ(vec.data(), old_data);
}

TEST(TestPushBack, Realloc) {
    InlineTrivialVector<int, 4> vec;
    vec.assign(vec.max_inline_size(), 0);
    auto val = 5;
    auto old_data = vec.data();
    auto old_capacity = vec.capacity();
    auto old_size = vec.size();
    vec.push_back(val);
    EXPECT_EQ(vec.size(), old_size + 1);
    EXPECT_EQ(vec[old_size], val);
    EXPECT_NE(vec.data(), old_data);
}

TEST(TestShoveBack, Empty) {
    TrivialVector<int> vec;
    auto val = 5;
    EXPECT_ASSERT(
        vec.shove_back(val);
    );
}

TEST(TestShoveBack, NoRealloc) {
    InlineTrivialVector<int, 1> vec;
    auto val = 5;
    auto old_data = vec.data();
    vec.shove_back(val);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], val);
    EXPECT_EQ(vec.data(), old_data);
}

TEST(TestShoveBack, Realloc) {
    InlineTrivialVector<int, 4> vec;
    vec.assign(vec.max_inline_size(), 0);
    auto val = 5;
    EXPECT_ASSERT(
        vec.shove_back(val);
    );
}
