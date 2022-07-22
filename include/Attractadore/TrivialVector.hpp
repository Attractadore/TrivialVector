#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

namespace Attractadore::TrivialVectorNameSpace {
template<std::input_iterator I, std::sentinel_for<I> SI, std::weakly_incrementable O, std::sentinel_for<O> SO>
    requires std::indirectly_copyable<I, O>
constexpr std::ranges::copy_result<I, O> copy_some(I first, SI last, O result_first, SO result_last) {
    for (; result_first != result_last and first != last ; ++result_first, ++first) {
        *result_first = *first;
    }
    return {.in = first, .out = result_first};
}

template<typename P>
class VectorIterator: std::contiguous_iterator_tag {
    using Element = std::pointer_traits<P>::element_type;
    using Ptr = P;
    using ConstPtr = typename std::pointer_traits<Ptr>::rebind<const Element>;
};

template<typename T, typename Allocator>
concept TrivialVectorHeaderConcept =
    std::is_trivial_v<T> and
    std::same_as<typename Allocator::value_type, T>;

#define TRIVIAL_VECTOR_HEADER_TEMPLATE \
template<typename T, typename Allocator> \
    requires Attractadore::TrivialVectorNameSpace::TrivialVectorHeaderConcept<T, Allocator>
#define TRIVIAL_VECTOR_HEADER Attractadore::TrivialVectorNameSpace::TrivialVectorHeader<T, Allocator>

template<
    typename T,
    typename Allocator = std::allocator<T>
> requires TrivialVectorHeaderConcept<T, Allocator>
class TrivialVectorHeader: private Allocator {
protected:
    using AllocTraits = std::allocator_traits<Allocator>;
    using Ptr = AllocTraits::pointer;
    using PtrTraits = std::pointer_traits<Ptr>;
    using ConstPtr = AllocTraits::const_pointer;
    //using SizeT = unsigned;
    //using SizeT = int;
    using SizeT = size_t;
    //using SizeT = ssize_t;

    Ptr         m_data;
    SizeT       m_capacity;
    SizeT       m_size = 0;

public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = Ptr;
    using const_pointer = ConstPtr;
    // TODO: replace with real iterators
#if 0
    using iterator = VectorIterator<pointer>;
    using const_iterator = VectorIterator<const_pointer>;
#else
    using iterator = pointer;
    using const_iterator = const_pointer;
#endif
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

protected:
    constexpr TrivialVectorHeader(Allocator alloc, Ptr data_init, size_type capacity) noexcept:
        Allocator{std::move(alloc)}, m_data{data_init}, m_capacity(capacity) {
        assert(capacity <= max_size());
    }
    constexpr ~TrivialVectorHeader() = default;

public:
    constexpr TrivialVectorHeader& operator=(
        const TrivialVectorHeader& other
    ) {
        constexpr bool propagate =
            AllocTraits::propagate_on_container_copy_assignment::value;
        if constexpr (propagate) {
            if (not allocators_equal(other)) {
                bool must_realloc =
                    not data_is_inlined() or
                    capacity() < other.size();
                if (must_realloc) {
                    auto [new_data, new_capacity] =
                        other.allocate_for_size(other.size());
                    deallocate();
                    m_data = new_data;
                    m_capacity = new_capacity;
                }
                allocator() = other.get_allocator();
            }
        }
        assign(other);
        return *this;
    }

    constexpr TrivialVectorHeader& operator=(std::initializer_list<value_type> init) {
        assign(init);
        return *this;
    }

    constexpr void assign(size_type count, const value_type& value) {
        fit(count);
        std::ranges::fill(*this, value);
    }

    template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
        requires std::convertible_to<
            std::iter_value_t<Iter>, value_type>
    constexpr void assign(Iter first, Sent last) {
        if constexpr (std::sized_sentinel_for<Sent, Iter>) {
            auto new_size = std::ranges::distance(first, last);
            fit(new_size);
            std::ranges::copy(first, last, data());
        } else {
            clear();
            append(first, last);
        }
    }

