#pragma once
#include <algorithm>
#include <limits>
#include <memory>

namespace Attractadore::TrivialVectorNameSpace {
template<typename T>
concept IsComplete = requires(T t) {
    sizeof(t);
};

template<typename P>
class VectorIterator: std::contiguous_iterator_tag {
    using Element = std::pointer_traits<P>::element_type;
    using Ptr = P;
    using ConstPtr = typename std::pointer_traits<Ptr>::rebind<const Element>;
};

#define SVO_TRIVIAL_VECTOR_TEMPLATE \
template<typename T, unsigned InlineCapacity, typename Allocator> \
    requires std::is_trivial_v<T>

#define SVO_TRIVIAL_VECTOR Attractadore::TrivialVectorNameSpace::SVOTrivialVector<T, InlineCapacity, Allocator>

inline constexpr unsigned DefaultInlineBufferSize = 64;

template<typename T>
inline unsigned DefaultInlineCapacity = DefaultInlineBufferSize / sizeof(T);

template<typename T, unsigned InlineCapacity = DefaultInlineCapacity<T>, typename Allocator = std::allocator<T>>
    requires std::is_trivial_v<T>
class SVOTrivialVector {
    using AllocTraits = std::allocator_traits<Allocator>;
    using Ptr = AllocTraits::pointer;
    using ConstPtr = AllocTraits::const_pointer;
    using Size = unsigned;

    [[no_unique_address]]
    Allocator   m_allocator;
    Ptr         m_data;
    Size        m_size;
    Size        m_capacity;

public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = Ptr;
    using const_pointer = ConstPtr;
    using iterator = VectorIterator<pointer>;
    using const_iterator = VectorIterator<const_pointer>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr SVOTrivialVector() noexcept(noexcept(Allocator()));
    constexpr explicit SVOTrivialVector(Allocator alloc) noexcept;
    constexpr explicit SVOTrivialVector(size_type size, Allocator alloc = Allocator());
    constexpr SVOTrivialVector(size_type count, const T& value, Allocator alloc = Allocator());
    template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
        requires std::convertible_to<std::iter_value_t<Iter>, value_type>
    constexpr SVOTrivialVector(Iter first, Sent last, Allocator alloc = Allocator());
    template<std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, value_type>
    constexpr explicit SVOTrivialVector(R&& r, Allocator alloc = Allocator());
    // TODO: conversions?
    constexpr SVOTrivialVector(std::initializer_list<value_type> init, Allocator alloc = Allocator());
    constexpr SVOTrivialVector(const SVOTrivialVector& other);
    constexpr SVOTrivialVector(const SVOTrivialVector& other, Allocator alloc);
    constexpr SVOTrivialVector(SVOTrivialVector&& other) noexcept;
    constexpr SVOTrivialVector(SVOTrivialVector&& other, Allocator alloc);
    constexpr SVOTrivialVector& operator=(const SVOTrivialVector& other);
    constexpr SVOTrivialVector& operator=(SVOTrivialVector&& other) noexcept(
        AllocTraits::propagate_on_container_move_assignment::value or
        AllocTraits::is_always_equal::value
    );
    template<std::ranges::input_range R>
    constexpr SVOTrivialVector& operator=(R&& r);
    constexpr SVOTrivialVector& operator=(std::initializer_list<value_type> init);
    constexpr ~SVOTrivialVector();

    constexpr void assign(size_type count, const T& value);
    template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
        requires std::convertible_to<std::iter_value_t<Iter>, value_type>
    constexpr void assign(Iter first, Sent last);
    template<std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, value_type>
    constexpr void assign(R&& r);
    constexpr void assign(std::initializer_list<value_type> init);

    constexpr const allocator_type& get_allocator() const noexcept;

    constexpr reference at(size_type idx);
    constexpr const_reference at(size_type idx) const;
    constexpr reference operator[](size_type idx) noexcept;
    constexpr const_reference operator[](size_type idx) const noexcept;
    constexpr reference front() noexcept;
    constexpr const_reference front() const noexcept;
    constexpr reference back() noexcept;
    constexpr const_reference back() const noexcept;
    constexpr const value_type* cdata() const noexcept;
    constexpr const value_type* data() const noexcept;
    constexpr value_type* data() noexcept;

    constexpr const_iterator cbegin() const noexcept;
    constexpr const_iterator cend() const noexcept;
    constexpr const_iterator begin() const noexcept;
    constexpr const_iterator end() const noexcept;
    constexpr iterator begin() noexcept;
    constexpr iterator end() noexcept;

