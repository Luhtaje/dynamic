#ifndef DYNAMIC_RINGBUFFER_HPP
#define DYNAMIC_RINGBUFFER_HPP

#include <memory>
#include <algorithm>
#include <limits>
#include <utility>
#include <stdexcept>
#include <cstring>

namespace
{
    // Buffer always reserves two "extra" spaces. This ensures that reserve and other relocating functions work correctly (ensures that there is room to operate on index and move it after).
    constexpr size_t allocBuffer = 2;
}
// Forward declaration of _rBuf_const_iterator.
template<class _rBuf>
class _rBuf_const_iterator;

//Base class that wraps memory allocation into an initialization (RAII).
template<typename T, typename Allocator = std::allocator<T>>
struct ring_buffer_base {

    using size_type = std::size_t;

protected:
    size_type m_capacity;  /*!< Capacity of the buffer. How many elements of type T the buffer has currently allocated memory for.*/

    T* m_data;  /*!< Pointer to allocated memory.*/
    Allocator m_allocator;  /*!< Allocator used to allocate/deallocate and construct/destruct elements. Default is std::allocator<T>*/

public:
    ring_buffer_base(const Allocator& alloc, size_type capacity)
        : m_allocator(alloc), m_data(std::allocator_traits<Allocator>::allocate(m_allocator, capacity)), m_capacity(capacity)
    {
    }

    ring_buffer_base(ring_buffer_base&& other) noexcept : m_allocator(std::move(other.m_allocator))
    {
        m_data = std::exchange(other.m_data, nullptr);
        m_capacity = std::exchange(other.m_capacity, 0);
    }

    void swap(ring_buffer_base& other) noexcept
    {
        std::swap(m_allocator, other.m_allocator);
        std::swap(m_data, other.m_data);
        std::swap(m_capacity, other.m_capacity);
    }

    ring_buffer_base() { std::allocator_traits<Allocator>::deallocate(m_data, m_capacity); }
};

/// @brief Dynamic Ringbuffer is a dynamically growing std::container with support for queue, stack and priority queue adaptor functionality. 
/// @tparam T Type of the elements.
/// @tparam Allocator Allocator used for (de)allocation and (de)construction. Defaults to std::allocator<T>
template<typename T, typename Allocator = std::allocator<T>> 
class ring_buffer : private ring_buffer_base<T,Allocator>
{

public:

    using base = typename ring_buffer<T,Allocator>::ring_buffer_base;

    using value_type = T;
    using allocator_type = Allocator;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;


//========================================
// Iterators
//========================================

    /// @brief Custom iterator class.
    /// @tparam _rBuf ring_buffer class type.
    template<class _rBuf>
    class _rBuf_const_iterator
    {

    public:
        using iterator_category = std::random_access_iterator_tag;

        using value_type = typename _rBuf::value_type;
        using difference_type = typename _rBuf::difference_type;
        using pointer = typename _rBuf::const_pointer;
        using reference = const value_type&;

    public:
        _rBuf_const_iterator() : m_container(nullptr), m_logicalIndex(0) {}

        /// @brief Constructor.
        /// @param index Index representing the logical element of the buffer where iterator points to.
        explicit _rBuf_const_iterator(const _rBuf* container, difference_type index) : m_container(container), m_logicalIndex(index) {}

        /// @brief Arrow operator.
        /// @return pointer.
        /// @details Constant complexity.
        pointer operator->() const
        {
            return &(*m_container)[m_logicalIndex];
        }

        /// @brief Postfix increment
        /// @note If the iterator is incremented over the end() iterator leads to invalid iterator (dereferencing is undefined behaviour).
        /// @details Constant complexity.
        _rBuf_const_iterator& operator++() noexcept
        {
            m_logicalIndex++;

            return (*this);
        }

        /// @brief Postfix increment
        /// @param  int empty parameter to guide overload resolution.
        /// @note If the iterator is incremented over the end() iterator leads to invalid iterator (dereferencing is undefined behaviour).
        /// @details Constant complexity.
        _rBuf_const_iterator operator++(int)
        {
            auto temp(*this);
            ++m_logicalIndex;

            return temp;
        }

        /// @brief Prefix decrement.
        /// @note Decrementing the iterator past begin() leads to invalid iterator (dereferencing is undefined behaviour).
        /// @details Constant complexity.
        _rBuf_const_iterator& operator--()
        {
            --m_logicalIndex;
            return(*this);
        }

        /// @brief Postfix decrement
        /// @param  int empty parameter to guide overload resolution.
        /// @note Decrementing iterator past begin() results in undefined behaviour.
        /// @details Constant complexity.
        _rBuf_const_iterator operator--(int)
        {
            auto temp(*this);
            --m_logicalIndex;
            return temp;
        }

        /// @brief Moves iterator.
        /// @param offset Amount of elements to move. Negative values move iterator backwards.
        /// @note If offset is such that the iterator is beyond end() or begin(), the return iterator is invalid (dereferencing it is undefined behaviour).
        /// @details Constant complexity.
        _rBuf_const_iterator& operator+=(difference_type offset) noexcept
        {
            if (offset < 0)
            {
                m_logicalIndex -= abs(offset);
            }
            else
            {
                m_logicalIndex += offset;
            }
            return (*this);
        }

        /// @brief Move iterator forward by specified amount.
        /// @param movement Amount of elements to move the iterator.
        /// @note If offset is such that the iterator is beyond end() or begin(), the return iterator is invalid (dereferencing it is undefined behaviour).
        /// @details Constant complexity.
        _rBuf_const_iterator operator+(const difference_type offset) const
        {
            _rBuf_const_iterator temp(m_container, m_logicalIndex);
            return (temp += offset);
        }

        /// @brief Addition operator with the offset at the beginning of the operation.
        /// @param offset The number of positions to move the iterator forward.
        /// @param iter Base iterator to what the offset is added to.
        /// @note If offset is such that the iterator is beyond end() or begin(), the return iterator is invalid (dereferencing it is undefined behaviour).
        /// @details Constant complexity.
        friend _rBuf_const_iterator operator+(const difference_type offset, _rBuf_const_iterator iter)
        {
            auto temp = iter;
            temp += offset;
            return temp;
        }

        /// @brief Returns an iterator that points to an element, which is the current element decremented by the given offset.
        /// @param offset The number of positions to move the iterator backward.
        /// @return An iterator pointing to the element that is offset positions before the current element.
        /// @details Constant complexity.
        _rBuf_const_iterator& operator-=(const difference_type offset) noexcept
        {
            return (*this += -offset);
        }

        /// @brief Returns an iterator that points to an element, which is the current element decremented by the given offset.
        /// @param offset The number of positions to move the iterator backward.
        /// @return An iterator pointing to the element that is offset positions before the current element.
        /// @note If offset is such that the index of the iterator is beyond end() or begin(), the return iterator is invalid (dereferencing it is undefined behaviour).
        /// @details Constant complexity.
        _rBuf_const_iterator operator-(const difference_type offset) const
        {
            _rBuf_const_iterator temp(m_container, m_logicalIndex);
            return (temp -= offset);
        }

        /// @brief Gets distance between two iterators.
        /// @param iterator Iterator to get distance to.
        /// @return Amount of elements between the iterators.
        /// @details Constant complexity.
        difference_type operator-(const _rBuf_const_iterator& other) const noexcept
        {
            return (m_logicalIndex - other.m_logicalIndex);
        }

        /// @brief Index operator.
        /// @param offset The offset from iterator.
        /// @return Return reference to element pointed by the iterator with offset.
        /// @note If offset is such that the iterator is beyond end() or begin() this function has undefined behaviour.
        /// @details Constant complexity.
        reference operator[](const difference_type offset) const noexcept
        {
            return m_container->operator[](m_logicalIndex + offset);
        }

        /// @brief Comparison operator== overload
        /// @param other iterator to compare
        /// @return True if iterators point to same element in same container.
        /// @details Constant complexity.
        bool operator==(const _rBuf_const_iterator& other) const noexcept
        {
            return (m_logicalIndex == other.m_logicalIndex) && (m_container == other.m_container);
        }

        /// @brief Comparison operator != overload
        /// @param other iterator to compare
        /// @return ture if underlying pointers are not the same
        /// @details Constant complexity.
        bool operator!=(const _rBuf_const_iterator& other) const noexcept
        {
            return !(m_logicalIndex == other.m_logicalIndex && m_container == other.m_container);
        }

        /// @brief Comparison operator < overload
        /// @param other iterator to compare against.
        /// @return True if other is larger.
        /// @note Comparing to an iterator from another container is undefined.
        /// @details Constant complexity.
        bool operator<(const _rBuf_const_iterator& other) const noexcept
        {
            return (m_logicalIndex < other.m_logicalIndex);
        }

