#pragma once
#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <utility>
#include <stdexcept>

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

#define TRIVIAL_VECTOR_HEADER_TEMPLATE \
template<typename T, std::unsigned_integral SizeT, typename Allocator> \
    requires std::is_trivial_v<T>
#define TRIVIAL_VECTOR_HEADER Attractadore::TrivialVectorNameSpace::TrivialVectorHeader<T, SizeT, Allocator>

template<
    typename T,
    std::unsigned_integral SizeT = unsigned,
    typename Allocator = std::allocator<T>
> requires std::is_trivial_v<T>
class TrivialVectorHeader: private Allocator {
    using AllocTraits = std::allocator_traits<Allocator>;
    using Ptr = AllocTraits::pointer;
    using PtrTraits = std::pointer_traits<Ptr>;
    using ConstPtr = AllocTraits::const_pointer;

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
    using iterator = VectorIterator<pointer>;
    using const_iterator = VectorIterator<const_pointer>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

protected:
    constexpr TrivialVectorHeader(Allocator alloc, Ptr data_init, SizeT capacity) noexcept:
        Allocator{std::move(alloc)}, m_data{data_init}, m_capacity{capacity} {}
    constexpr ~TrivialVectorHeader() = default;

public:
    constexpr TrivialVectorHeader& operator=(const TrivialVectorHeader& other);
    constexpr TrivialVectorHeader& operator=(TrivialVectorHeader&& other);
    constexpr TrivialVectorHeader& operator=(std::initializer_list<value_type> init);

    constexpr void assign(size_type count, const value_type& value) {
        fit(count);
        fill(value);
    }
    template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
        requires std::convertible_to<std::iter_value_t<Iter>, value_type>
    constexpr void assign(Iter first, Sent last);
    template<std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, value_type>
    constexpr void assign(R&& r);
    constexpr void assign(std::initializer_list<value_type> init) { assign(init.begin(), init.end()); }

    constexpr void write(size_t count, const value_type& value) noexcept {
        adjust(count);
        fill(value);
    }
    template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
        requires std::convertible_to<std::iter_value_t<Iter>, value_type>
    constexpr void write(Iter first, Sent last) noexcept;
    template<std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, value_type>
    constexpr void write(R&& r) noexcept { write(std::ranges::begin(r), std::ranges::end(r)); }
    constexpr void write(std::initializer_list<value_type> init) noexcept { write(init.begin(), init.end()); }

    constexpr void fill(const value_type& value) noexcept;

    constexpr const allocator_type& get_allocator() const noexcept { return *this; }

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
    constexpr const value_type* cdata() const noexcept { return data(); }
    constexpr const value_type* data() const noexcept { return std::to_address(m_data); }
    constexpr value_type* data() noexcept { return std::to_address(m_data); }

    constexpr const_iterator cbegin() const noexcept { return begin(); }
    constexpr const_iterator cend() const noexcept { return end(); }
    constexpr const_iterator begin() const noexcept { return const_iterator{m_data}; }
    constexpr const_iterator end() const noexcept { return const_iterator{m_data + size()}; }
    constexpr iterator begin() noexcept { return const_iterator{m_data}; }
    constexpr iterator end() noexcept { return const_iterator{m_data + size()}; }

    constexpr const_iterator crbegin() const noexcept { return rbegin(); }
    constexpr const_iterator crend() const noexcept { return rend(); }
    constexpr const_iterator rbegin() const noexcept { return const_reverse_iterator{end()}; }
    constexpr const_iterator rend() const noexcept { return const_reverse_iterator{begin()}; }
    constexpr iterator rbegin() noexcept { return reverse_iterator{end()}; }
    constexpr iterator rend() noexcept { return reverse_iterator{begin()}; }

    constexpr bool empty() const noexcept { return size() == 0; }
    constexpr size_type size() const noexcept { return m_size; }
    static constexpr size_type max_size() noexcept {
        return std::min<size_t>(
            std::numeric_limits<difference_type>::max(),
            std::numeric_limits<size_type>::max() / sizeof(value_type));
    }
    constexpr void reserve(size_type new_capacity);
    constexpr size_type capacity() const noexcept { return m_capacity; }
    constexpr void shrink_to_fit() noexcept;
    constexpr void clear() noexcept;