    template<std::ranges::input_range R>
        requires std::convertible_to<
            std::ranges::range_value_t<R>, value_type>
    constexpr void assign(R&& r) {
        if constexpr (std::ranges::sized_range<R>) {
            auto new_size = std::ranges::size(r);
            fit(new_size);
            std::ranges::copy(std::forward<R>(r), data());
        } else {
            assign(std::ranges::begin(r), std::ranges::end(r));
        }
    }

    constexpr void assign(std::initializer_list<value_type> init) {
        assign(init.begin(), init.end());
    }

    constexpr const allocator_type& get_allocator() const noexcept { return *this; }

private:
    constexpr void range_check(size_type idx) const {
        if (idx >= size()) {
            throw std::out_of_range{
                "TrivialVector range check: index " +
                std::to_string(idx) +
                " >= size " +
                std::to_string(size())};
        }
    }

public:
    constexpr const_reference at(size_type idx) const {
        range_check(idx);
        return m_data[idx];
    }

    constexpr reference at(size_type idx) {
        range_check(idx);
        return m_data[idx];
    }

    constexpr const_reference operator[](size_type idx) const noexcept {
        assert(idx < size());
        return m_data[idx];
    }

    constexpr reference operator[](size_type idx) noexcept {
        assert(idx < size());
        return m_data[idx];
    }

    constexpr const_reference front() const noexcept {
        assert(not empty());
        return m_data[0];
    }

    constexpr reference front() noexcept {
        assert(not empty());
        return m_data[0];
    }

    constexpr const_reference back() const noexcept {
        assert(not empty());
        return m_data[size() - 1];
    }

    constexpr reference back() noexcept {
        assert(not empty());
        return m_data[size() - 1];
    }

    constexpr const value_type* cdata() const noexcept {
        return data();
    }

    constexpr const value_type* data() const noexcept {
        return std::to_address(m_data);
    }

    constexpr value_type* data() noexcept {
        return std::to_address(m_data);
    }

    constexpr const_iterator cbegin() const noexcept {
        return begin();
    }

    constexpr const_iterator cend() const noexcept {
        return end();
    }

    constexpr const_iterator begin() const noexcept {
        return const_iterator{m_data};
    }

    constexpr const_iterator end() const noexcept {
        return const_iterator{m_data + size()};
    }

    constexpr iterator begin() noexcept {
        return iterator{m_data};
    }

    constexpr iterator end() noexcept {
        return iterator{m_data + size()};
    }

    constexpr const_reverse_iterator crbegin() const noexcept {
        return rbegin();
    }

    constexpr const_reverse_iterator crend() const noexcept {
        return rend();
    }

    constexpr const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator{end()};
    }

    constexpr const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator{begin()};
    }

    constexpr reverse_iterator rbegin() noexcept {
        return reverse_iterator{end()};
    }

    constexpr reverse_iterator rend() noexcept {
        return reverse_iterator{begin()};
    }

    constexpr bool empty() const noexcept {
        return size() == 0;
    }

    constexpr size_type size() const noexcept {
        return m_size;
    }

    static constexpr size_type max_size() noexcept {
        return std::min<size_type>(
            std::numeric_limits<SizeT>::max(),
            std::numeric_limits<size_type>::max() / sizeof(value_type));
    }

    constexpr void reserve(size_type new_capacity) {
        if (new_capacity > capacity()) {
            reallocate(new_capacity);
        }
    }

    constexpr size_type capacity() const noexcept {
        return m_capacity;
    }

    constexpr void clear() noexcept {
        truncate(0);
    }

protected:
    template<typename F>
    constexpr iterator do_sized_insert(
        const_iterator pos, size_t count, F do_assign
    ) {
        assert(count);
        auto new_size = size() + count;
        [[likely]]
        if (new_size <= capacity()) {
            return do_sized_place(pos, count, std::move(do_assign));
        } else {
            return do_sized_realloc_insert(pos, count, std::move(do_assign));
        };
    }

    template<typename F>
    constexpr iterator do_sized_place(
        const_iterator pos, size_t count, F do_assign
    ) noexcept {
        assert(count);
        auto idx = std::ranges::distance(begin(), pos);
        auto new_size = size() + count;
        assert(new_size <= capacity());
        auto assign_begin = begin() + idx;
        auto old_end = end();
        m_size = new_size;
        std::ranges::copy_backward(assign_begin, old_end, end());
        do_assign(assign_begin);
        return assign_begin;
    }

    template<typename F>
    constexpr iterator do_sized_realloc_insert(
        const_iterator pos, size_t count, F do_assign
    ) {
        assert(count);
        auto new_size = size() + count;
        auto [new_data, new_capacity] = allocate_for_size(new_size);
        auto assign_begin = std::ranges::copy(begin(), pos, new_data).out;
        auto assign_end = do_assign(assign_begin);
        std::ranges::copy(pos, end(), assign_end);
        deallocate();
        m_data = new_data;
        m_capacity = new_capacity;
        m_size = new_size;
        return iterator{assign_begin};
    }