        /// @brief Comparison operator > overload
        /// @param other iterator to compare against.
        /// @return True if other is smaller.
        /// @note Comparing to an iterator from another container is undefined.
        /// @details Constant complexity.
        bool operator>(const _rBuf_const_iterator& other) const noexcept
        {
            return (other.m_logicalIndex < m_logicalIndex);
        }

        /// @brief Less or equal operator.
        /// @param other Other iterator to compare against.
        /// @return Returns true if index of this is less or equal than other's. Otherwise false.
        /// @note Comparing to an iterator from another container is undefined.
        /// @details Constant complexity.
        bool operator<=(const _rBuf_const_iterator& other) const noexcept
        {
            return (!(other < m_logicalIndex));
        }

        /// @brief Greater or equal than operator.
        /// @param other Iterator to compare against.
        /// @return Returns true if this's index is greater than or equal to other.
        /// @note Comparing to an iterator from another container is undefined.
        /// @details Constant complexity.
        bool operator>=(const _rBuf_const_iterator& other) const noexcept
        {
            return (!(m_logicalIndex < other.m_logicalIndex));
        }

        /// @brief Custom assingment operator overload.
        /// @param index Logical index of the element which point to.
        /// @details Constant complexity.
        _rBuf_const_iterator& operator=(const size_t index) noexcept
        {
            m_logicalIndex = index;
            return (*this);
        };

        /// @brief Dereference operator.
        /// @return Object pointed by iterator.
        /// @details Constant complexity.
        reference operator*() const noexcept
        {
            return (*m_container)[m_logicalIndex];

        }

        /// @brief Returns the logical index of the element the iterator is pointing to.
        /// @details Constant complexity.
        difference_type getIndex() const noexcept
        {
            return m_logicalIndex;
        }

    protected:
        // The parent container.
        const _rBuf* m_container;

        // The iterator does not point to any memory location, but is interfaced to the Ring Buffer via an index which is the logical index
        // to an element. Logical index 0 is the first element in the buffer and last is size - 1.
        difference_type m_logicalIndex;
    };

    /// @brief Custom iterator class.
    /// @tparam T Type of the element what iterator points to.
    template<class _rBuf>
    class _rBuf_iterator : public _rBuf_const_iterator<_rBuf>
    {

    public:
        using iterator_category = std::random_access_iterator_tag;

        using value_type = typename _rBuf::value_type;
        using difference_type = typename _rBuf::difference_type;
        using pointer = typename _rBuf::pointer;
        using reference = value_type&;

    public:

        /// @brief Default constructor
        _rBuf_iterator() = default;

        /// @brief Constructor.
        /// @param container Pointer to the ring_buffer element which owns this iterator.
        /// @param index Index pointing to the logical element of the ring_buffer.
        /// @details Constant complexity.
        explicit _rBuf_iterator(_rBuf* container, size_type index) : _rBuf_const_iterator<_rBuf>(container, index) {}

        /// @brief Dereference operator
        /// @return  Returns the object the iterator is currently pointing to.
        /// @details Constant complexity.
        reference operator*() const noexcept
        {
            return (*(const_cast<_rBuf*>(_rBuf_const_iterator<_rBuf>::m_container)))[_rBuf_const_iterator<_rBuf>::m_logicalIndex];
        }

        /// @brief Arrow operator. 
        /// @return Returns a pointer to the object the iterator is currently pointing to.
        /// @details Constant complexity.
        pointer operator->() const noexcept
        {
            return const_cast<pointer>(_rBuf_const_iterator<_rBuf>::operator->());
        }

        /// @brief Prefix increment.
        /// @note Incrementing the iterator over the end() iterator leads to invalid iterator (dereferencing is undefined behaviour).
        /// @details Constant complexity.
        _rBuf_iterator& operator++() noexcept
        {
            ++_rBuf_const_iterator<_rBuf>::m_logicalIndex;
            return (*this);
        }

        /// @brief Postfix increment
        /// @param  int empty parameter to guide overload resolution.
        /// @note Incrementing the iterator over the end() iterator leads to invalid iterator (dereferencing is undefined behaviour).
        /// @details Constant complexity.
        _rBuf_iterator operator++(int)
        {
            auto temp(*this);
            ++_rBuf_const_iterator<_rBuf>::m_logicalIndex;
            return temp;
        }

        /// @brief Prefix decrement
        /// @Details Constant complexity.
        /// @note Decrementing the iterator past begin() leads to invalid iterator (dereferencing is undefined behaviour).
        _rBuf_iterator& operator--() noexcept
        {
            --_rBuf_const_iterator<_rBuf>::m_logicalIndex;
            return(*this);
        }

        /// @brief Postfix decrement
        /// @param  int empty parameter to guide overload resolution.
        /// @details Constant complexity.
        /// @note Decrementing the iterator past begin() leads to invalid iterator (dereferencing is undefined behaviour).
        _rBuf_iterator operator--(int)
        {
            auto temp(*this);
            --_rBuf_const_iterator<_rBuf>::m_logicalIndex;
            return temp;
        }

        /// @brief Moves iterator forward.
        /// @param offset Amount of elements to move.
        /// @note Moving the iterator beyond begin() or end() makes the iterator point to an invalid element.
        /// @details Constant complexity.
        _rBuf_iterator& operator+=(difference_type offset) noexcept
        {
            if (offset < 0)
            {
                _rBuf_const_iterator<_rBuf>::m_logicalIndex -= abs(offset);
            }
            else
            {
                _rBuf_const_iterator<_rBuf>::m_logicalIndex += offset;
            }
            return (*this);
        }

        /// @brief Create a temporary iterator that has been moved forward by specified amount.
        /// @param offset Amount of elements to move the iterator.
        /// @details Constant complexity.
        _rBuf_iterator operator+(const difference_type offset) const
        {
            _rBuf_iterator temp(*this);
            return (temp += offset);
        }

        /// @brief Friend operator+. Creates a copy of an iterator which has been moved by given amount.
        /// @param offset Amount of elements to move the iterator. 
        /// @param iter Reference to base iterator.
        /// @note Enables (n + a) expression, where n is a constant and a is iterator type.
        /// @details Constant complexity.
        friend _rBuf_iterator operator+(const difference_type offset, const _rBuf_iterator& iter)
        {
            auto temp = iter;
            temp += offset;
            return temp;
        }

        /// @brief Decrement this iterator by offset.
        /// @param offset The number of positions to move the iterator backward.
        /// @return An iterator pointing to the element that is offset positions before the current element.
        /// @details Constant complexity.
        _rBuf_iterator& operator-=(const difference_type offset) noexcept
        {
            return (*this += -offset);
        }

        /// @brief Get iterator decremented by offset.
        /// @param offset Signed amount to decrement from the iterator index.
        /// @return An iterator pointing to an element that points to *this - offset.
        /// @note If offset is such that the index of the iterator is beyond end() or begin(), the return iterator is invalid (dereferencing it is undefined behaviour).
        /// @details Constant complexity.
        _rBuf_iterator operator-(const difference_type offset) const
        {
            _rBuf_iterator temp(*this);
            return (temp -= offset);
        }

        /// @brief Decrement operator between two iterators.
        /// @param other Other iterator.
        /// @return Return the difference between the elements to what the iterators point to.
        /// @details Constant complexity.
        difference_type operator-(const _rBuf_iterator& other) const noexcept
        {
            return (_rBuf_const_iterator<_rBuf>::m_logicalIndex - other._rBuf_const_iterator<_rBuf>::m_logicalIndex);
        }

        /// @brief Index operator.
        /// @param offset Signed offset from iterator index.
        /// @return Return object pointer by the iterator with an offset.
        /// @note If offset is such that the iterator is beyond end() or begin() this function has undefined behaviour.
        /// @details Constant complexity.
        reference operator[](const difference_type offset) const noexcept
        {
            return const_cast<reference>(_rBuf_const_iterator<_rBuf>::m_container->operator[](offset));
        }

        /// @brief Comparison operator < overload.
        /// @param other iterator to compare.
        /// @return true if others index is larger.
        /// @details Constant complexity.
        bool operator<(const _rBuf_iterator& other) const noexcept
        {
            return (_rBuf_const_iterator<_rBuf>::m_logicalIndex < other._rBuf_const_iterator<_rBuf>::m_logicalIndex);
        }

        /// @brief Comparison operator > overload.
        /// @param other iterator to compare against.
        /// @return True if others index is smaller.
        /// @details Constant complexity.
        bool operator>(const _rBuf_iterator& other) const noexcept
        {
            return (_rBuf_const_iterator<_rBuf>::m_logicalIndex > other._rBuf_const_iterator<_rBuf>::m_logicalIndex);
        }