    constexpr iterator insert(const_iterator pos, const value_type& value);
    constexpr iterator insert(const_iterator pos, size_type count, const value_type& value);
    template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
        requires std::convertible_to<std::iter_value_t<Iter>, value_type>
    constexpr iterator insert(const_iterator pos, Iter first, Sent last);
    template<std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, value_type>
    constexpr iterator insert(const_iterator pos, R&& r);
    constexpr iterator insert(
        const_iterator pos, std::initializer_list<value_type> init
    ) { return insert(pos, init.begin(), init.end()); }
    constexpr iterator emplace(const_iterator pos);
    constexpr iterator emplace(const_iterator pos, const value_type& value) { return insert(pos, value); }

    constexpr iterator add(const_iterator pos) noexcept;
    constexpr iterator add(const_iterator pos, const value_type& value) noexcept;
    constexpr iterator add(const_iterator pos, size_type count, const value_type& value) noexcept;
    template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
        requires std::convertible_to<std::iter_value_t<Iter>, value_type>
    constexpr iterator add(const_iterator pos, Iter first, Sent last) noexcept;
    template<std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, value_type>
    constexpr iterator add(const_iterator pos, R&& r) noexcept;
    constexpr iterator add(
        const_iterator pos, std::initializer_list<value_type> init
    ) noexcept { return add(pos, init.begin(), init.end()); }

    constexpr iterator erase(const_iterator pos) noexcept;
    constexpr iterator erase(const_iterator first, const_iterator last) noexcept;
    template<typename R> requires
        std::ranges::contiguous_range<R> and
        std::ranges::common_range<R> and
        std::convertible_to<std::ranges::iterator_t<R>, const_iterator>
    constexpr iterator erase(R&& r) noexcept { return erase(std::ranges::begin(r), std::ranges::end(r)); }

    constexpr void push_back(const value_type& value) { emplace_back() = value; }
    constexpr iterator append(size_type count, const value_type& value) {
        return insert(end(), count, value);
    }
    template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
        requires std::convertible_to<std::iter_value_t<Iter>, value_type>
    constexpr iterator append(Iter first, Sent last) {
        return insert(end(), first, last);
    }
    template<std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, value_type>
    constexpr iterator append(R&& r) {
        return insert(end(), r);
    }
    constexpr iterator append(std::initializer_list<value_type> init) {
        return insert(end(), init);
    }
    constexpr reference emplace_back();
    constexpr reference emplace_back(const value_type& value) { return emplace_back() = value; }

    constexpr reference write_back() noexcept;
    constexpr reference write_back(const value_type& value) noexcept { return write_back() = value; }
    constexpr iterator add_back(size_type count, const value_type& value) noexcept {
        return add(end(), count, value);
    }
    template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
        requires std::convertible_to<std::iter_value_t<Iter>, value_type>
    constexpr iterator add_back(Iter first, Sent last) noexcept {
        return add(end(), first, last);
    }
    template<std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, value_type>
    constexpr iterator add_back(R&& r) noexcept {
        return add(end(), r);
    }
    constexpr iterator add_back(std::initializer_list<value_type> init) noexcept {
        return add(end(), init);
    }

    constexpr void pop_back() noexcept;
    constexpr value_type pop() noexcept;

    constexpr void resize(size_type new_size);
    constexpr void resize(size_type new_size, const value_type& value);
    template<typename R> requires
        std::ranges::contiguous_range<R> and
        std::ranges::common_range<R> and
        std::convertible_to<std::ranges::iterator_t<R>, const_iterator>
    constexpr void resize(R&& r) noexcept;
    constexpr void adjust(size_type new_size) noexcept;
    constexpr void fit(size_type new_size);
    template<typename OtherAllocator>
    constexpr void swap(TrivialVectorHeader<T, OtherAllocator>& other) noexcept(
        false // TODO
    );

protected:
    constexpr allocator_type& get_allocator() noexcept { return *this; }

    struct AllocateResult {
        pointer     ptr;
        size_type   n;
    };

    constexpr AllocateResult allocate_for_size(size_t new_size);
    constexpr pointer allocate(size_t new_capacity);
    constexpr void reallocate(size_t new_capacity);
    constexpr void deallocate() noexcept;