public:
    template<typename... Args>
        requires std::constructible_from<value_type, Args&&...>
    constexpr iterator emplace(
        const_iterator pos, Args&&... args 
    ) {
        return do_sized_insert(pos, 1,
            [&] (auto it) {
                *it = value_type(std::forward<Args>(args)...);
                return ++it;
            }
        );
    }

    constexpr iterator insert(
        const_iterator pos, const value_type& value
    ) {
        return emplace(pos, value);
    }

    constexpr iterator insert_space(
        const_iterator pos, size_type count
    ) {
        if (count) {
            return do_sized_insert(pos, count,
                [&] (auto it) { return it + count; }
            );
        } else {
            return begin() + std::ranges::distance(begin(), pos);
        }
    }

    constexpr iterator insert(
        const_iterator pos, size_type count, const value_type& value
    ) {
        if (count) {
            return do_sized_insert(pos, count,
                [&] (auto it) {
                    return std::ranges::fill_n(it, count, value);
                }
            );
        } else {
            return begin() + std::ranges::distance(begin(), pos);
        }
    }

    template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
        requires std::convertible_to<
            std::iter_value_t<Iter>, value_type>
    constexpr iterator insert(
        const_iterator pos, Iter first, Sent last
    ) {
        if constexpr (std::sized_sentinel_for<Sent, Iter>) {
            auto count = std::ranges::distance(first, last);
            if (count) {
                return do_sized_insert(pos, count,
                    [&] (auto it) {
                        return std::ranges::copy(first, last, it).out;
                    }
                );
            } else {
                return begin() + std::ranges::distance(begin(), pos);
            }
        } else {
            auto front_size = std::ranges::distance(begin(), pos);
            auto new_size = size();
            auto append_some = [&] {
                auto [new_first, new_end] = copy_some(
                    first, last,
                    data() + new_size, data() + capacity());
                first = new_first;
                new_size = std::ranges::distance(data(), new_end);
            };
            append_some();
            while (first != last) {
                auto [new_data, new_capacity] =
                    allocate_for_size(new_size + 1);
                std::ranges::copy_n(data(), new_size, new_data);
                deallocate();
                m_data = new_data;
                m_capacity = new_capacity;
                append_some();
            }
            std::ranges::rotate(
                data() + front_size,
                data() + size(),
                data() + new_size);
            m_size = new_size;
            return begin() + front_size;
        }
    }

    template<std::ranges::input_range R>
        requires std::convertible_to<
            std::ranges::range_value_t<R>, value_type>
    constexpr iterator insert(const_iterator pos, R&& r) {
        if constexpr (std::ranges::sized_range<R>) {
            auto count = std::ranges::size(r);
            if (count) {
                return do_sized_insert(pos, count,
                    [&] (auto it) {
                        return std::ranges::copy(
                            std::forward<R>(r), it).out;
                    }
                );
            } else {
                return begin() + std::ranges::distance(begin(), pos);
            }
        } else {
            return insert(std::ranges::begin(r), std::ranges::end(r));
        }
    }

    constexpr iterator insert(
        const_iterator pos, std::initializer_list<value_type> init
    ) {
        return insert(pos, init.begin(), init.end());
    }

    constexpr iterator append(size_type count, const value_type& value) {
        return insert(end(), count, value);
    }

    template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
        requires std::convertible_to<
            std::iter_value_t<Iter>, value_type>
    constexpr iterator append(Iter first, Sent last) {
        return insert(end(), first, last);
    }

    template<std::ranges::input_range R>
        requires std::convertible_to<
            std::ranges::range_value_t<R>, value_type>
    constexpr iterator append(R&& r) {
        return insert(end(), std::forward<R>(r));
    }

    constexpr iterator append(std::initializer_list<value_type> init) {
        return insert(end(), init);
    }

    template<typename... Args>
        requires std::constructible_from<value_type, Args&&...>
    constexpr reference emplace_back(Args&&... args) {
        [[likely]]
        if (size() < capacity()) {
            return data()[m_size++] =
                value_type(std::forward<Args>(args)...);
        } else {
            auto [new_data, new_capacity] = allocate_for_size(m_size + 1);
            std::ranges::copy_n(data(), size(), new_data);
            deallocate();
            m_data = new_data;
            m_capacity = new_capacity;
            return data()[m_size++] =
                value_type(std::forward<Args>(args)...);
        }
    }

    constexpr void push_back(const value_type& value) {
        emplace_back(value);
    }

    constexpr reference shove_back(const value_type& value) noexcept {
        assert(size() < capacity());
        return data()[m_size++] = value;
    }

    constexpr iterator erase(const_iterator pos) noexcept {
        assert(pos < end());
        auto idx = std::ranges::distance(begin(), pos);
        auto it = begin() + idx;
        std::ranges::copy(pos + 1, end(), it);
        m_size--;
        return it;
    }

    constexpr iterator erase(
        const_iterator first, const_iterator last
    ) noexcept {
        assert(first <= end() and first <= last and last <= end());
        auto idx = std::ranges::distance(begin(), first);
        auto it = begin() + idx;
        if (first != last) {
            auto new_end = std::ranges::copy(last, end(), it).out;
            m_size = std::ranges::distance(begin(), new_end);
        }
        return it;
    }

    template<std::ranges::input_range R> requires
        std::convertible_to<std::ranges::iterator_t<R>, const_iterator> and
        std::convertible_to<std::ranges::sentinel_t<R>, const_iterator>
    constexpr iterator erase(R&& r) noexcept {
        return erase(std::ranges::begin(r), std::ranges::end(r));
    }

    constexpr value_type pop_back() noexcept {
        assert(not empty());
        return data()[--m_size];
    }

    constexpr void resize(size_type new_size) {
        if (capacity() < new_size) {
            auto [new_data, new_capacity] =
                allocate_for_size(m_size + 1);
            std::ranges::copy(*this, new_data);
            m_data = new_data;
            m_capacity = new_capacity;
        }
        m_size = new_size;
    }

    constexpr void resize(
        size_type new_size, const value_type& value
    ) {
        auto old_size = size();
        resize(new_size);
        if (new_size > old_size) {
            std::ranges::fill(data() + old_size, data() + new_size, value);
        }
    }

    constexpr void resize(
        const_iterator first, const_iterator last
    ) noexcept {
        assert(begin() <= first and first <= last and last <= end());
        if (first != begin()) {
            std::ranges::copy(first, last, begin());
        }
        m_size = std::ranges::distance(first, last);
    }

    template<std::ranges::input_range R> requires
        std::convertible_to<std::ranges::iterator_t<R>, const_iterator> and
        std::convertible_to<std::ranges::sentinel_t<R>, const_iterator>
    constexpr void resize(R&& r) noexcept {
        resize(std::ranges::begin(r), std::ranges::end(r));
    }

    constexpr void truncate(size_type new_size) noexcept {
        assert(new_size <= size());
        m_size = new_size;
    }

    constexpr void fit(size_type new_size) {
        if (capacity() < new_size) {
            auto [new_data, new_capacity] =
                allocate_for_size(new_size);
            m_data = new_data;
            m_capacity = new_capacity;
        }
        m_size = new_size;
    }