        /// @brief Comparison <= overload.
        /// @param other Other iterator to compare against.
        /// @return True if other points to logically smaller or the same indexed element.
        /// @details Constant complexity.
        bool operator<=(const _rBuf_iterator& other) const noexcept
        {
            return (_rBuf_const_iterator<_rBuf>::m_logicalIndex <= other._rBuf_const_iterator<_rBuf>::m_logicalIndex);
        }

        /// @brief Comparison >= overload.
        /// @param other Other iterator to compare against.
        /// @return True if other points to logically larger or same indexed element.
        /// @details Constant complexity.
        bool operator>=(const _rBuf_iterator& other) const noexcept
        {
            return (_rBuf_const_iterator<_rBuf>::m_logicalIndex >= other._rBuf_const_iterator<_rBuf>::m_logicalIndex);
        }

        /// @brief Custom assingment operator overload.
        /// @param index Logical index of the element to set the iterator to.
        /// @note Undefined behaviour for negative index.
        /// @details Constant complexity.
        _rBuf_iterator& operator=(const size_t index) noexcept
        {
            _rBuf_const_iterator<_rBuf>::m_logicalIndex = index;
            return (*this);
        };

        /// @brief Index getter.
        /// @return Returns the index of the element this iterator is pointing to.
        /// @details Constant complexity.
        difference_type getIndex() noexcept
        {
            return _rBuf_const_iterator<_rBuf>::m_logicalIndex;
        }
    };


    /// @brief a LegacyForwardIterator that internally holds a raw pointer. 
    template<typename T>
    class InitializerIterator {

public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        InitializerIterator (value_type* ptr = nullptr) : m_ptr(ptr)
        {
        }

        reference operator*() const noexcept { 
            return *m_ptr;
        }

        pointer operator->() noexcept
        {
            return m_ptr;
        }

        InitializerIterator operator++() noexcept
        {
            ++m_ptr;
            return *this;
        }

        InitializerIterator operator++(int) noexcept
        {
            InitializerIterator temp = *this;
            ++(*this);
            return temp;
        }

        InitializerIterator operator+(int count) noexcept
        {
            InitializerIterator temp = *this;
            for (auto i = 0; i < count; i++)
            {
                 temp.m_ptr++;
            }

            return temp;
        }

        friend bool operator==(const InitializerIterator& left, const InitializerIterator& right) noexcept
        {
            return left.m_ptr == right.m_ptr;
        }

        friend bool operator !=(const InitializerIterator& left, const InitializerIterator& right) noexcept
        {
            return left.m_ptr != right.m_ptr;
        }

private:
        T* m_ptr;
    };

    using init_iterator = InitializerIterator<T>;

    using iterator = _rBuf_iterator<ring_buffer<T>>;
    using const_iterator = _rBuf_const_iterator<ring_buffer<T>>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

//============================
// End of iterators
//============================

    /// @brief Default constructor.
    /// @post this->empty() == true.
    /// @throw Can throw std::bad_alloc if there is not enough memory available for allocation.
    /// @exception If any exception is thrown the buffer will be in a valid but unexpected state. (Basic exception guarantee).
    /// @details Constant complexity.
    ring_buffer() : ring_buffer(allocator_type())
    {
    }

    /// @brief Constructs the container with a custom allocator.
    /// @param alloc Custom allocator for the buffer.
    /// @post this->empty() == true.
    /// @throw Can throw std::bad_alloc if there is not enough memory available for allocation, or some exception from T's constructor.
    /// @exception If any exception is thrown the buffer will be in a valid but unexpected state. (Basic exception guarantee).
    /// @details Constant complexity.
    explicit ring_buffer(const allocator_type& alloc) : ring_buffer_base(alloc, allocBuffer), m_headIndex(0), m_tailIndex(0)
    {
    }

    /// @brief Constructs the buffer to a given size with given values and optionally a custom allocator.
    /// @param size Amount of elements to be initialized in the buffer.
    /// @param val Reference to a value which the elements are initialized to.
    /// @param alloc Custom allocator.
    /// @pre T needs to satisfy CopyInsertable.
    /// @post std::distance(begin(), end()) == size().
    /// @note Allocates memory for count + allocBuffer elements.
    /// @throw Can throw std::bad_alloc if there is not enough memory available for allocation, or some exception from T's constructor.
    /// @exception If any exception is thrown the buffer will be in a valid but unexpected state. (Basic exception guarantee).
    /// @details Linear complexity in relation to amount of constructed elements (O(n)).
    ring_buffer(size_type count, const_reference val, const allocator_type& alloc = allocator_type()) : ring_buffer_base(alloc, count + allocBuffer), m_headIndex(count), m_tailIndex(0)
    {
        std::uninitialized_fill_n(init_begin(), count, val);
    }
    
    /// @brief Custom constructor. Initializes a buffer to with count amount of default constructed value_type elements.
    /// @param count Capacity of the buffer.
    /// @pre T must satisfy DefaultInsertable.
    /// @throw Can throw std::bad_alloc if there is not enough memory available for allocation, or some exception from T's constructor.
    /// @exception If any exception is thrown the buffer will be in a valid but unexpected state. (Basic exception guarantee).
    /// @details Linear complexity in relation to count (O(n)).
    explicit ring_buffer(size_type count, const allocator_type& alloc = allocator_type()) : ring_buffer_base(alloc, count + allocBuffer), m_headIndex(count), m_tailIndex(0)
    {
        init_iterator first = init_begin();
        init_iterator current = init_begin();

        try
        {
            for (size_t i = 0; i < count; i++)
            {
                std::allocator_traits<allocator_type>::construct(m_allocator, &*current);
                current++;
            }
        }
        catch (...)
        {
            for (; first != current; first++)
            {
                std::allocator_traits<allocator_type>::destroy(m_allocator, &*first);
            }
            
            m_headIndex = 0;

            throw;
        }
    }

    /// @brief Construct the buffer from range [begin,end).
    /// @param beginIt Iterator to first element of range.
    /// @param endIt Iterator pointing to past-the-last element of range.
    /// @pre valye_type must satisfy CopyInsertable. InputIt must be deferencable to value_type, and incrementing rangeBegin (repeatedly) must reach rangeEnd. Otherwise behaviour is undefined.
    /// @throw Can throw std::bad_alloc if there is not enough memory available for allocation, or something from T's constructor.
    /// @exception If any exception is thrown the buffer will be in a valid but unexpected state. (Basic exception guarantee).
    /// @details Linear complexity in relation to the size of the range (O(n)).
    /// @note Behavior is undefined if elements in range are not valid.
    template<typename InputIt,typename = std::enable_if_t<std::is_convertible<typename std::iterator_traits<InputIt>::value_type,value_type>::value>>
    ring_buffer(InputIt beginIt, InputIt endIt, const allocator_type& alloc = allocator_type())
        : ring_buffer_base(alloc, std::distance<InputIt>(beginIt,endIt) + allocBuffer), m_headIndex(std::distance<InputIt>(beginIt, endIt)), m_tailIndex(0)
    {
        std::uninitialized_copy(beginIt, endIt, init_begin());
    }

    /// @brief Initializer list contructor.
    /// @param init Initializer list to initialize the buffer from.
    /// @pre T must satisfy CopyInsertable.
    /// @throw Can throw std::bad_alloc if there is not enough memory for allocation, or some exception from T's constructor..
    /// @note Allocates memory for 2 extra elements.
    /// @details Linear complexity in relation to initializer list size (O(n)).
    ring_buffer(std::initializer_list<T> init) : ring_buffer(init.begin(),init.end())
    {
    }

    /// @brief Copy constructor.
    /// @param rhs Reference to a RingBuffer to create a copy from.
    /// @pre T must meet CopyInsertable.
    /// @post this == ring_buffer(rhs).
    /// @throw Can throw std::bad_alloc if there is not enough memory for memory allocation, or something from T's constructor.
    /// @except If any exception is thrown, invariants are preserved.(Basic Exception Guarantee).
    /// @details Linear complexity in relation to buffer size.
    ring_buffer(const ring_buffer& rhs) 
    : m_headIndex(rhs.m_headIndex), m_tailIndex(rhs.m_tailIndex), ring_buffer_base(std::allocator_traits<allocator_type>::select_on_container_copy_construction(rhs.m_allocator), rhs.capacity())
    {
        std::uninitialized_copy(rhs.begin(), rhs.end(), init_begin());
    }