    constexpr const T* inline_data() const noexcept;
    constexpr T* inline_data() noexcept;

    template<typename F>
    constexpr iterator do_sized_insert(const_iterator pos, size_t count, F do_assign);

    template<typename F>
    constexpr iterator do_sized_add(const_iterator pos, size_t count, F do_assign) noexcept;

    template<typename F>
    constexpr iterator do_sized_realloc_insert(const_iterator pos, size_t count, F do_assign);
};

template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
TrivialVectorHeader(Iter first, Sent last) -> TrivialVectorHeader<std::iter_value_t<Iter>>;

template<std::ranges::input_range R>
TrivialVectorHeader(R&& r) -> TrivialVectorHeader<std::ranges::range_value_t<R>>;

#define INLINE_TRIVIAL_VECTOR_TEMPLATE \
template<typename T, unsigned InlineCapacity, std::unsigned_integral SizeT, typename Allocator> \
    requires std::is_trivial_v<T>
#define INLINE_TRIVIAL_VECTOR Attractadore::TrivialVectorNameSpace::InlineTrivialVector<T, InlineCapacity, SizeT, Allocator>

template<typename T, unsigned MinCapacity, size_t MinAlignSize>
struct InlineStorage {
    static constexpr unsigned AlignSize     = std::max(alignof(T), MinAlignSize);
    static constexpr unsigned MinBufferSize = sizeof(std::array<T, MinCapacity>);
    static constexpr unsigned BufferSize    =
        (MinBufferSize / AlignSize + (MinBufferSize % AlignSize != 0)) * AlignSize;
    static constexpr unsigned Capacity      = BufferSize / sizeof(T);

    static constexpr unsigned capacity = Capacity;
    std::array<T, capacity> m_storage;

    constexpr const T* data() const noexcept { return m_storage.data(); }
    constexpr T* data() noexcept { return m_storage.data(); }
};

template<typename T, size_t MinAlignSize>
struct InlineStorage<T, 0, MinAlignSize> {
    static constexpr unsigned capacity = 0;
    constexpr std::nullptr_t data() const noexcept { return nullptr; }
};

inline constexpr unsigned DefaultInlineBufferSize = 64;

template<typename T>
inline unsigned DefaultInlineCapacity = DefaultInlineBufferSize / sizeof(T);

template<
    typename T,
    unsigned InlineCapacity = DefaultInlineCapacity<T>,
    std::unsigned_integral SizeT = unsigned,
    typename Allocator = std::allocator<T>