protected:
    struct AllocateResult {
        pointer     ptr;
        size_type   n;
    };

    constexpr Allocator& allocator() noexcept {
        return *this;
    }

    constexpr bool allocators_equal(
        const TrivialVectorHeader& other
    ) const noexcept {
        return
            AllocTraits::is_always_equal::value or
            get_allocator() == other.get_allocator();
    }

    static constexpr AllocateResult allocate_for_size(
        Allocator& alloc, size_t new_size
    ) {
        try {
            auto new_capacity = std::bit_ceil(new_size);
            return { allocate(alloc, new_capacity), new_capacity };
        } catch (std::bad_alloc&) {
            return { allocate(alloc, new_size), new_size };
        }
    }

    constexpr AllocateResult allocate_for_size(size_t new_size) {
        return allocate_for_size(allocator(), new_size);
    }

    static constexpr pointer allocate(Allocator& alloc, size_t new_capacity) {
        return AllocTraits::allocate(alloc, new_capacity);
    }

    constexpr pointer allocate(size_t new_capacity) {
        return allocate(allocator(), new_capacity);
    }

    constexpr void reallocate(size_t new_capacity);

    constexpr void deallocate() noexcept {
        if (capacity() > 0 and not data_is_inlined()) {
            AllocTraits::deallocate(*this, data(), capacity());
        }
    }

    constexpr const T* inline_data() const noexcept;
    constexpr T* inline_data() noexcept;

    constexpr bool data_is_inlined() const noexcept {
        return data() == inline_data();
    };
};