    /// @brief Copy constructor with custom allocator.
    /// @param rhs Reference to a RingBuffer to create a copy from.
    /// @param alloc Allocator for the new buffer.
    /// @pre T must meet CopyInsertable.
    /// @post this == ring_buffer(rhs) but with a different allocator.
    /// @throw Might throw std::bad_alloc if there is not enough memory for memory allocation, or something from value_types constructor.
    /// @except If any exception is thrown, invariants are preserved.(Basic Exception Guarantee).
    /// @details Linear complexity in relation to buffer size.
    ring_buffer(const ring_buffer& rhs, const allocator_type& alloc) : ring_buffer_base(alloc, rhs.m_capacity) m_headIndex(rhs.m_headIndex), m_tailIndex(rhs.m_tailIndex)
    {
        std::uninitialized_copy(rhs.begin(), rhs.end(), init_begin());
    }

    /// @brief Move constructor.
    /// @param other Rvalue reference to other buffer.
    /// @details Constant complexity.
    ring_buffer(ring_buffer&& other) noexcept : ring_buffer_base(std::forward<ring_buffer_base>(other)), m_headIndex(other.m_headIndex), m_tailIndex(other.m_tailIndex)
    {
        other.m_headIndex = 0;
        other.m_tailIndex = 0;
    }

    /// @brief Move constructor with different allocator.
    /// @param other Rvalue reference to other buffer.
    /// @param alloc Allocator for the new ring buffer.
    /// @details Linear complexity in relation to buffer size.
    ring_buffer(ring_buffer&& other, const allocator_type& alloc) : ring_buffer_base(alloc, other.m_capacity), m_headIndex(other.m_headIndex), m_tailIndex(other.m_tailIndex)
    {
        
        init_iterator first = init_begin();
        init_iterator current = init_begin();

        try
        {
            for (size_t i = 0; i < other.size(); i++)
            {
                std::allocator_traits<allocator_type>::construct(m_allocator, &*current, std::move(other[i]));
                current++;
            }
        }
        catch (...)
        {
            for (; first != current; first++)
            {
                std::allocator_traits<allocator_type>::destroy(m_allocator, &*first);
            }
            m_headIndex = 0;
            m_tailIndex = 0;
            throw;
        }

        other.m_headIndex = 0;
        other.m_tailIndex = 0;
    }

    /// Destructor.
    ~ring_buffer()
    {
        // Calls destructor for each element in the buffer.
        for_each(begin(), end(), [this](T& elem) { std::allocator_traits<allocator_type>::destroy(m_allocator, &elem); });
    }

    /// @brief Inserts an element to the buffer.
    /// @param pos Iterator where the the element should be inserted. 
    /// @param value Value to insert.
    /// @return Iterator that points to the inserted element.
    /// @note Internally calls insertBase.
    /// @throw Might throw std::bad_alloc, or something from T's move/copy constructor.
    /// @exception  If any exception is thrown, invariants are retained. (Basic Exception guarantee).
    /// @details Linear Complexity in relation to amount of inserted elements.
    iterator insert(const_iterator pos, const value_type& value)
    {
        return insertBase(pos, 1, value);
    }

    /// @brief Inserts an element to the buffer.
    /// @param pos Iterator where the the element should be inserted
    /// @param value Value to insert.
    /// @return Iterator that pos to the inserted element.
    /// @note Internally calls insertBase.
    /// @throw Might throw std::bad_alloc, or something from T's move/copy constructor.
    /// @exception  If any exception is thrown, invariants are retained. (Basic Exception guarantee).
    /// @details Linear Complexity in relation to amount of inserted elements.
    iterator insert(const_iterator pos, value_type&& value)
    {
        return insertBase(pos, 1, std::move(value));
    }

    /// @brief Inserts an element to the buffer.
    /// @param pos Iterator where the the element should be inserted
    /// @param value Value to insert. T must meet the requirements of CopyInsertable.
    /// @return Iterator that pos to the inserted element.
    /// @note Internally calls insertBase.
    /// @throw Might throw std::bad_alloc, or something from T's move/copy constructor.
    /// @exception  If any exception is thrown, invariants are retained. (Basic Exception guarantee).
    /// @details Linear Complexity in relation to amount of inserted elements.
    iterator insert(const_iterator pos, const size_type amount, const value_type& value)
    {
        return insertBase(pos, amount, value);
    }

    /// @brief Inserts a range of elements into the buffer to a specific position.
    /// @tparam InputIt Type of iterator for the range.
    /// @param pos Iterator to the position where range will be inserted to.
    /// @param sourceBegin Iterator to first element of the range.
    /// @param sourceEnd Iterator past the last element of the range.
    /// @return Returns an iterator to an element in the buffer which is copy of the first element in the range.
    /// @note Internally calls insertRangeBase.
    /// @pre pos must be a valid dereferenceable iterator within the container. Iterators must point to elements that are implicitly convertible to value_type. Otherwise behavior is undefined.
    /// @throw Can throw std::bad_alloc and something from value_types constructor. 
    /// @exception If any exceptiong is thrown, invariants are retained (Basic Excpetion guarantee)
    /// @details Amortized linear complexity in relation to range size.
    template <typename InputIt>
    iterator insert(const_iterator pos, InputIt sourceBegin, InputIt sourceEnd)
    {
        return insertRangeBase(pos, sourceBegin, sourceEnd);
    }

    /// @brief Inserts initializer list into buffer to a specific position.
    /// @param pos Iterator where the list will be inserted.
    /// @param list Initiliazer list to insert.
    /// @pre pos must be a valid dereferenceable iterator within the container. Otherwise behavior is undefined.
    /// @return Returns Iterator to the first element inserted, or the element pointed by pos if the initializer list was empty.
    /// @note Internally calls insertRangeBase.
    /// @throw Can throw std::bad_alloc and something from value_types constructor.
    /// @exception If any exceptiong is thrown, invariants are retained (Basic Excpetion guarantee)
    /// @details Amortized linear complexity in relation to range size.
    iterator insert(const_iterator pos, std::initializer_list<T> list)
    {
        return insertRangeBase(pos, list.begin(), list.end());
    }

    /// @brief Construct an element in place from arguments.
    /// @param pos Iterator pointing to the element where the new element will be constructed.
    /// @param args Argument pack containing arguments to construct value_type element.
    /// @return Returns an iterator pointing to the element constructed from args.
    /// @pre T must meet EmplaceConstructible.
    /// @post returned iterator points at the element constructed from args.
    /// @throw Can throw std::bad_alloc if memory is allocated. Can also throw from T's constructor when constructing the element. Additionally, rotate can throw bad_alloc and if T does not provide a noexcept move semantics.
    /// @exception If any exception is thrown, invariants are preserved. (Basic exception guarantee).
    /// @details Amortized constant complexity.
    template<class... Args>
    iterator emplace(const_iterator pos, Args&&... args)
    {
        validateCapacity(1);

        iterator it(this, pos.getIndex());

        std::allocator_traits<allocator_type>::construct(m_allocator, &*end());
        std::move_backward(it, end() - 1, end());

        std::allocator_traits<allocator_type>::destroy(m_allocator, &*it);
        std::allocator_traits<allocator_type>::construct(m_allocator, &*it, std::forward<Args>(args)...);
        increment(m_headIndex);

        return it;
    }

    /// @brief Constructs an element in place to front from argumets.
    /// @param args Argument pack containing arguments to construct value_type element.
    /// @pre value_type is EmplaceConstructible from args.
    /// @throw Can throw std::bad_alloc if memory is allocated. Can also throw from T's constructor when constructing the element.
    /// @exception If any exception is thrown, invariants are preserved. (Basic exception guarantee).
    /// @details  Amortized constant complexity.
    template<class... Args>
    void emplace_front(Args&&... args)
    {
        validateCapacity(1);

        auto tempIndex = m_tailIndex;
        decrement(tempIndex);
        
        std::allocator_traits<allocator_type>::construct(m_allocator, &m_data[tempIndex], std::forward<Args>(args)...);
        m_tailIndex = tempIndex;
    }

    /// @brief Constructs an element in place to front from argumets.
    /// @param args Argument pack containing arguments to construct value_type element.
    /// @pre value_type is EmplaceConstructible from args.
    /// @throw Can throw std::bad_alloc if memory is allocated. Can also throw from T's constructor when constructing the element.
    /// @exception If any exception is thrown, invariants are preserved. (Basic exception guarantee).
    /// @details Amortized constant complexity.
    template<class... Args>
    void emplace_back(Args&&... args)
    {
        validateCapacity(1);

        std::allocator_traits<allocator_type>::construct(m_allocator, &*end(), std::forward<Args>(args)...);
        increment(m_headIndex);
    }