    constexpr const_iterator crbegin() const noexcept;
    constexpr const_iterator crend() const noexcept;
    constexpr const_iterator rbegin() const noexcept;
    constexpr const_iterator rend() const noexcept;
    constexpr iterator rbegin() noexcept;
    constexpr iterator rend() noexcept;

    constexpr bool empty() const noexcept;
    constexpr size_type size() const noexcept;
    static constexpr size_type max_size() noexcept;
    constexpr void reserve(size_type new_capacity);
    constexpr size_type capacity() const noexcept;
    constexpr void shrink_to_fit();
    constexpr void clear() noexcept;

    // TODO: converions?
    constexpr iterator insert(const_iterator pos, const value_type& value);
    constexpr iterator insert(const_iterator pos, size_type count, const value_type& value);
    template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
    constexpr iterator insert(const_iterator pos, Iter first, Sent last);
    template<std::ranges::input_range R>
    constexpr iterator insert(const_iterator pos, R&& r);
    constexpr iterator insert(const_iterator pos, std::initializer_list<value_type> init);
    constexpr iterator emplace(const_iterator pos);
    constexpr iterator emplace(const_iterator pos, const value_type& value);
    constexpr iterator erase(const_iterator pos) noexcept;
    constexpr iterator erase(const_iterator first, const_iterator last) noexcept;
    template<typename R> requires
        std::ranges::contiguous_range<R> and
        std::ranges::common_range<R> and
        std::convertible_to<std::ranges::iterator_t<R>, const_iterator>
    constexpr iterator erase(R&& r) noexcept;
    constexpr void push_back(const value_type& value);
    constexpr reference emplace_back();
    constexpr reference emplace_back(const value_type& value);
    constexpr void pop_back() noexcept;
    constexpr void resize(size_type new_size);
    constexpr void resize(size_type new_size, const value_type& value);
    template<typename R> requires
        std::ranges::contiguous_range<R> and
        std::ranges::common_range<R> and
        std::convertible_to<std::ranges::iterator_t<R>, const_iterator>
    constexpr void resize(R&& r);
    template<unsigned OtherInlineCapacity, typename OtherAllocator>
    constexpr void swap(SVOTrivialVector<T, OtherInlineCapacity, OtherAllocator>& other) noexcept(
        false // TODO
    );
};

template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
SVOTrivialVector(Iter first, Sent last) -> SVOTrivialVector<std::iter_value_t<Iter>>;

template<std::ranges::input_range R>
SVOTrivialVector(R&& r) -> SVOTrivialVector<std::ranges::range_value_t<R>>;

template<
    typename T1, unsigned InlineCapacity1, typename Allocator1,
    typename T2, unsigned InlineCapacity2, typename Allocator2
> constexpr bool operator==(
    const SVOTrivialVector<T1, InlineCapacity1, Allocator1>& l,
    const SVOTrivialVector<T2, InlineCapacity2, Allocator2>& r
) noexcept {
    return std::ranges::equal(l, r);
}

template<
    typename T1, unsigned InlineCapacity1, typename Allocator1,
    typename T2, unsigned InlineCapacity2, typename Allocator2
> constexpr auto operator<=>(
    const SVOTrivialVector<T1, InlineCapacity1, Allocator1>& l,
    const SVOTrivialVector<T2, InlineCapacity2, Allocator2>& r
) noexcept {
    return std::lexicographical_compare_three_way(
        l.begin(), l.end(),
        r.begin(), r.end());
}

template<
    typename T1, unsigned InlineCapacity1, typename Allocator1,
    typename T2, unsigned InlineCapacity2, typename Allocator2
> void swap(
    SVOTrivialVector<T1, InlineCapacity1, Allocator1>& l,
    SVOTrivialVector<T2, InlineCapacity2, Allocator2>& r
) noexcept (
    false //TODO
) {
    l.swap(r);
}
}

namespace std {
template<typename T, unsigned InlineCapacity, typename Allocator, typename U>
constexpr SVO_TRIVIAL_VECTOR::size_type erase(SVO_TRIVIAL_VECTOR& vec, const U& value) {
    auto old_size = vec.size();
    auto new_size = std::ranges::remove(vec, value).size();
    vec.truncate(new_size);
    return old_size - new_size;
}

template<typename T, unsigned InlineCapacity, typename Allocator, typename Pred>
constexpr SVO_TRIVIAL_VECTOR::size_type erase_if(SVO_TRIVIAL_VECTOR& vec, Pred pred) {
    auto old_size = vec.size();
    auto new_size = std::ranges::remove_if(vec, std::move(pred)).size();
    vec.truncate(new_size);
    return old_size - new_size;
}
}