template<typename T, unsigned InlineCapacity, typename Allocator>
concept InlineTrivialVectorConcept = TrivialVectorHeaderConcept<T, Allocator>;

#define INLINE_TRIVIAL_VECTOR_TEMPLATE \
template<typename T, unsigned InlineCapacity, typename Allocator> \
    requires Attractadore::TrivialVectorNameSpace::InlineTrivialVectorConcept<T, InlineCapacity, Allocator> 
#define INLINE_TRIVIAL_VECTOR Attractadore::TrivialVectorNameSpace::InlineTrivialVector<T, InlineCapacity, Allocator>

template<typename T, unsigned MinCapacity, size_t MinAlignSize>
struct InlineStorage {
    static constexpr unsigned AlignSize     = std::max(alignof(T), MinAlignSize);
    static constexpr unsigned MinBufferSize = sizeof(std::array<T, MinCapacity>);
    static constexpr unsigned BufferSize    =
        (MinBufferSize / AlignSize + (MinBufferSize % AlignSize != 0)) * AlignSize;
    static constexpr unsigned Capacity      = BufferSize / sizeof(T);

    std::array<T, Capacity> m_storage;

    constexpr const T* addr() const noexcept { return m_storage.data(); }
    constexpr T* addr() noexcept { return m_storage.data(); }
};

template<typename T, size_t MinAlignSize>
struct InlineStorage<T, 0, MinAlignSize> {
    static constexpr unsigned Capacity = 0;
    constexpr std::nullptr_t addr() const noexcept { return nullptr; }
};

inline constexpr unsigned DefaultInlineBufferSize = 64;

template<typename T>
inline unsigned DefaultInlineCapacity = DefaultInlineBufferSize / sizeof(T);

template<
    typename T,
    unsigned InlineCapacity = DefaultInlineCapacity<T>,
    typename Allocator = std::allocator<T>