    /// @brief Erase an element at a given position.
    /// @param pos Pointer to the element to be erased.
    /// @pre value_type must be nothrow-MoveConstructible. pos must be a valid dereferenceable iterator within the container. Otherwise behavior is undefined.
    /// @return Returns an iterator that was immediately following the ereased element. If the erased element was last in the buffer, returns a pointer to end().
    /// @exception If value_type is nothrow_move_constructible and nothrow_move_assignable function is noexcept. Otherwise provides no exception guarantee at all.
    /// @details Linear Complexity in relation to distance of end buffer from the target element.
    iterator erase(const_iterator pos)
    {
        return eraseBase(pos, pos + 1);
    }

    /// @brief Erase the specified elements from the container according to the range [first,last). Might destroy or move assign to the elements depending if last == end(). If last == end(), elements in [first,last) are destroyed.
    /// @param first iterator to the first element to erase.
    /// @param last iterator past the last element to erase.
    /// @pre First and last must be valid iterators to *this.
    /// @return Returns an iterator to the element that was immediately following the last erased elements. If last == end(), then new end() is returned.
    /// @throw Possibly throws from value_types move/copy assignment operator if last != end().
    /// @exception If value_type is nothrow_move_constructible and nothrow_move_assignable function is noexcept. Otherwise provides no exception guarantee at all.
    /// @details Linear Complexity in relation to size of the range, and then linear in remaining elements after the erased range.
    iterator erase(const_iterator first, const_iterator last)
    {
        return eraseBase(first, last);
    }

    /// @brief Destroys all elements in a buffer. Does not modify capacity.
    /// @post All existing references, pointers and iterators are to be considered invalid.
    /// @details Linear complexity in relation to size of the buffer.
    void clear() noexcept
    {
        for(; m_tailIndex < m_headIndex ; ++m_tailIndex)
        {
            std::allocator_traits<allocator_type>::destroy(m_allocator, &m_data[m_tailIndex]);
        }

        m_headIndex = 0;
        m_tailIndex = 0;
    }

    /// @brief Replaces the elements in the buffer with copy of [sourceBegin, sourceEnd)
    /// @param sourceBegin Iterator to beginning of the range.
    /// @param sourceEnd Past the end iterator of the range.
    /// @pre value_type is CopyInsertable and elements of [sourceBegin, sourceEnd) are not in *this. InputIt must be dereferenceable to value_type, and incrementing sourceBegin (repeatedly) must reach sourceEnd. Otherwise behaviour is undefined.
    /// @post All existing references, pointers and iterators are to be considered invalid.
    /// @throw Can throw std::bad_alloc or from value_types constructor. 
    /// @exception If any exception is throw, invariants are preserved (Basic Exception Guarantee).
    /// @details Linear Complexity in relation to ranges size (O(n * 2) from linear clear and linear construction).
    /// @note Destroys all elements in the buffer before constructing the assigned elements.
    template <typename InputIt>
    void assign(InputIt sourceBegin, InputIt sourceEnd)
    {
        clear();
        for (; sourceBegin != sourceEnd; ++sourceBegin)
        {
            push_back(*sourceBegin);
        }
    }

    /// @brief Replaces the elements in the buffer.
    /// @param list Initializer list containing the elements to replace the existing ones.
    /// @pre value_type is CopyInsertable. List does not contain elements of *this.
    /// @post All existing references, pointers and iterators are to be considered invalid.
    /// @throw Can throw std::bad_alloc or from value_types constructor.
    /// @exception If any exception is throw, invariants are preserved (Basic Exception Guarantee).
    /// @details Linear Complexity in relation to ranges size (O(n * 2) from linear clear and linear construction).
    /// @note Destroys all elements in the buffer before constructing the assigned elements.
    void assign(std::initializer_list<T> list)
    {
        assign(list.begin(), list.end());
    }

    /// @brief Replaces the elements in the buffer with given value.
    /// @param amount Size of the buffer after the assignment.
    /// @param value Value of all elements after the assignment.
    /// @pre value_type is CopyInsertable.
    /// @post All existing references, pointers and iterators are to be considered invalid.
    /// @throw Can throw std::bad_alloc or from value_types constructor.
    /// @exception If any exception is throw, invariants are preserved (Basic Exception Guarantee).
    /// @details Linear Complexity in relation to ranges size (O(n * 2) from linear clear and linear construction).
    /// @note Destroys all elements in the buffer before constructing the assigned elements.
    void assign(const size_type amount, value_type value)
    {
        clear();
        for(size_t i = 0; i < amount; i++)
        {
            push_back(value);
        }
    }

    /// @brief Copy assignment operator.
    /// @param other Ringbuffer to be copied.
    /// @return Returns reference to the left hand side RungBuffer after swap.
    /// @post *this == other.
    /// @details Constant complexity.
    /// @throw Can throw std::bad_alloc or something from value_types constructor.
    /// @exception If any exception is thrown, this function has no effect (Strong Exception Guarantee).
    /// @details Linear complexity in relation to buffer size.
    ring_buffer& operator=(const ring_buffer& other)
    {

        if (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value && m_allocator != other.m_allocator)
        {
            clear();
            std::allocator_traits<allocator_type>::deallocate(m_allocator,m_data, m_capacity);

            m_allocator = other.m_allocator;
            m_data = std::allocator_traits<allocator_type>::allocate(m_allocator, other.m_capacity);
            m_capacity = other.m_capacity;
            m_headIndex = other.m_headIndex;
            m_tailIndex = other.m_tailIndex;

            for (size_type i = 0; i < other.size(), i++)
            {
                std::iterator_traits<allocator_type>::construct(m_allocator, &this->operator[](i), other[i])
            }
        } 
        else
        {
            if (size() >= other.size())
            {
                std::copy(other.begin(), other.end(), begin());
                for (size_type i = other.size(); i < size(); i++)
                {
                    std::allocator_traits<allocator_type>::destroy(m_allocator, &this->operator[](i))
                }
                resize(other.size());
            }
            else
            {
                std::copy(other.begin(), other.begin() + size(), begin());
                for (size_type i = size(); i < other.size(); i++)
                {
                    push_back(other[i]);
                }
            }
        }

        return *this;
    }

    /// @brief Move assignment operator.
    /// @param other Rvalue ref to other buffer.
    /// @pre value_type is MoveConstructible.
    /// @post *this has values other had before the assignment.
    /// @return Reference to the buffer to move from.
    /// @exception If value_type is not MoveConstructible, and value_type provides a throwing CopyConstructor is used this function has undefined behaviour.
    /// @details Constant complexity.
    ring_buffer& operator=(ring_buffer&& other) noexcept
    {
        ring_buffer copy(std::move(other));
        copy.swap(*this);
        return *this;
    }

    /// @brief Initializer list assign operator. 
    /// @param init Initializer list to assign to the buffer.
    /// @return Returns a reference to the buffer.
    /// @pre T is CopyInsertable.
    /// @post All existing iterators are invalidated. 
    /// @note Internally calls assign(), which destroys all elements before CopyInserting from initializer list.
    /// @details Linear complexity in relation to amount of existing elements and size of initializer list.
    ring_buffer& operator=(std::initializer_list<T> init)
    {
        assign(init);
        return *this;
    }

    /// @brief Index operator.
    /// @param logicalIndex Index of the element. If LogicalIndex >= size(), this function has undefined behavriour.
    /// @details Constant complexity.
    /// @note The operator acts as interface that hides the physical memory layout from the user. Logical index neeeds to be added to internal tail index to get actual element address. 
    /// @return Returns a reference to the element.
    reference operator[](const size_type logicalIndex) noexcept
    {
        return m_data[(m_tailIndex + logicalIndex) % m_capacity];
    }

    /// @brief Index operator.
    /// @param logicalIndex Index of the element used to access n:th element of the buffer.
    /// @details Constant complexity
    /// @note The operator acts as interface that hides the physical memory layout from the user. Logical index neeeds to be added to internal tail index to get actual element address.
    /// @return Returns a const reference the the element ad logicalIndex.
    const_reference operator[](const size_type logicalIndex) const noexcept
    {
        return m_data[(m_tailIndex + logicalIndex) % m_capacity];
    }

    /// @brief Get a specific element of the buffer with bounds checking.
    /// @param logicalIndex Index of the element.
    /// @return Returns a reference the the element at index.
    /// @throw Throws std::out_of_range if index is larger or equal to buffers size.
    /// @exception If any exceptions is thrown this function has no effect (Strong exception guarantee).
    /// @details Constant complexity.
    reference at(size_type logicalIndex)
    {
        if(logicalIndex >= size())
        {
            throw std::out_of_range("Index is out of range");
        }

        auto index = m_tailIndex + logicalIndex;

        if(m_capacity <= index)
        {
            index -= m_capacity;
        }
        return m_data[index];
    }

