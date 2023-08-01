#ifndef DYNAMIC_RINGBUFFER_HPP
#define DYNAMIC_RINGBUFFER_HPP

#include <memory>
#include <algorithm>
#include <limits>
#include <utility>
#include <stdexcept>


// Forward declaration of _rBuf_const_iterator.
template<class _rBuf>
class _rBuf_const_iterator;


/// @brief Dynamic Ringbuffer is a dynamically growing std::container with support for queue, stack and priority queue adaptor functionality. 
/// @tparam T Type of the elements.
/// @tparam Allocator Allocator used for (de)allocation and (de)construction. Defaults to std::allocator<T>
template<typename T, typename Allocator = std::allocator<T>> 
class ring_buffer
{

public:

    using value_type = T;
    using allocator_type = Allocator;
    using reference = T&;
    using const_reference= const T&;
    using pointer = T*;
    using const_pointer= const T*;

    using difference_type = ptrdiff_t;
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
        /// @param index Index representing the logical element of the.
        explicit _rBuf_const_iterator(const _rBuf* container, difference_type index) : m_container(container), m_logicalIndex(index) {}

        /// @brief Arrow operator.
        /// @return pointer.
        /// @details Constant complexity.
        pointer operator->() const
        {
            //TODO: check value initialization and bounds. Shhhh......
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
            //TODO:    check value initialization and "under" begin() decrement.
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

        // TODO
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
        /// @param other iterator to compare
        /// @return true if other is larger.
        /// @details Constant complexity.
        bool operator<(const _rBuf_const_iterator& other) const noexcept
        {
            return (m_logicalIndex < other.m_logicalIndex);
        }

        /// @brief Comparison operator > overload
        /// @param other iterator to compare
        /// @return true if other is smaller.
        /// @details Constant complexity.
        bool operator>(const _rBuf_const_iterator& other) const noexcept
        {
            //TODO:compatibility check
            return (other.m_logicalIndex < m_logicalIndex);
        }

        /// <summary>
        /// @details Constant complexity.
        bool operator<=(const _rBuf_const_iterator& other) const noexcept
        {
            //TODO:compatibility check
            return (!(other < m_logicalIndex));
        }

        /// <summary>
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
            //TODO: bounds checking and value-initialization.
            return const_cast<reference>(_rBuf_const_iterator<_rBuf>::operator*());
        }

        /// @brief Arrow operator. 
        /// @return Returns a pointer to the object the iterator is currently pointing to.
        /// @details Constant complexity.
        pointer operator->() const noexcept
        {
            //TODO: check value initialization and bounds.
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
            //TODO: compatibility check
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
            //TODO:compatibility check
            return (_rBuf_const_iterator<_rBuf>::m_logicalIndex < other._rBuf_const_iterator<_rBuf>::m_logicalIndex);
        }

        /// @brief Comparison operator > overload.
        /// @param other iterator to compare against.
        /// @return True if others index is smaller.
        /// @details Constant complexity.
        bool operator>(const _rBuf_iterator& other) const noexcept
        {
            //TODO:compatibility check
            return (_rBuf_const_iterator<_rBuf>::m_logicalIndex > other._rBuf_const_iterator<_rBuf>::m_logicalIndex);
        }