> requires InlineTrivialVectorConcept<T, InlineCapacity, Allocator>
class InlineTrivialVector:
    public TrivialVectorHeader<T, Allocator>,
    private InlineStorage<T, InlineCapacity, alignof(TrivialVectorHeader<T, Allocator>)>
{
    template<typename OtherT,
    unsigned OtherInlineCapacity,
    typename OtherAllocator>
    friend class InlineTrivialVector;

    using Base = TrivialVectorHeader<T, Allocator>;
    friend Base;
    using Storage = InlineStorage<T, InlineCapacity, alignof(Base)>;
    using typename Base::AllocTraits;
    using Base::m_data;
    using Base::m_capacity;
    using Base::m_size;

    static constexpr struct ConstructorTag {} constructor_tag {};

public:
    using typename Base::value_type;
    using typename Base::allocator_type;
    using typename Base::size_type;
    using typename Base::difference_type;
    using typename Base::reference;
    using typename Base::const_reference;
    using typename Base::pointer;
    using typename Base::const_pointer;
    using typename Base::iterator;
    using typename Base::const_iterator;
    using typename Base::reverse_iterator;
    using typename Base::const_reverse_iterator;

    constexpr InlineTrivialVector() noexcept(noexcept(Allocator()))
        requires std::default_initializable<Allocator>:
        InlineTrivialVector(Allocator()) {}

    constexpr explicit InlineTrivialVector(Allocator alloc) noexcept:
        Base{std::move(alloc), inline_data(), max_inline_size()} {}

    constexpr explicit InlineTrivialVector(size_type size)
        requires std::default_initializable<Allocator>:
        InlineTrivialVector(size, Allocator()) {}

    constexpr InlineTrivialVector(size_type size, Allocator alloc):
        InlineTrivialVector(std::move(alloc))
    {
        this->fit(size);
    }

    constexpr InlineTrivialVector(size_type count, const value_type& value)
        requires std::default_initializable<Allocator>:
        InlineTrivialVector(count, value, Allocator()) {}

    constexpr InlineTrivialVector(size_type count, const value_type& value, Allocator alloc):
        InlineTrivialVector(count, std::move(alloc))
    {
        std::ranges::fill(*this, value);
    }

    template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
        requires std::convertible_to<std::iter_value_t<Iter>, value_type>
    constexpr InlineTrivialVector(Iter first, Sent last)
        requires std::default_initializable<Allocator>:
        InlineTrivialVector(first, last, Allocator()) {}

    template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
        requires std::convertible_to<std::iter_value_t<Iter>, value_type>
    constexpr InlineTrivialVector(Iter first, Sent last, Allocator alloc):
        InlineTrivialVector{std::move(alloc)}
    {
        this->assign(first, last);
    }

    template<std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, value_type>
    constexpr explicit InlineTrivialVector(R&& r)
        requires std::default_initializable<Allocator>:
        InlineTrivialVector(std::forward<R>(r), Allocator()) {}

    template<std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, value_type>
    constexpr InlineTrivialVector(R&& r, Allocator alloc):
        InlineTrivialVector(std::move(alloc))
    {
        this->assign(std::forward<R>(r));
    }

    constexpr InlineTrivialVector(const InlineTrivialVector& other):
        InlineTrivialVector(
            AllocTraits::select_on_container_copy_construction(
                other.get_allocator()))
    {
        this->assign(other);
    }

    constexpr InlineTrivialVector(InlineTrivialVector&& other) noexcept:
        InlineTrivialVector(std::move(other.allocator()))
    {
        bool can_move = not other.data_is_inlined();
        if (can_move) {
            m_data =
                std::exchange(other.m_data, other.inline_data());
            m_capacity =
                std::exchange(other.m_capacity, other.max_inline_size());
        } else {
            std::ranges::copy_n(other.data(), max_inline_size(), this->data());
        }
        m_size =
            std::exchange(other.m_size, 0);
    }

    constexpr InlineTrivialVector(
        std::initializer_list<value_type> init
    ): InlineTrivialVector(init.begin(), init.end()) {}

    constexpr InlineTrivialVector& operator=(
        const InlineTrivialVector& other
    ) = default;

    constexpr InlineTrivialVector& operator=(
        InlineTrivialVector&& other
    ) noexcept(
        AllocTraits::propagate_on_container_move_assignment::value or
        AllocTraits::is_always_equal::value
    ) {
        constexpr bool propagate =
            AllocTraits::propagate_on_container_move_assignment::value;

        bool can_move = [&] {
            if constexpr (propagate) {
                return not other.data_is_inlined();
            } else {
                return
                    this->allocators_equal(other) and
                    not other.data_is_inlined();
            }
        } ();

        if (can_move) {
            deallocate();
            if constexpr (propagate) {
                this->allocator() = std::move(other.allocator());
            }
            m_data      = other.m_data;
            m_capacity  = other.m_capacity;
            m_size      = other.m_size;
            other.m_data        = other.inline_data();
            other.m_capacity    = other.max_inline_size();
            other.m_size        = 0;
        } else {
            if constexpr (propagate) {
                // Other's data is stored inline
                deallocate();
                this->allocator() = std::move(other.allocator());
                assert(this->capacity() >= other.size());
                std::ranges::copy(other, this->data());
                m_size = other.size();
            } else {
                // Either other's data is stored inline
                // or other's allocator differs from this
                this->assign(other);
            }
            other.clear();
        }

        return *this;
    }

    constexpr void swap(InlineTrivialVector& other) noexcept {
        constexpr bool propagate =
            AllocTraits::propagate_on_container_swap::value;
        assert(propagate or this->allocators_equal(other));

        auto inline_heap_swap = [] (
            InlineTrivialVector& inl, InlineTrivialVector& heap
        ) noexcept {
            assert(inl.data_is_inlined());
            assert(not heap.data_is_inlined());
            std::ranges::copy_n(inl.data(), max_inline_size(), heap.inline_data());
            inl.m_data = std::exchange(heap.m_data, heap.inline_data());
            inl.m_capacity = std::exchange(heap.m_capacity, heap.max_inline_size());
        };

        if (not this->data_is_inlined() and not other.data_is_inlined()) {
            std::ranges::swap(m_data, other.m_data);
            std::ranges::swap(m_capacity, other.m_capacity);
        } else if (this->data_is_inlined() and other.data_is_inlined()) {
            constexpr auto swap_size = max_inline_size();
            std::array<T, swap_size> tmp;
            std::ranges::copy_n(this->data(), swap_size, tmp.data());
            std::ranges::copy_n(other.data(), swap_size, this->data());
            std::ranges::copy_n(tmp.data(), swap_size, other.data());
        } else if (this->data_is_inlined()) {
            inline_heap_swap(*this, other);
        } else {
            inline_heap_swap(other, *this);
        }

        if constexpr (propagate) {
            std::ranges::swap(this->allocator(), other.allocator());
        }

        std::ranges::swap(m_size, other.m_size);
    }

    static constexpr size_type max_inline_size() noexcept {
        return Storage::Capacity;
    }

    constexpr void shrink_to_fit() noexcept {
        if (not this->data_is_inlined()) {
            if (this->size() <= max_inline_size()) {
                std::ranges::copy(*this, this->inline_data());
                deallocate();
            } else if (this->size() <= this->capacity()) {
                try {
                    this->reallocate(this->size());
                } catch (const std::bad_alloc&) {}
            }
        }
    }

protected:
    constexpr void deallocate() noexcept {
        if (m_data != inline_data()) {
            AllocTraits::deallocate(this->allocator(), m_data, m_capacity);
            m_data = inline_data();
            m_capacity = max_inline_size();
        }
    }

    constexpr const T* inline_data() const noexcept {
        return Storage::addr();
    }

    constexpr T* inline_data() noexcept {
        return Storage::addr();
    }
};

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr void TRIVIAL_VECTOR_HEADER::reallocate(size_type new_capacity) {
    assert(new_capacity < max_size());
    auto new_data = allocate(new_capacity);
    std::ranges::copy(*this, new_data);
    deallocate();
    m_data = new_data;
    m_capacity = new_capacity;
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::inline_data() const noexcept -> const T* {
    return static_cast<const InlineTrivialVector<T, 1, Allocator>*>(this)->inline_data();
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::inline_data() noexcept -> T* {
    return static_cast<InlineTrivialVector<T, 1, Allocator>*>(this)->inline_data();
}

INLINE_TRIVIAL_VECTOR_TEMPLATE
void swap(INLINE_TRIVIAL_VECTOR& lhs, INLINE_TRIVIAL_VECTOR& rhs) noexcept {
    lhs.swap(rhs);
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr TRIVIAL_VECTOR_HEADER::size_type erase(
    TRIVIAL_VECTOR_HEADER& vec, const T& value
) noexcept {
    auto old_size = vec.size();
    auto new_size = std::ranges::size(
        std::ranges::remove(vec, value)
    );
    vec.truncate(new_size);
    return old_size - new_size;
}

template<typename T, typename Allocator, typename Pred>
    requires std::same_as<std::invoke_result_t<Pred, T>, bool> 
constexpr TRIVIAL_VECTOR_HEADER::size_type erase_if(
    TRIVIAL_VECTOR_HEADER& vec, Pred pred
) {
    auto old_size = vec.size();
    auto new_size = std::ranges::size(
        std::ranges::remove_if(vec, std::move(pred))
    );
    vec.truncate(new_size);
    return old_size - new_size;
}
}

namespace Attractadore {
using TrivialVectorNameSpace::InlineTrivialVector;
template<
    typename T,
    typename Allocator = std::allocator<T>
> using TrivialVector = InlineTrivialVector<T, 0, Allocator>;
}