    /// @brief Get a specific element of the buffer.
    /// @param logicalIndex Index of the element.
    /// @return Returns a const reference the the element at index.
    /// @throw Throws std::out_of_range if index is larger or equal to buffers size.
    /// @exception If any exceptions is thrown this function has no effect (Strong exception guarantee).
    /// @details Constant complexity.
    const_reference at(size_type logicalIndex) const
    {
        if(logicalIndex >= size())
        {
            throw std::out_of_range("Index is out of range.");
        }

        auto index(m_tailIndex + logicalIndex);
        if(m_capacity <= index)
        {
            index -= m_capacity;
        }
        return m_data[index];
    }

    /// @brief Member swap implementation. Swaps RingBuffers member to member.
    /// @param other Reference to a ring_buffer to swap with.
    /// @details Constant complexity.
    /// @note Requires allocator_type to return true_type from propagate_on_container_swap, otherwise memory operations 
    void swap(ring_buffer& other) noexcept
    {
        using std::swap;
        if (std::allocator_traits<allocator_type>::propagate_on_container_swap::value)
        {
            swap(m_allocator, other.m_allocator);
        }

        swap(m_data, other.m_data);
        swap(m_headIndex, other.m_headIndex);
        swap(m_tailIndex, other.m_tailIndex);
        swap(m_capacity, other.m_capacity);
    }

    /// @brief Friend swap.
    /// @param a Swap candidate.
    /// @param b Swap candidate.
    /// @details Constant complexity.
    friend void swap(ring_buffer& a, ring_buffer& b) noexcept
    {
        a.swap(b);
    }

	/// @brief Sorts ringbuffer so that logical tail matches the first element in physical memory.
    /// @return Returns a pointer to the first element.
    /// @pre T must meet MoveInsertable, or CopyInsertable.
    /// @post &this[0] == m_data.
    /// @throw Can throw std::bad_alloc.
    /// @exception If T's Move (or copy) constructor is not noexcept and throws, behaviour is undefined. Otherwise if exceptions are thrown (std::bad_alloc) this function has no effect (Strong exception guarantee).
    /// @note Invalidates all existing pointers and references.
    /// @details Linear complexity in relation to buffer size.
    pointer data()
    {
        if(!size())
        {
            return m_data;
        }

        // Data pointer for "do stuff and swap" idiom to provide strong excepion guarantee.
        auto tempData = std::allocator_traits<allocator_type>::allocate(m_allocator, m_capacity);

        for (size_t i = 0; i < size(); i++)
        {
            std::allocator_traits<allocator_type>::construct(m_allocator, tempData + i, std::move(this->operator[](i)));
            std::allocator_traits<allocator_type>::destroy(m_allocator, &this->operator[](i));
        }

        std::allocator_traits<allocator_type>::deallocate(m_allocator, m_data, size());

        // If memory was allocated, the buffer matches beginning of physical memory.
        m_headIndex = size();
        m_tailIndex = 0;

        // Assings the data from temp to original buffer. The resources from temp will be released when function goes out of scope.
        std::swap(tempData, m_data);

        return m_data;
    }

    /// @brief Gets the size of the container.
    /// @return Size of buffer.
    /// @details Constant complexity.
    size_type size() const noexcept
    {
        if(m_headIndex < m_tailIndex)
        {
            return m_headIndex + m_capacity - m_tailIndex;
        }

        return m_headIndex - m_tailIndex;
    }

    /// @brief Gets the theoretical maximum size of the container.
    /// @return Maximum size of the buffer.
    /// @details Constant complexity.
    size_type max_size() const noexcept
    {
        constexpr auto maxSize = std::numeric_limits<std::size_t>::max();
        return maxSize / sizeof(T);
    }

    /// @brief Capacity getter.
    /// @return m_capacity Returns how many elements have been allocated for the buffers use. 
    /// @details Constant complexity.
    size_type capacity() const noexcept
    {
        return m_capacity;
    }

    /// @brief Allocator getter.
    /// @return Return the allocator used by the container.
    /// @details Constant complexity.
    allocator_type get_allocator() const noexcept
    {
        return m_allocator;
    }

    /// @brief Check if buffer is empty
    /// @return True if buffer is empty
    /// @details Constant complexity.
    bool empty() const noexcept
    {
        return m_tailIndex == m_headIndex;
    }

    /// @brief Allocates memory and copies the existing buffer to the new memory location. Can be used to increase or decrease capacity.
    /// @throw Throws std::bad_alloc if there is not enough memory for allocation. Throws std::bad_array_new_lenght if std::numeric_limits<std::size_t>::max() / sizeof(T) < newsize.
    /// @param newCapacity Amount of memory to allocate. If newCapacity is less than or equal to m_capacity, function does nothing.
    /// @param enableShrink True to enable reserve to reduce the capacity, to a minimum of size() +2.
    /// @pre T must meet MoveInsertable.
    /// @throw Can throw std::bad_alloc. 
    /// @exception If T's move (or copy if T has no move) constructor throws, behaviour is undefined. Otherwise Stong Exception Guarantee.
    /// @note All references, pointers and iterators are invalidated. If memory is allocated, the memory layout is rotated so that first element matches the beginning of physical memory.
    /// @details Linear complexity in relation to size of the buffer (O(n)).
    void reserve(size_type newCapacity, bool enableShrink = false)
    {
        if (enableShrink)
        {
            if (newCapacity < size()) return;
        }
        else
        {
            if (newCapacity <= m_capacity) return;
        }

        // Temp for "move and swap" idiom to provide exception safety.
        auto temp = ring_buffer<T, allocator_type>();
        temp.m_capacity = newCapacity;
        temp.m_data = std::allocator_traits<allocator_type>::allocate(temp.m_allocator, newCapacity);

        for(size_t i = 0; i < this->size(); i++)
        {
            std::allocator_traits<allocator_type>::construct(temp.m_allocator, temp.m_data + i, std::move(this->operator[](i)));
        }
        temp.m_headIndex = this->size();

        this->swap(temp);
    }

    /// @brief Inserts an element in the back of the buffer. If buffer would get full after the operation, allocates more memory.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @param val Element to insert.  Needs to be CopyConstructible.
    /// @note All iterators are invalidated. If allocation happens, all pointers and references are invalidated.
    /// @exception If the move/copy constructor of value_type throws, behaviour is undefined. Otherwise in case of exception this function retains invariants (Basic Exception Guarantee).
    /// @details Constant complexity.
    void push_front(const value_type& val)
    {
        validateCapacity(1);

        // Decrement temporary index in case constructor throws to retain invariants (elements of the buffer are always initialized).
        auto newIndex = m_tailIndex;
        decrement(newIndex);
        std::allocator_traits<allocator_type>::construct(m_allocator, m_data + newIndex, val);
        m_tailIndex = newIndex;
    }

    /// @brief Inserts an element in the back of the buffer by move if move constructor is provided by value_type. If buffer would get full after the operation, allocates more memory.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @param val Rvalue reference to the element to insert.
    /// @pre value_type needs to satisfy MoveInsertable or CopyInsertable.
    /// @note All iterators are invalidated. If allocation happens, all pointers and references are invalidated.
    /// @exception If the move/copy constructor of value_type throws, behaviour is undefined. Otherwise in case of any exception this function retains invariants (Basic Exception Guarantee).
    /// @details Constant complexity.
    void push_front(value_type&& val)
    {
        validateCapacity(1);

        // Decrement temporary index incase constructor throws to retain invariants (elements of the buffer are always initialized).
        auto newIndex = m_tailIndex;
        decrement(newIndex);
        std::allocator_traits<allocator_type>::construct(m_allocator, m_data + newIndex, std::forward<value_type>(val));
        m_tailIndex = newIndex;
    }

    /// @brief Inserts an element in the back of the buffer. If buffer would get full after the operation, allocates more memory.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @param val Value of type T to be appended.
    /// @note Allocates memory before the insertion if the buffer would be full after the operation.
    /// @throw Can throw std::bad_alloc.
    /// @exception If the copy constructor of value_type throws, behaviour is undefined. Otherwise in case of exception this function retains invariants (Basic Exception Guarantee).
    /// @pre value_type needs to satisfy CopyInsertable.
    /// @post If more memory is allocated all pointers, iterators and references are invalidated.
    /// @details Constant complexity.
    void push_back(const value_type& val)
    {
        validateCapacity(1);

        std::allocator_traits<allocator_type>::construct(m_allocator, m_data + m_headIndex, val);
        increment(m_headIndex);
    }