> requires std::is_trivial_v<T>
class InlineTrivialVector:
    public TrivialVectorHeader<T, SizeT, Allocator>,
    private InlineStorage<T, InlineCapacity, alignof(TrivialVectorHeader<T, SizeT, Allocator>)>
{
    using Base = TrivialVectorHeader<T, SizeT, Allocator>;
    using Storage = InlineStorage<T, InlineCapacity, alignof(Base)>;
    using typename Base::AllocTraits;

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

    constexpr InlineTrivialVector() noexcept(noexcept(Allocator()));
    constexpr explicit InlineTrivialVector(Allocator alloc) noexcept;
    constexpr explicit InlineTrivialVector(size_type size, Allocator alloc = Allocator());
    constexpr InlineTrivialVector(size_type count, const T& value, Allocator alloc = Allocator());
    template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
        requires std::convertible_to<std::iter_value_t<Iter>, value_type>
    constexpr InlineTrivialVector(Iter first, Sent last, Allocator alloc = Allocator());
    template<std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, value_type>
    constexpr explicit InlineTrivialVector(R&& r, Allocator alloc = Allocator());
    // TODO: conversions?
    constexpr InlineTrivialVector(std::initializer_list<value_type> init, Allocator alloc = Allocator());
    constexpr InlineTrivialVector(const InlineTrivialVector& other);
    constexpr InlineTrivialVector(const InlineTrivialVector& other, Allocator alloc);
    constexpr InlineTrivialVector(InlineTrivialVector&& other) noexcept;
    constexpr InlineTrivialVector(InlineTrivialVector&& other, Allocator alloc);
    constexpr InlineTrivialVector& operator=(const InlineTrivialVector& other);
    constexpr InlineTrivialVector& operator=(InlineTrivialVector&& other) noexcept(
        AllocTraits::propagate_on_container_move_assignment::value or
        AllocTraits::is_always_equal::value
    );
    template<std::ranges::input_range R>
    constexpr InlineTrivialVector& operator=(R&& r);
    constexpr InlineTrivialVector& operator=(std::initializer_list<value_type> init);
    constexpr ~InlineTrivialVector();

    static constexpr size_type max_inline_size() noexcept { return Storage::capacity; }

protected:
    constexpr const T* inline_data() const noexcept { return Storage::data(); }
    constexpr T* inline_data() noexcept { return Storage::data(); }
};

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::operator=(
    const TrivialVectorHeader& other
) -> TrivialVectorHeader& {
    bool copy_alloc =
        AllocTraits::propagate_on_container_copy_assignment and
        not AllocTraits::is_always_equal and
        get_allocator() != other.get_allocator();
    if (copy_alloc) {
        if (capacity() < other.size()) {
            auto new_alloc = other.get_allocator();
            auto [new_data, new_capacity] = allocate_for_size(new_alloc, other.size());
            deallocate();
            get_allocator() = std::move(new_alloc);
            m_data = new_data;
            m_capacity = new_capacity;
        } else {
            get_allocator() = other.get_allocator();
        }
        std::ranges::copy(other, data());
        m_size = other.size();
    } else {
        assign(other);
    }
    return *this;
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::operator=(
    TrivialVectorHeader&& other
) -> TrivialVectorHeader& {
    bool can_steal =
        (AllocTraits::propagate_on_container_move_assignment or
         AllocTraits::is_always_equal or 
         get_allocator() == other.get_allocator()) and
        not other.data_is_inlined();
    if (can_steal) {
        deallocate();
        constexpr bool move_alloc = 
            AllocTraits::propagate_on_container_move_assignment;
        if constexpr(move_alloc) {
            get_allocator() = std::move(other.get_allocator());
        }
        m_data = std::exchange(other.m_data, nullptr);
        m_size = std::exchange(other.m_size, 0);
        m_capacity = std::exchange(other.m_capacity, 0);
    } else {
        // Stealing is not possible if:
        // 1) other's data is stored inline
        // 2) allocators are not equal and propagation is not allowed
        //
        // Solution: resize and copy
        fit(other.size());
        std::ranges::copy(other, data());
    }
    return *this;
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::operator=(
    std::initializer_list<T> init
) -> TrivialVectorHeader& {
    assign(init);
    return *this;
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
    requires std::convertible_to<std::iter_value_t<Iter>, typename TRIVIAL_VECTOR_HEADER::value_type>
constexpr void TRIVIAL_VECTOR_HEADER::assign(Iter first, Sent last) {
    if constexpr (std::sized_sentinel_for<Sent, Iter>) {
        auto new_size = std::ranges::distance(first, last);
        fit(new_size);
        std::ranges::copy(first, last, data());
    } else {
        auto new_size = 0;
        for (; new_size < capacity() and first != last; new_size++, ++first) {
            data()[new_size] = *first;
        }
        m_size = new_size;
        std::ranges::copy(first, last, std::back_inserter(*this));
    }
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, typename TRIVIAL_VECTOR_HEADER::value_type>
constexpr void TRIVIAL_VECTOR_HEADER::assign(R&& r) {
    if constexpr (std::ranges::sized_range<R>) {
        auto new_size = std::ranges::size(r);
        fit(new_size);
        std::ranges::copy(r, data());
    } else {
        assign(std::ranges::begin(r), std::ranges::end(r));
    }
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
    requires std::convertible_to<std::iter_value_t<Iter>, typename TRIVIAL_VECTOR_HEADER::value_type>
constexpr void TRIVIAL_VECTOR_HEADER::write(Iter first, Sent last) noexcept {
    auto new_end = std::ranges::copy(first, last, data());
    m_size = std::ranges::distance(data(), new_end);
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr void TRIVIAL_VECTOR_HEADER::fill(const value_type& value) noexcept {
    std::ranges::fill(*this, value);
}

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
constexpr void TRIVIAL_VECTOR_HEADER::reserve(size_type new_capacity) {
    if (new_capacity > capacity()) {
        reallocate(new_capacity);
    }
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr void TRIVIAL_VECTOR_HEADER::shrink_to_fit() noexcept {
    if (size() < capacity()) {
        try {
            reallocate(size());
        } catch (const std::bad_alloc&) {}
    }
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr void TRIVIAL_VECTOR_HEADER::clear() noexcept {
    adjust(0);
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::insert(
    const_iterator pos, const value_type& value
) -> iterator {
    return do_sized_insert(pos, 1,
        [&](auto it) {
            *it = value;
            return ++it;
        }
    );
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::insert(
    const_iterator pos, size_type count, const value_type& value
) -> iterator {
    if (count) {
        return do_sized_insert(pos, count,
            [&](auto it) { return std::ranges::fill_n(it, count, value).out; }
        );
    }
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
    requires std::convertible_to<std::iter_value_t<Iter>, typename TRIVIAL_VECTOR_HEADER::value_type>
constexpr auto TRIVIAL_VECTOR_HEADER::insert(
    const_iterator pos, Iter first, Sent last
) -> iterator {
    if constexpr (std::sized_sentinel_for<Sent, Iter>) {
        auto count = std::ranges::distance(first, last);
        if (count) {
            return do_sized_insert(pos, count,
                [&] (auto it) { return std::ranges::copy(first, last, it).out; }
            );
        }
    } else {
        auto idx = std::ranges::distance(begin(), pos);
        // Copy as much of the range as possible into available storage
        auto cap_end = data() + capacity();
        auto insert_begin = data() + idx;
        auto insert_end = std::ranges::copy_backward(insert_begin, end(), cap_end).out;
        Ptr it;
        std::tie(first, it) = copy_some(first, last, insert_begin, insert_end);
        if (first == last) {
            // All of the new data fits
            // If all available space was used, there is no need to shift elements
            if (it != insert_end) {
                auto new_end = std::ranges::copy(insert_end, cap_end, it);
                m_size = std::ranges::distance(data(), new_end);
            } else {
                m_size = capacity();
            }
            return iterator{insert_begin};
        } else {
            // The new data doesn't fit
            auto end_size = std::ranges::distance(insert_begin, end());
            try { while (first != last) {
                auto [new_data, new_capacity] = allocate_for_size(capacity() + 1);
                auto new_cap_end = new_data + new_capacity;
                // Copy everything except the last bit
                auto new_insert_begin = std::ranges::copy(data(), insert_end, new_data).out;
                auto new_insert_end = new_cap_end - end_size;
                // Copy input data until it runs out or doesn't fit 
                std::tie(first, new_insert_end) = copy_some(first, last, new_insert_begin, new_insert_end);
                // Copy last bit
                std::ranges::copy_n(insert_end, end_size, new_insert_end);
                deallocate();
                m_data = new_data;
                m_capacity = new_capacity;
                insert_end = new_insert_end;
            }} catch (std::bad_alloc&) {
                std::ranges::copy(insert_end, cap_end, data() + idx);
                throw;
            }
            m_size = std::ranges::distance(data(), insert_end) + end_size;
            return begin() + idx;
        }
    }
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, typename TRIVIAL_VECTOR_HEADER::value_type>
constexpr auto TRIVIAL_VECTOR_HEADER::insert(
    const_iterator pos, R&& r
) -> iterator {
    if constexpr (std::ranges::sized_range<R>) {
        auto count = std::ranges::size(r);
        if (count) {
            return do_sized_insert(pos, count,
                [&] (auto it) { return std::ranges::copy(r, it).out; }
            );
        }
    } else {
        return insert(std::ranges::begin(r), std::ranges::end(r));
    }
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::emplace(
    const_iterator pos
) -> iterator {
    return do_sized_insert(pos, 1, [](auto it) { return ++it; });
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::add(
    const_iterator pos
) noexcept -> iterator {
    return do_sized_add(pos, 1, [](auto it) { return ++it; });
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::add(
    const_iterator pos, const value_type& value
) noexcept -> iterator {
    return do_sized_add(pos, 1,
        [&](auto it) {
            *it = value;
            return ++it;
        }
    );
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::add(
    const_iterator pos, size_type count, const value_type& value
) noexcept -> iterator {
    if (count) {
        return do_sized_add(pos, count,
            [&](auto it) { return std::ranges::fill_n(it, count, value).out; }
        );
    }
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
template<std::input_iterator Iter, std::sentinel_for<Iter> Sent>
    requires std::convertible_to<std::iter_value_t<Iter>, typename TRIVIAL_VECTOR_HEADER::value_type>
constexpr auto TRIVIAL_VECTOR_HEADER::add(
    const_iterator pos, Iter first, Sent last
) noexcept -> iterator {
    if constexpr (std::sized_sentinel_for<Sent, Iter>) {
        auto count = std::ranges::distance(first, last);
        if (count) {
            return do_sized_add(pos, count,
                [&] (auto it) { return std::ranges::copy(first, last, it).out; }
            );
        }
    } else {
        auto idx = std::ranges::distance(begin(), pos);
        auto cap_end = data() + capacity();
        auto insert_begin = data() + idx;
        auto insert_end = std::ranges::copy_backward(insert_begin, end(), cap_end).out;
        auto it = std::ranges::copy(first, last, insert_begin).out;
        assert(it <= insert_end);
        // If all available space was used, there is no need to shift elements
        if (it != insert_end) {
            auto new_end = std::ranges::copy(insert_end, cap_end, it);
            m_size = std::ranges::distance(data(), new_end);
        } else {
            m_size = capacity();
        }
        return iterator{insert_begin};
    }
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, typename TRIVIAL_VECTOR_HEADER::value_type>
constexpr auto TRIVIAL_VECTOR_HEADER::add(
    const_iterator pos, R&& r
) noexcept -> iterator {
    if constexpr (std::ranges::sized_range<R>) {
        auto count = std::ranges::size(r);
        if (count) {
            return do_sized_add(pos, count,
                [&] (auto it) { return std::ranges::copy(r, it).out; }
            );
        }
    } else {
        return add(std::ranges::begin(r), std::ranges::end(r));
    }
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
template<typename AssignF>
constexpr auto TRIVIAL_VECTOR_HEADER::do_sized_insert(
    const_iterator pos, size_type count, AssignF do_assign
) -> iterator {
    assert(count);
    auto new_size = size() + count;
    [[likely]]
    if (new_size <= capacity()) {
        return do_sized_add(pos, count, std::move(do_assign));
    } else {
        return do_sized_realloc_insert(pos, count, std::move(do_assign));
    };
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
template<typename AssignF>
constexpr auto TRIVIAL_VECTOR_HEADER::do_sized_add(
    const_iterator pos, size_type count, AssignF do_assign
) noexcept -> iterator {
    assert(count);
    auto idx = std::ranges::distance(begin(), pos);
    auto new_size = size() + count;
    assert(new_size <= capacity());
    auto assign_begin = begin() + idx;
    auto assign_end = assign_begin + count;
    m_size = new_size;
    std::ranges::copy_backward(assign_begin, assign_end, end());
    do_assign(assign_begin);
    return assign_begin;
}


TRIVIAL_VECTOR_HEADER_TEMPLATE
template<typename AssignF>
constexpr auto TRIVIAL_VECTOR_HEADER::do_sized_realloc_insert(
    const_iterator pos, size_type count, AssignF do_assign
) -> iterator {
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

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::erase(
    const_iterator pos
) noexcept -> iterator {
    auto idx = std::ranges::distance(begin(), pos);
    assert(idx < size());
    auto it = begin() + idx;
    std::ranges::copy_backward(pos + 1, end(), it);
    m_size--;
    return it;
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::erase(
    const_iterator first, const_iterator last
) noexcept -> iterator {
    auto idx = std::ranges::distance(begin(), first);
    assert(idx <= size());
    auto it = begin() + idx;
    if (first != last) {
        auto count = std::ranges::distance(first, last);
        assert(idx + count <= size());
        std::ranges::copy_backward(last, end(), it);
        m_size -= count;
    }
    return it;
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::emplace_back() -> reference {
    [[likely]]
    if (size() < capacity()) {
        return data()[m_size++];
    } else {
        auto [new_data, new_capacity] = allocate_for_size(m_size + 1);
        std::ranges::copy_n(data(), size(), new_data);
        m_data = new_data;
        m_capacity = new_capacity;
        return data()[m_size++];
    }
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::write_back() noexcept -> reference {
    return data()[m_size++];
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr void TRIVIAL_VECTOR_HEADER::pop_back() noexcept {
    m_size--;
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::pop() noexcept -> value_type {
    return data()[--m_size];
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr void TRIVIAL_VECTOR_HEADER::resize(size_type new_size) {
    if (capacity() < new_size) {
        auto [new_data, new_capacity] = allocate_for_size(m_size + 1);
        std::ranges::copy_n(data(), size(), new_data);
        m_data = new_data;
        m_capacity = new_capacity;
    }
    m_size = new_size;
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr void TRIVIAL_VECTOR_HEADER::resize(size_type new_size, const value_type& value) {
    auto old_size = size();
    resize(new_size);
    auto idx = std::min(old_size, new_size);
    std::ranges::fill(data() + idx, data() + new_size, value);
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
template<typename R> requires
    std::ranges::contiguous_range<R> and
    std::ranges::common_range<R> and
    std::convertible_to<
        std::ranges::iterator_t<R>,
        typename TRIVIAL_VECTOR_HEADER::const_iterator>
constexpr void TRIVIAL_VECTOR_HEADER::resize(R&& r) noexcept {
    assert(r.begin() >= begin() and r.end() <= end());
    if (r.begin() != begin()) {
        std::ranges::copy(r, begin());
    }
    m_size = r.size();
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr void TRIVIAL_VECTOR_HEADER::adjust(size_type new_size) noexcept {
    assert(new_size <= capacity());
    m_size = new_size;
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr void TRIVIAL_VECTOR_HEADER::fit(size_type new_size) {
    if (capacity() < new_size) {
        auto [new_data, new_capacity] = allocate_for_size(m_size + 1);
        m_data = new_data;
        m_capacity = new_capacity;
    }
    m_size = new_size;
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::inline_data() const noexcept -> const T* {
    return static_cast<const InlineTrivialVector<T, 1, SizeT, Allocator*>>(*this)->inline_data();
}

TRIVIAL_VECTOR_HEADER_TEMPLATE
constexpr auto TRIVIAL_VECTOR_HEADER::inline_data() noexcept -> T* {
    return static_cast<const InlineTrivialVector<T, 1, SizeT, Allocator*>>(*this)->inline_data();
}

template<
    typename T1, typename Allocator1,
    typename T2, typename Allocator2
> constexpr bool operator==(
    const TrivialVectorHeader<T1, Allocator1>& l,
    const TrivialVectorHeader<T2, Allocator2>& r
) noexcept {
    return std::ranges::equal(l, r);
}

template<
    typename T1, typename Allocator1,
    typename T2, typename Allocator2
> constexpr auto operator<=>(
    const TrivialVectorHeader<T1, Allocator1>& l,
    const TrivialVectorHeader<T2, Allocator2>& r
) noexcept {
    return std::lexicographical_compare_three_way(
        l.begin(), l.end(),
        r.begin(), r.end());
}

template<
    typename T1, typename Allocator1,
    typename T2, typename Allocator2
> void swap(
    TrivialVectorHeader<T1, Allocator1>& l,
    TrivialVectorHeader<T2, Allocator2>& r
) noexcept (
    false //TODO
) {
    l.swap(r);
}
}

namespace std {
template<typename T, std::unsigned_integral SizeT, typename Allocator, typename U>
constexpr TRIVIAL_VECTOR_HEADER::size_type erase(TRIVIAL_VECTOR_HEADER& vec, const U& value) {
    auto old_size = vec.size();
    auto new_size = std::ranges::remove(vec, value).size();
    vec.adjust(new_size);
    return old_size - new_size;
}

template<typename T, std::unsigned_integral SizeT, typename Allocator, typename Pred>
constexpr TRIVIAL_VECTOR_HEADER::size_type erase_if(TRIVIAL_VECTOR_HEADER& vec, Pred pred) {
    auto old_size = vec.size();
    auto new_size = std::ranges::remove_if(vec, std::move(pred)).size();
    vec.adjust(new_size);
    return old_size - new_size;
}
}