        /// @brief Comparison <= overload.
        /// @param other Other iterator to compare against.
        /// @return True if other points to logically smaller or the same indexed element.
        /// @details Constant complexity.
        bool operator<=(const _rBuf_iterator& other) const noexcept
        {
            //TODO:compatibility check
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


    using iterator = _rBuf_iterator<ring_buffer<T>>;
    using const_iterator = _rBuf_const_iterator<ring_buffer<T>>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

//============================
// End of iterators
//============================


    /// @brief Default constructor. Constructs to 0 size and 2 capacity.
    /// @throw Might throw std::bad_alloc if there is not enough memory available for allocation.
    /// @post this->empty() == true.
    /// @exception If any exception is thrown the buffer will be in a valid but unexpected state. (Basic exception guarantee).
    /// @details Constant complexity.
    ring_buffer() : ring_buffer(0, allocator_type())
    {
    }

    /// @brief Constructs the container with a custom allocator to 0 size and 2 capacity.
    /// @param alloc Custom allocator for the buffer.
    /// @post this->empty() == true.
    /// @throw Might throw std::bad_alloc if there is not enough memory available for allocation.
    /// @exception If any exception is thrown the buffer will be in a valid but unexpected state. (Basic exception guarantee).
    /// @details Constant complexity.
    explicit ring_buffer(const allocator_type& alloc) : ring_buffer(0, alloc)
    {
    }

    /// @brief Constructs the buffer to a given size with given values and optionally a custom allocator.
    /// @param size Amount of elements to be initialized in the buffer.
    /// @param val Reference to a value which the elements are initialized to.
    /// @param alloc Custom allocator.
    /// @pre T needs to satisfy CopyInsertable.
    /// @post std::distance(begin(), end()) == count.
    /// @note Allocates memory for count + 2 elements.
    /// @throw Might throw std::bad_alloc if there is not enough memory available for allocation.
    /// @exception If any exception is thrown the buffer will be in a valid but unexpected state. (Basic exception guarantee).
    /// @details Linear complexity in relation to amount of constructed elements (O(n)).
    ring_buffer(size_type count, const_reference val, const allocator_type& alloc = allocator_type()) : m_headIndex(0), m_tailIndex(0), m_allocator(alloc)
    {
        m_capacity = count + 2;
        m_data = m_allocator.allocate(m_capacity);

        for (size_t i = 0; i < count; i++)
        {
            push_back(val);
        }
    }

    /// @brief Custom constructor. Initializes a buffer to a capacity without constructing any elemen.
    /// @param capacity Capacity of the buffer.
    /// @pre T must satisfy DefaultInsertable.
    /// @throw Might throw std::bad_alloc if there is not enough memory available for allocation, or some exception from T's constructor.
    /// @exception If any exception is thrown the buffer will be in a valid but unexpected state. (Basic exception guarantee).
    /// @details Linear complexity in relation to count (O(n)).
    explicit ring_buffer(size_type count, const allocator_type& alloc = allocator_type()) : m_headIndex(0), m_tailIndex(0), m_allocator(alloc)
    {
        m_data = m_allocator.allocate(count + 2);
        m_capacity = count + 2;

        for (size_t i = 0; i < count; i++)
        {
            m_allocator.construct(m_data + i);
            increment(m_headIndex);
        }
    }

    /// @brief Construct the buffer from range [begin,end).
    /// @param beginIt Iterator to first element of range.
    /// @param endIt Iterator pointing to past-the-last element of range.
    /// @pre valye_type must satisfy CopyInsertable. InputIt must be deferencable to value_type, and incrementing rangeBegin (repeatedly) must reach rangeEnd. Otherwise behaviour is undefined.
    /// @throw Might throw std::bad_alloc if there is not enough memory available for allocation, or something from T's constructor.
    /// @exception If any exception is thrown the buffer will be in a valid but unexpected state. (Basic exception guarantee).
    /// @details Linear complexity in relation to the size of the range (O(n)).
    /// @note Behavior is undefined if elements in range are not valid.
    template<typename InputIt>
    ring_buffer(InputIt beginIt, InputIt endIt, const allocator_type& alloc = allocator_type()) : m_headIndex(0) , m_tailIndex(0)
    {
        const auto size = std::distance<InputIt>(beginIt , endIt);

        m_data = m_allocator.allocate(size + 2);
        m_capacity = size + 2;

        for (difference_type i = 0; i < size; i++)
        {
            m_allocator.construct(m_data + i, *(beginIt + i));
            increment(m_headIndex);
        }
    }

    /// @brief Initializer list contructor.
    /// @param init Initializer list to initialize the buffer from.
    /// @pre T must satisfy CopyInsertable.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @note Allocates memory for 2 extra elements.
    /// @details Linear complexity in relation to initializer list size (O(n)).
    ring_buffer(std::initializer_list<T> init) : ring_buffer(init.begin(),init.end())
    {
    }

    /// @brief Copy constructor.
    /// @param rhs Reference to a RingBuffer to create a copy from.
    /// @pre T must meet CopyInsertable.
    /// @post this == ring_buffer(rhs).
    /// @throw Might throw std::bad_alloc if there is not enough memory for memory allocation, or something from value_types constructor.
    /// @except If any exception is thrown, invariants are preserved.(Basic Exception Guarantee).
    /// @details Linear complexity in relation to buffer size.
    ring_buffer(const ring_buffer& rhs) : m_headIndex(rhs.m_tailIndex), m_tailIndex(rhs.m_tailIndex), m_capacity(rhs.m_capacity)
    {
        m_data = m_allocator.allocate(rhs.capacity());

        for (size_t i = 0; i < rhs.size(); i++)
        {
            m_allocator.construct(&this->operator[](i), rhs[i]);
            increment(m_headIndex);
        }
    }

    /// @brief Copy constructor with custom allocator.
    /// @param rhs Reference to a RingBuffer to create a copy from.
    /// @param alloc Allocator for the new buffer.
    /// @pre T must meet CopyInsertable.
    /// @post this == ring_buffer(rhs) but with a different allocator.
    /// @throw Might throw std::bad_alloc if there is not enough memory for memory allocation, or something from value_types constructor.
    /// @except If any exception is thrown, invariants are preserved.(Basic Exception Guarantee).
    /// @details Linear complexity in relation to buffer size.
    ring_buffer(const ring_buffer& rhs, const allocator_type& alloc) : m_headIndex(rhs.m_tailIndex), m_tailIndex(rhs.m_tailIndex), m_capacity(rhs.m_capacity), m_allocator(alloc)
    {
        m_data = m_allocator.allocate(rhs.capacity());

        for (size_type i = 0; i < rhs.size(); i++)
        {
            m_allocator.construct(m_data + i, rhs[i]);
            increment(m_headIndex);
        }
    }

    /// @brief Move constructor.
    /// @param other Rvalue reference to other buffer.
    /// @pre value_type must resolve std::is_nothrow_move_constructible<value_type>::value to true. Otherwise function does nothing.
    /// @details Linear complexity in relation to buffer size, unless other's allocator compares equal or is propagated on move assignment, then complexity is Constant.
    ring_buffer(ring_buffer&& other) noexcept : m_headIndex(other.m_headIndex), m_tailIndex(other.m_tailIndex), m_capacity(other.m_capacity), m_allocator(std::move(other.m_allocator))
    {
        if (!std::is_nothrow_move_constructible<value_type>::value) return;

        if (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
        {
            m_allocator = other.m_allocator;
            m_data = std::exchange(other.m_data, nullptr);
        }
        else if (m_allocator == other.m_allocator)
        {
            m_data = std::exchange(other.m_data, nullptr);
        }
        // Allocator is different and does not propagate, move elements individually.
        else
        {
            m_data = m_allocator.allocate(m_capacity);

            for (size_t i = 0; i < other.size(); i++)
            {
                m_allocator.construct(&this->operator[](i), std::move(other[i]));
            }
            other.m_data = nullptr;
        }

        other.m_capacity = 0;
        other.m_headIndex = 0;
        other.m_tailIndex = 0;
    }

    /// @brief Move constructor with different allocator.
    /// @param other Rvalue reference to other buffer.
    /// @param alloc Allocator for the new ring buffer.
    /// @pre value_type must resolve std::is_nothrow_move_constructible<value_type>::value to true. Otherwise function does nothing.
    /// @details Linear complexity in relation to buffer size, unless other's allocator compares equal or is propagated on move assignment, then complexity is Constant.
    ring_buffer(ring_buffer&& other, const allocator_type& alloc) noexcept : m_headIndex(other.m_headIndex), m_tailIndex(other.m_tailIndex), m_capacity(other.m_capacity), m_allocator(alloc)
    {
        if (!std::is_nothrow_move_constructible<value_type>::value) return;
        
        // Allocator is the same, just take it all.
        if (m_allocator == other.get_allocator())
        {
            m_data = other.m_data;
            other.m_data = nullptr;
            other.m_headIndex = 0;
            other.m_tailIndex = 0;
            other.m_capacity = 0;

        }
        // Allocator is different, allocate memory with new allocator and manually move the elements. Clean up original buffer after.
        else
        {
            m_data = m_allocator.allocate(m_capacity);
            for (size_type i = 0; i < other.size(); i++)
            {
                m_allocator.construct(&this->operator[](i), std::move(other[i]));
            }

            // Clean up the original buffer.
            other.clear();
            other.m_allocator.deallocate(other.m_data, other.m_capacity);
            other.m_data = nullptr;
            other.m_capacity = 0;
            other.m_headIndex = 0;
            other.m_tailIndex = 0;
        }

    }

    /// Destructor.
    ~ring_buffer()
    {
        // Calls destructor for each element in the buffer.
        for_each(begin(),end(),[this](T& elem) { m_allocator.destroy(&elem); });

        // After destruction deallocate the memory.
        m_allocator.deallocate(m_data, m_capacity);
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
    /// @details Constant complexity.
    template<class... Args>
    iterator emplace(const_iterator pos, Args&&... args)
    {
        validateCapacity(1);

        iterator it(this, pos.getIndex());
        m_allocator.construct(&*end(), std::forward<Args>(args)...);
        increment(m_headIndex);
        std::rotate(it, end() - 1, end());

        return it;
    }

    /// @brief Constructs an element in place to front from argumets.
    /// @param args Argument pack containing arguments to construct value_type element.
    /// @pre value_type is EmplaceConstructible from args.
    /// @throw Can throw std::bad_alloc if memory is allocated. Can also throw from T's constructor when constructing the element.
    /// @exception If any exception is thrown, invariants are preserved. (Basic exception guarantee).
    /// @details Constant complexity.
    template<class... Args>
    void emplace_front(Args&&... args)
    {
        validateCapacity(1);

        auto tempIndex = m_tailIndex;
        decrement(tempIndex);
        
        m_allocator.construct(&m_data[tempIndex], std::forward<Args>(args)...);
        m_tailIndex = tempIndex;
    }

    /// @brief Constructs an element in place to front from argumets.
    /// @param args Argument pack containing arguments to construct value_type element.
    /// @pre value_type is EmplaceConstructible from args.
    /// @throw Can throw std::bad_alloc if memory is allocated. Can also throw from T's constructor when constructing the element.
    /// @exception If any exception is thrown, invariants are preserved. (Basic exception guarantee).
    /// @details Constant complexity.
    template<class... Args>
    void emplace_back(Args&&... args)
    {
        validateCapacity(1);

        m_allocator.construct(&*end(), std::forward<Args>(args)...);
        increment(m_headIndex);
    }

    /// @brief Erase an element at a given position.
    /// @param pos Pointer to the element to be erased.
    /// @pre value_type must be nothrow-MoveConstructible. pos must be a valid dereferenceable iterator within the container. Otherwise behavior is undefined.
    /// @return Returns an iterator that was immediately following the ereased element. If the erased element was last in the buffer, returns a pointer to the new last element.
    /// @except Linear Complexity.
    iterator erase(const_iterator pos)
    {
        iterator returnIt(this, pos.getIndex());

        if (pos != end() - 1)
        {
            // Move assign elements effectively overwriting the 
            for (auto it = returnIt; it < end() - 1; it++)
            {
                *it = std::move(*(it + 1));
            }
            ++returnIt;
        }


        // Last element is always destroyed.
        m_allocator.destroy(&*(end() - 1));
        decrement(m_headIndex);

        return returnIt;
    }

    /// @brief Erase the specified elements from the container according to the range [first,last). Might destroy or move assign to the elements depending if last == end(). If last == end(), elements in [first,last) are destroyed.
    /// @param first iterator to the first element to erase.
    /// @param last iterator past the last element to erase.
    /// @pre First and last must be valid iterators to *this.
    /// @return Returns an iterator to the element that was immediately following the erased elements. If last == end(), then end() is returned.
    /// @throw Possibly throws from value_types move/copy assignment operator if last != end().
    /// @exception If last != end, this function will not throw (noexcept). If the assignemnt throws, the invariants are retained. (Basic exception guarantee).
    /// @details Linear Complexity in relation to size of the range, and then linear in remaining elements after the erased range.
    iterator erase(const_iterator first, const_iterator last)
    {
        iterator returnIt(this, first.getIndex());

        const auto diff = std::distance(first, last);
 
        // If the range does not reach the end, move elements.
        if (last != end())
        {
            auto tempIt = returnIt;
            // Move elements effectively overwriting the elements to be erased.
            for (; tempIt + diff !=  end(); ++tempIt)
            {
                *tempIt = std::move(*(tempIt + diff));
            }

            for (; tempIt != end(); ++tempIt)
            {
                m_allocator.destroy(&*tempIt);
            }
        }
        else
        {
            for (auto it = end() - diff; it != end(); it++)
            {
                m_allocator.destroy(&*(it));
            }
        }

        decrement(m_headIndex, diff);
        return returnIt;
    }

    /// @brief Destroys all elements in a buffer. Does not modify capacity.
    /// @post All existing references, pointers and iterators are to be considered invalid.
    /// @details Linear complexity in relation to size of the buffer.
    void clear() noexcept
    {
        for(; m_tailIndex < m_headIndex ; ++m_tailIndex)
        {
            m_allocator.destroy(&m_data[m_tailIndex]);
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
        ring_buffer copy(other);
        copy.swap(*this);
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
        // If sum of tailIndex (physical first element) and logical index(logical element) is larger than vector capacity, 
        // wrap index around to begin.
        auto index(m_tailIndex + logicalIndex);

        if(m_capacity <= index)
        {
            index -= m_capacity;
        }

        return m_data[index];
    }

    /// @brief Index operator.
    /// @param logicalIndex Index of the element used to access n:th element of the buffer.
    /// @details Constant complexity
    /// @note The operator acts as interface that hides the physical memory layout from the user. Logical index neeeds to be added to internal tail index to get actual element address.
    /// @return Returns a const reference the the element ad logicalIndex.
    const_reference operator[](const size_type logicalIndex) const noexcept
    {
        auto index(m_tailIndex + logicalIndex);

        // Adjust real index if buffer has wrapped around the allocated physical memory (head is "lower" than tail)
        if(m_capacity <= index)
        {
            index -= m_capacity;
        }

        return m_data[index];
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
    void swap(ring_buffer& other) noexcept
    {
        using std::swap;
        swap(m_data, other.m_data);
        swap(m_headIndex, other.m_headIndex);
        swap(m_tailIndex, other.m_tailIndex);
        swap(m_capacity, other.m_capacity);
        swap(m_allocator, other.m_allocator);

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
        auto tempData = m_allocator.allocate(m_capacity);

        for (size_t i = 0; i < size(); i++)
        {
            m_allocator.construct(tempData + i, std::move(this->operator[](i)));
            m_allocator.destroy(&this->operator[](i));
        }

        m_allocator.deallocate(m_data, size());

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
    /// @details Linear complexity in relation to size of the buffer.
    void reserve(size_type newCapacity, bool enableShrink = false)
    {
        if (!enableShrink)
        {
            if (newCapacity <= m_capacity) return;
        }
        else
        {
            if (newCapacity < size() ) return;
        }

        // Temp for "copy and swap" idiom to exception safe operation.
        auto temp = ring_buffer<T, allocator_type>();
        temp.m_capacity = newCapacity;
        temp.m_data = temp.m_allocator.allocate(newCapacity);

        for(size_t i = 0; i < this->size(); i++)
        {
            temp.m_allocator.construct(temp.m_data + i, std::move(this->operator[](i)));
        }
        temp.m_headIndex = this->size();

        // Assings the data from temp to original buffer. The resources from temp will be released when function goes out of scope.
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
        m_allocator.construct(&m_data[newIndex], val);
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
        m_allocator.construct(&m_data[newIndex], std::forward<value_type>(val));
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
        if(this->size() + 1 == m_capacity)
        {
            reserve(m_capacity / 2 + m_capacity);
        }

        m_allocator.construct(&m_data[m_headIndex], val);
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
    
        m_allocator.construct(&m_data[m_headIndex], std::forward<value_type>(val));
        increment(m_headIndex);
    }

    /// @brief Remove the first element in the buffer.
    /// @pre Buffers size > 0, otherwise behaviour is undefined.
    /// @post All iterators, pointers and references are invalidated.
    /// @details Constant complexity.
    void pop_front() noexcept
    {
        m_allocator.destroy(&m_data[m_tailIndex]);
        increment(m_tailIndex);
    }

    /// @brief Erase an element from the logical back of the buffer.
    /// @pre Buffers size > 0, otherwise behaviour is undefined.
    /// @post All pointers and references are invalidated. Iterators persist except end() - 1 iterator is invalidated (it becomes new past-the-last iterator).
    /// @details Constant complexity.
    void pop_back() noexcept
    {
        decrement(m_headIndex);
        m_allocator.destroy(&m_data[m_headIndex]);

    }

    /// @brief Releases unused allocated memory. 
    /// @pre T must satisfy MoveConstructible or CopyConstructible.
    /// @post m_capacity == size() + 2.
    /// @note Reduces capacity by allocating a smaller memory area and moving the elements. Shrinking the buffer invalidates all pointers, iterators and references.
    /// @throw Might throw std::bad_alloc if memory allocation fails.
    /// @exception If T's move (or copy) constructor can and does throw, behaviour is undefined. If any other exception is thrown (bad_alloc) this function has no effect (Strong exception guarantee).
    /// @details Linear complexity in relation to size of the buffer.
    void shrink_to_fit()
    {
        // Reserve would allocate size + 2 elements, so no point in shrinking if buffer is smaller than that.
        if (m_capacity <= size() + 2)
        { 
            return;
        }

        reserve(size(), true);
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

    /// @brief Reserves more memory if needed for an increase in size. If more memory is needed, allocates (capacity * 1.5) or if that is not enough (capacity * 1.5 + increase).
    /// @param increase Expected increase in size of the buffer, based on which memory is allocated.
    /// @details Linear complexity in relation to buffer size if more memory needs to be allocated, otherwise constant complexity.
    /// @exception May throw std::bad_alloc. If any exception is thrown this function does nothing. Strong exception guarantee.
    /// @note This function should be called before increasing the size of the buffer.
    void validateCapacity(size_t increase)
    {
        if (m_capacity <= size() + increase)
        {
            if (m_capacity / 2 + m_capacity <= size() + increase)
            {
                reserve(m_capacity + increase + m_capacity / 2 );
            }
            else
            {
                reserve(m_capacity / 2 + m_capacity);
            }
        }
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

        for (size_type i = 0; i < amount; i++)
        {
            // Construct element at the end.
            m_allocator.construct(&*end(), std::forward<InsertValue>(value));
            increment(m_headIndex);
        }

        // Rotate elements from the back into pos.
        std::rotate(it, end() - amount, end());

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
        
        for (; rangeBegin != rangeEnd; rangeBegin++)
        {
            m_allocator.construct(&*end(), *rangeBegin);
            increment(m_headIndex);
        }

        iterator it(this, pos.getIndex());
        std::rotate(it, end() - amount, end());

        return it;
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

//==========================================
// Members 
//==========================================

    size_type m_headIndex; /*!< Index of the head. Index pointing to past the last element.*/ 
    size_type m_tailIndex; /*!< Index of the tail. Index to the first element in the buffer.*/
    size_type m_capacity;  /*!< Capacity of the buffer. How many elements of type T the buffer has currently allocated memory for.*/
    
    T* m_data;  /*!< Pointer to allocated memory.*/
    Allocator m_allocator;  /*!< Allocator used to allocate/ deallocate and construct/ destruct elements. Default is std::allocator<T>*/
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