    /// @brief Inserts an element in the back of the buffer by move if move constructor is provided for value_type. If buffer would get full after the operation, allocates more memory.
    /// @param val Rvalue reference to the value to be appended.
    /// @note Allocates memory before the insertion if the buffer would be full after the operation.
    /// @throw Can throw std::bad_alloc if more memory is allocated.
    /// @exception If the move/copy constructor of value_type throws, behaviour is undefined. Otherwise in case of any exception this function retains invariants (Basic Exception Guarantee).
    /// @pre value_type needs to satisfy MoveInsertable or CopyInsertable.
    /// @post If more memory is allocated all pointers, iterators and references are invalidated.
    /// @details Constant complexity.
    void push_back(value_type&& val)
    {
        validateCapacity(1);
    
        std::allocator_traits<allocator_type>::construct(m_allocator, m_data + m_headIndex, std::forward<value_type>(val));
        increment(m_headIndex);
    }

    /// @brief Remove the first element in the buffer.
    /// @pre Buffers size > 0, otherwise behaviour is undefined.
    /// @post All iterators, pointers and references are invalidated.
    /// @details Constant complexity.
    void pop_front() noexcept
    {
        std::allocator_traits<allocator_type>::destroy(m_allocator, m_data + m_tailIndex);
        increment(m_tailIndex);
    }

    /// @brief Erase an element from the logical back of the buffer.
    /// @pre Buffers size > 0, otherwise behaviour is undefined.
    /// @post All pointers and references are invalidated. Iterators persist except end() - 1 iterator is invalidated (it becomes new past-the-last iterator).
    /// @details Constant complexity.
    void pop_back() noexcept
    {
        decrement(m_headIndex);
        std::allocator_traits<allocator_type>::destroy(m_allocator, m_data + m_headIndex);

    }

    /// @brief Releases unused allocated memory. 
    /// @pre T must satisfy MoveConstructible or CopyConstructible.
    /// @post m_capacity == size() + allocBuffer.
    /// @note Reduces capacity by allocating a smaller memory area and moving the elements. Shrinking the buffer invalidates all pointers, iterators and references.
    /// @throw Might throw std::bad_alloc if memory allocation fails.
    /// @exception If T's move (or copy) constructor can and does throw, behaviour is undefined. If any other exception is thrown (bad_alloc) this function has no effect (Strong exception guarantee).
    /// @details Linear complexity in relation to size of the buffer.
    void shrink_to_fit()
    {
        reserve(size() + allocBuffer, true);
    }

//===========================================================
//  std::queue adaptor functions
//===========================================================

    /// @brief Returns a reference to the first element in the buffer. Behaviour is undefined for empty buffer.
    /// @return Reference to the first element.
    /// @details Constant complexity.
    reference front() noexcept
    {
        return m_data[m_tailIndex];
    }

    /// @brief Returns a reference to the first element in the buffer. Behaviour is undefined for empty buffer.
    /// @return const_reference to the first element.
    /// @details Constant complexity.
    const_reference front() const noexcept
    {
        return m_data[m_tailIndex];
    }

    /// @brief Returns a reference to the last element in the buffer. Behaviour is undefined for empty buffer.
    /// @return Reference to the last element in the buffer.
    /// @details Constant complexity.
    reference back() noexcept
    {
        // Since head points to next-to-last element, it needs to be decremented once to get the correct element. 
        // If the index is at the beginning border of the allocated memory area it needs to be wrapped around to the end. 
        if (m_headIndex == 0)
        {
            return m_data[m_capacity - 1];
        }
        return m_data[m_headIndex-1];
    }

    /// @brief Returns a const-reference to the last element in the buffer. Behaviour is undefined for empty buffer.
    /// @return const_reference to the last element in the buffer.
    /// @details Constant complexity.
    const_reference back() const noexcept
    {
        // Since head points to next-to-last element, it needs to be decremented once to get the correct element. 
        // If the index is at the beginning border of the allocated memory area it needs to be wrapped around to the end. 
        if (m_headIndex == 0)
        {
            return m_data[m_capacity - 1];
        }
        return m_data[m_headIndex-1];
    }

    /// @brief Construct iterator at begin.
    /// @return Iterator pointing to first element.
    /// @details Constant complexity. Iterator is invalid if the buffer is empty (dereferencing points to uninitialized memory.).
    iterator begin() noexcept
    {
        return iterator(this, 0);
    }

    /// @brief Construct const_iterator at begin.
    /// @return Const_iterator pointing to first element.
    /// @details Constant complexity. Iterator is invalid if the buffer is empty (dereferencing points to uninitialized memory.).
    const_iterator begin() const noexcept
    {
        return const_iterator(this, 0);
    }

    /// @brief Construct iterator at end.
    /// @return Iterator pointing past last element.
    /// @details Constant complexity.
    iterator end() noexcept
    {
        return iterator(this, size());
    }

    /// @brief Construct const_iterator at end.
    /// @return Const_iterator pointing past last element.
    /// @details Constant complexity.
    const_iterator end() const noexcept
    {
        return const_iterator(this, size());
    }

    /// @brief Construct const_iterator at begin.
    /// @return Const_iterator pointing to first element.
    /// @details Constant complexity.
    const_iterator cbegin() const noexcept
    {
        return const_iterator(this, 0);
    }

    /// @brief Construct const_iterator pointing to past the last element.
    /// @return Const_iterator pointing past last element.
    /// @details Constant complexity.
    const_iterator cend() const noexcept
    {
        return const_iterator(this, size());
    }

    /// @brief Get a reverse iterator pointing to the first element in reverse order (last element in normal order).
    /// @return reverse_iterator pointing to first element in reverse order.
    /// @details Constant complexity. Iterator is invalid if the buffer is empty (dereferencing points to uninitialized memory.).
    reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }

    /// @brief Get a const reverse iterator pointing to the first element in reverse order (last element in normal order).
    /// @return const_reverse_iterator pointing to the first element in reverse order.
    /// @details Constant complexity.
    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }

    /// @brief Get a const reverse iterator pointing to the first element in reverse order (last element in normal order).
    /// @details Constant complexity.
    /// @return const_reverse_iterator pointing to the first element in reverse order.
    const_reverse_iterator crbegin() const
    {
        return const_reverse_iterator(end());
    }

    /// @brief Get a reverse iterator pointing to one past the last element in reverse order (one before the first element in normal order).
    /// @details Constant complexity.
    /// @return reverse_iterator pointing to one past the last element in reverse order.
    reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }

    /// @brief Get a const reverse iterator pointing to one past the last element in reverse order (one before the first element in normal order).
    /// @details Constant complexity.
    /// @return const_reverse_iterator pointing to one past the last element in reverse order.
    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }

    /// @brief Get a const reverse iterator pointing to one past the last element in reverse order (one before the first element in normal order).
    /// @details Constant complexity.
    /// @return const_reverse_iterator pointing to one past the last element in reverse order.
    const_reverse_iterator crend() const
    {
        return const_reverse_iterator(begin());
    }


private:
    
    template<typename T>
    void swap(ring_buffer_base<T>& left, ring_buffer_base<T>& right) noexcept
    {
        std::swap(left.m_data,right.m_data);
        std::swap(left.m_capacity, right.m_capacity);
    }
     
    /// @brief Returns a ForwardIterator to the beginning of the allocated memory area, that is not invalid while size == 0. 
    /// @note Used to initialize elements with std::uninitialized_fill and other respective algorithms.
    init_iterator init_begin()
    {
        return init_iterator(m_data);
    }

    init_iterator init_begin(int count)
    {
        return init_iterator(m_data) + count;
    }

    /// @brief Reserves more memory if needed for an increase in size. If more memory is needed, allocates (capacity * 1.5) or if that is not enough (capacity * 1.5 + increase).
    /// @param increase Expected increase in size of the buffer, based on which memory is allocated.
    /// @details Linear complexity in relation to buffer size if more memory needs to be allocated, otherwise constant complexity.
    /// @exception May throw std::bad_alloc. If any exception is thrown this function does nothing. Strong exception guarantee.
    /// @note This function should be called before increasing the size of the buffer.
    void validateCapacity(size_t increase)
    {
        if (m_capacity > size() + increase + allocBuffer) return;

        auto enlargedCap = m_capacity / 2 + m_capacity;

        if (enlargedCap > size() + increase)
        {
            reserve(enlargedCap + allocBuffer);
            return;
        }
        
        // Special case where 1.5*m_capacity is not enough, allocate <increase> + the normal capacity increase.
        if(enlargedCap > 2)
        {
            reserve(enlargedCap + increase + allocBuffer);
        }
        // Special case where capacity is 0 / 1 and multiplying does not produce a change.
        else
        {
            reserve(2);
        }
    }


    /// @brief Shifts elements after posIndex by <amount> using std::memmove.
    /// @param posIndex Logical index to first empty element after shift.
    /// @param amount Amount of shift. Creates <amount> empty elements at posIndex. 
    /// @details Amortized linear complexity in relation to amount of elements between pos and head. Sometimes shifts the memory layout before operation that adds linear complexity in relation to buffer size.
    /// @throw data can throw bad_alloc, value_type construction can throw something, and memmove as well.
    /// @exception No exception safety guarantee.
    void byteShift(const size_t posIndex, const size_t amount)
    {
        // If head has reached over physical memory border, or there not enough room to memmove in one chunk, reset the memory layout.
        if( m_tailIndex > m_headIndex ||
            m_tailIndex + posIndex + amount >= m_capacity)
        {
            // Simplifies a lot of edge cases with a minor performance cost.
            data();
        }

        //Always copies towards head, could optimize by copying towards the end that is closer to pos. Keeping it simple for now.
        for (size_t i = 0; i < amount; i++)
        {
            std::allocator_traits<allocator_type>::construct(m_allocator, m_data + m_headIndex + i);
        }

        // "Shift" elements forward from the breaking point by amount.
        std::memmove(m_data + m_tailIndex + posIndex + amount, m_data + m_tailIndex + posIndex, (size() - posIndex) * sizeof(value_type));
        increment(m_headIndex, amount);
    }

    /// @brief Shift ekements after posIndex by <amount> using move / copy constructors.
    /// @param posIndex Logical index to first empty element after shift.
    /// @param amount Amount of shift. Creates <amount> empty elements at posIndex. 
    /// @details Linear complexity in relation to amount and end() - posIndex.
    void slowShift(const size_t posIndex, const size_t amount)
    {
        // Construct empty elements at the end.
        for (size_t i = 0; i < amount; i++)
        {
            std::allocator_traits<allocator_type>::construct(m_allocator, m_data + m_headIndex);
            increment(m_headIndex);
        }

        std::move_backward(begin() + posIndex, end() - amount, end());
    }

    /// @brief Base function for inserting elements from value.
    /// @tparam Value type of the buffer.
    /// @param pos Iterator pointing to the element where after insert new element will exist.
    /// @param amount Amount of elements to insert. If 0, function does nothing.
    /// @param value Universal reference of the value to insert.
    /// @return Returns iterator pointing to the first element inserted. If amount == 0, returns iterator to the element where insertion was supposed to happen.
    /// @throw Might throw std::bad_alloc from allocating memory and rotate(), or something from T's move/copy constructor.
    /// @exception  If any exception is thrown, invariants are retained. (Basic Exception guarantee).
    /// @details Linear Complexity in relation to amount of inserted elements times (O(n*2) from construction and rotate).
    template<typename InsertValue>
    iterator insertBase(const_iterator pos, const size_type amount, InsertValue&& value)
    {

        iterator it(this, pos.getIndex());
        if (!amount) return it;

        validateCapacity(amount);

        // Make room for new elements.
        if (std::is_trivially_copyable<value_type>::value)
        {
            byteShift(pos.getIndex(), amount);
        }
        else
        {
            slowShift(pos.getIndex(), amount);
        }

        // Assign elements
        for (size_type i = 0; i < amount; i++)
        {
            *(it + i) = std::forward<InsertValue>(value);
        }

        return it;
    }

    /// @brief Base function for inserting elements from a range of [rangeBegin, rangeEnd).
    /// @tparam InputIt type of the buffer.
    /// @param pos Iterator pointing to the element where after insert new element will exist.
    /// @param rangeBegin Iterator pointing to the first element of the range.
    /// @param rangeEnd Iterator pointing past the last element to be inserted.
    /// @return Returns iterator pointing to the first element inserted.
    /// @pre value_type must meet CopyInsertable. InputIt must be deferencable to value_type, and incrementing rangeBegin possibly multiple times should reach rangeEnd. Otherwise behaviour is undefined.
    /// @post Each iterator in [rangeBegin, rangeEnd) is dereferenced once.
    /// @throw Might throw std::bad_alloc from allocating memory and rotate(), or something from T's move/copy constructor.
    /// @exception  If any exception is thrown, invariants are retained. (Basic Exception guarantee).
    /// @details Linear Complexity in relation to amount of inserted elements (O(n*2) from element construction and rotation).
    template<typename InputIt>
    iterator insertRangeBase(const_iterator pos, InputIt rangeBegin, InputIt rangeEnd)
    {
        const auto amount = std::distance<InputIt>(rangeBegin, rangeEnd);
        validateCapacity(amount);

        const auto posIndex = pos.getIndex();
        iterator it(this, posIndex);
        
        // Use memmove to move elements to create space to insert.
        if (std::is_trivially_copyable<value_type>::value)
        {
            byteShift(pos.getIndex(), amount);
        }
        else
        {
            slowShift(pos.getIndex(), amount);
        }

        //Assign the elements to the new memory slots.
        for (size_t i = 0; i < amount; i++)
        {
            *(it + i) = *(rangeBegin + i);
        }

        return it;
    }

    /// @brief Base function for erasing elements from the buffer. Erases always as a range.
    /// @param first Iterator pointing to the first element of the range to erase.
    /// @param last Iterator pointing to past the last element to erase.
    /// @return Returns an iterator pointing to the element immediately after the erased elements.
    /// @pre First and last must be valid iterators to *this.
    /// @throw If value_type is not nothrow move assignable/constructible, might throw something from swap. 
    /// @exception If swap does not throw, function is noexcept. Otherwise provides no exception guarantee at all.
    iterator eraseBase(const_iterator first, const_iterator last)
    {
        iterator returnIt(this, first.getIndex());

        const auto diff = std::distance(first, last);

        if (diff > 0)
        {
            auto tempIt(returnIt);
            for (; tempIt + diff < end(); tempIt++)
            {
                std::swap(*(tempIt + diff), *tempIt);
            }

            for (; tempIt < end(); tempIt++)
            {
                std::allocator_traits<allocator_type>::destroy(m_allocator, &*(tempIt));
            }

            decrement(m_headIndex, diff);
        }
        return returnIt;
    }

    /// @brief Increment an index. The ringbuffer internally increments the head and tail index when adding elements.
    /// @param index The index to increment.
    /// @details Constant complexity.
    void increment(size_t& index) noexcept
    {
        ++index;
        // Wrap index around at end of physical memory area.
        if(index >= m_capacity)
        {
            index = 0;
        }
    }

    /// @brief Increments an index multiple times. The ringbuffer internally increments the head and tail index when adding elements.
    /// @param index Index to increment.
    /// @param times Amount of increments.
    /// @details Linear complexity in relation to size of times argument.
    void increment(size_t& index, size_t times) noexcept
    {
        while(times > 0)
        {
            increment(index);
            times--;
        }
    }

    /// @brief Decrements an index. The ringbuffer internally decrements the head and tail index when removing elements.
    /// @param index The index to decrement.
    /// @details Constant complexity.
    void decrement(size_t& index) noexcept
    {
        if(index == 0)
        {
            index = m_capacity - 1;
        }
        else
        {
            --index;
        }
    }
    
    /// @brief Decrements an index multiple times. The ringbuffer internally decrements the head and tail index when removing elements.
    /// @param index Index to decrement.
    /// @param times Amount of decrements.
    void decrement(size_t& index, size_t times) noexcept
    {
        while(times > 0)
        {
            decrement(index);
            times--;
        }
    }

    size_type m_headIndex; /*!< Index of the head. Index pointing to past the last element.*/
    size_type m_tailIndex; /*!< Index of the tail. Index to the first element in the buffer.*/

};



//===========================
// Non-member functions
//===========================

/// @brief Equality comparator. Compares buffers element-to-element.
/// @tparam T Value type
/// @tparam Alloc Optional custom allocator. Defaults to std::allocator<T>.
/// @param lhs Left hand side operand
/// @param rhs right hand side operand
/// @return returns true if the buffers elements compare equal.
template<typename T , typename Alloc>
inline bool operator==(const ring_buffer<T,Alloc>& lhs, const ring_buffer<T,Alloc>& rhs)
{
    if(lhs.size() != rhs.size())
    {
        return false;
    }

    for(size_t i = 0; i < lhs.size(); i++)
    {
        if(lhs[i] != rhs[i])
        {
            return false;
        }
    }

    return true;
}

/// @brief Not-equal comparator. Compares buffers element-to-element.
/// @tparam T Value type
/// @tparam Alloc Optional custom allocator. Defaults to std::allocator<T>.
/// @param lhs Left hand side operand.
/// @param rhs Right hand side operand.
/// @return returns True if any of the elements are not equal.
template<typename T,typename Alloc>
inline bool operator!=(const ring_buffer<T,Alloc>& lhs, const ring_buffer<T,Alloc>& rhs)
{
    return !(lhs == rhs);
}

#endif /*DYNAMIC_RINGBUFFER_HPP*/