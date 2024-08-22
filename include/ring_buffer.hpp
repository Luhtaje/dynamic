#ifndef DYNAMIC_RINGBUFFER_HPP
#define DYNAMIC_RINGBUFFER_HPP

#include <memory>
#include <algorithm>
#include <limits>
#include <utility>
#include <stdexcept>
#include <cstring>
#include <vector>

namespace
{
    // Buffer always reserves two "extra" spaces. This ensures that reserve and other relocating functions work correctly (the "never full" invariant).
    constexpr size_t allocBuffer = 2;

    //Temporary object holder.
    template<typename Alloc>
    struct _alloc_temp
    {
        using value_type = typename Alloc::value_type;
        using _traits = std::allocator_traits<Alloc>;

        Alloc&  _alloc;

        union
        {
            value_type _value;
        };

        value_type& _getValue() noexcept
        {
            return _value;
        }

        const value_type& _getValue() const noexcept
        {
            return _value;
        }

        template<typename... Args>
        explicit _alloc_temp(Alloc allocator, Args&&... args) noexcept(
        noexcept(_traits::construct(_alloc, std::addressof(_getValue()), std::forward<Args>(args)...)))
        : _alloc(allocator)
        {
            _traits::construct(_alloc, std::addressof(_getValue()), std::forward<Args>(args)...);
        }

        ~_alloc_temp() noexcept
        {
            _traits::destroy(_alloc, std::addressof(_getValue()));
        }

    };

    //Base class that wraps memory allocation into an initialization (RAII).
    template<typename T, typename Allocator = std::allocator<T>>
    struct ring_buffer_base {

        using size_type = std::size_t;
        using allocator_type = Allocator;
        using alloc_traits = std::allocator_traits<allocator_type>;

        size_type m_capacity;  /*!< Capacity of the buffer. How many elements of type T the buffer has currently allocated memory for.*/

        T* m_data;  /*!< Pointer to allocated memory.*/
        Allocator m_allocator;  /*!< Allocator used to allocate/deallocate and construct/destruct elements. Default is std::allocator<T>*/

        ring_buffer_base(const Allocator& alloc, size_type capacity)
            : m_allocator(alloc), m_data(alloc_traits::allocate(m_allocator, capacity)), m_capacity(capacity)
        {
        }

        ring_buffer_base(const ring_buffer_base&) = delete;
        ring_buffer_base& operator=(const ring_buffer_base&) = delete;

        ring_buffer_base(ring_buffer_base&& other) noexcept : m_allocator(std::move(other.m_allocator)), m_data(std::exchange(other.m_data, nullptr)), m_capacity(std::exchange(other.m_capacity, 0))
        {
        }

        ring_buffer_base& operator=(ring_buffer_base&& other) noexcept
        {
            swap(*this,other);
            return *this;
        }

        template<typename U>
        void swap(ring_buffer_base<U>& left, ring_buffer_base<U>& right) noexcept
        {
            std::swap(left.m_allocator, right.m_allocator);    
            std::swap(left.m_data, right.m_data);
            std::swap(left.m_capacity, right.m_capacity);
        }

        ring_buffer_base() { alloc_traits::deallocate(m_data, m_capacity); }
    };
}

// Forward declaration of _rBuf_const_iterator.
template<class _rBuf>
class _rBuf_const_iterator;

/// @brief Dynamic Ringbuffer is a dynamically growing circular AllocatorAware std::container with support for queue, stack and priority queue adaptor functionality.
/// @tparam T Type of the elements.
/// @tparam Allocator Allocator used for (de)allocation and (de)construction. Defaults to std::allocator<T>
template<typename T, typename Allocator = std::allocator<T>> 
class ring_buffer : private ring_buffer_base<T,Allocator>
{

public:

    using base = typename ring_buffer<T,Allocator>::ring_buffer_base;

    using size_type = typename base::size_type;
    using allocator_type = typename base::allocator_type;
    using alloc_traits = typename base::alloc_traits;

    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using difference_type = std::ptrdiff_t;

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

        using c_iterator = _rBuf_const_iterator<_rBuf>;
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
        explicit _rBuf_iterator(_rBuf* container, size_type index) : c_iterator(container, index) {}

        /// @brief Dereference operator
        /// @return  Returns the object the iterator is currently pointing to.
        /// @details Constant complexity.
        reference operator*() const noexcept
        {
            return (*(const_cast<_rBuf*>(c_iterator::m_container)))[c_iterator::m_logicalIndex];
        }

        /// @brief Arrow operator. 
        /// @return Returns a pointer to the object the iterator is currently pointing to.
        /// @details Constant complexity.
        pointer operator->() const noexcept
        {
            return const_cast<pointer>(c_iterator::operator->());
        }

        /// @brief Prefix increment.
        /// @note Incrementing the iterator over the end() iterator leads to invalid iterator (dereferencing is undefined behaviour).
        /// @details Constant complexity.
        _rBuf_iterator& operator++() noexcept
        {
            ++c_iterator::m_logicalIndex;
            return (*this);
        }

        /// @brief Postfix increment
        /// @param  int empty parameter to guide overload resolution.
        /// @note Incrementing the iterator over the end() iterator leads to invalid iterator (dereferencing is undefined behaviour).
        /// @details Constant complexity.
        _rBuf_iterator operator++(int)
        {
            auto temp(*this);
            ++c_iterator::m_logicalIndex;
            return temp;
        }

        /// @brief Prefix decrement
        /// @Details Constant complexity.
        /// @note Decrementing the iterator past begin() leads to invalid iterator (dereferencing is undefined behaviour).
        _rBuf_iterator& operator--() noexcept
        {
            --c_iterator::m_logicalIndex;
            return(*this);
        }

        /// @brief Postfix decrement
        /// @param  int empty parameter to guide overload resolution.
        /// @details Constant complexity.
        /// @note Decrementing the iterator past begin() leads to invalid iterator (dereferencing is undefined behaviour).
        _rBuf_iterator operator--(int)
        {
            auto temp(*this);
            --c_iterator::m_logicalIndex;
            return temp;
        }

        /// @brief Moves iterator forward.
        /// @param offset Amount of elements to move.
        /// @note Moving the iterator beyond begin() or end() makes the iterator point to an invalid element (dereferencing is undefined behaviour).
        /// @details Constant complexity.
        _rBuf_iterator& operator+=(difference_type offset) noexcept
        {
            if (offset < 0)
            {
                c_iterator::m_logicalIndex -= abs(offset);
            }
            else
            {
                c_iterator::m_logicalIndex += offset;
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
            return (c_iterator::m_logicalIndex - other.c_iterator::m_logicalIndex);
        }

        /// @brief Index operator.
        /// @param offset Signed offset from iterator index.
        /// @return Return object pointer by the iterator with an offset.
        /// @note If offset is such that the iterator is beyond end() or begin() this function has undefined behaviour.
        /// @details Constant complexity.
        reference operator[](const difference_type offset) const noexcept
        {
            return const_cast<reference>(c_iterator::m_container->operator[](offset));
        }

        /// @brief Comparison operator < overload.
        /// @param other iterator to compare.
        /// @return true if others index is larger.
        /// @details Constant complexity.
        bool operator<(const _rBuf_iterator& other) const noexcept
        {
            return (c_iterator::m_logicalIndex < other.c_iterator::m_logicalIndex);
        }

        /// @brief Comparison operator > overload.
        /// @param other iterator to compare against.
        /// @return True if others index is smaller.
        /// @details Constant complexity.
        bool operator>(const _rBuf_iterator& other) const noexcept
        {
            return (c_iterator::m_logicalIndex > other.c_iterator::m_logicalIndex);
        }

        /// @brief Comparison <= overload.
        /// @param other Other iterator to compare against.
        /// @return True if other points to logically smaller or the same indexed element.
        /// @details Constant complexity.
        bool operator<=(const _rBuf_iterator& other) const noexcept
        {
            return (c_iterator::m_logicalIndex <= other.c_iterator::m_logicalIndex);
        }

        /// @brief Comparison >= overload.
        /// @param other Other iterator to compare against.
        /// @return True if other points to logically larger or same indexed element.
        /// @details Constant complexity.
        bool operator>=(const _rBuf_iterator& other) const noexcept
        {
            return (c_iterator::m_logicalIndex >= other.c_iterator::m_logicalIndex);
        }

        /// @brief Custom assingment operator overload.
        /// @param index Logical index of the element to set the iterator to.
        /// @note Undefined behaviour for negative index.
        /// @details Constant complexity.
        _rBuf_iterator& operator=(const size_t index) noexcept
        {
            c_iterator::m_logicalIndex = index;
            return (*this);
        };

        /// @brief Index getter.
        /// @return Returns the index of the element this iterator is pointing to.
        /// @details Constant complexity.
        difference_type getIndex() noexcept
        {
            return c_iterator::m_logicalIndex;
        }
    };


    using iterator = _rBuf_iterator<ring_buffer<T>>;
    using const_iterator = _rBuf_const_iterator<ring_buffer<T>>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

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
    explicit ring_buffer(const allocator_type& alloc) : base(alloc, allocBuffer), m_headIndex(0), m_tailIndex(0)
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
    ring_buffer(size_type count, const_reference val, const allocator_type& alloc = allocator_type()) : base(alloc, count + allocBuffer), m_headIndex(count), m_tailIndex(0)
    {
        std::uninitialized_fill_n(base::m_data, count, val);
    }
    
    /// @brief Custom constructor. Initializes a buffer with count amount of default constructed value_type elements.
    /// @param count amount of default constructed value_type elements.
    /// @pre T must satisfy DefaultInsertable.
    /// @throw Can throw std::bad_alloc if there is not enough memory available for allocation, or some exception from T's constructor.
    /// @exception If any exception is thrown the buffer will be in a valid but unexpected state. (Basic exception guarantee).
    /// @details Linear complexity in relation to count (O(n)).
    explicit ring_buffer(size_type count, const allocator_type& alloc = allocator_type()) : base(alloc, count + allocBuffer), m_headIndex(count), m_tailIndex(0)
    {
        size_t first = 0;
        size_t current = 0;

        try
        {
            for (size_t i = 0; i < count; i++)
            {
                alloc_traits::construct(base::m_allocator, base::m_data + current);
                current++;
            }
        }
        catch (...)
        {
            for (; first != current; first++)
            {
                alloc_traits::destroy(base::m_allocator, base::m_data + first);
            }
            
            m_headIndex = 0;

            throw;
        }
    }

    /// @brief Construct the buffer from range [begin,end).
    /// @param beginIt Iterator to first element of range.
    /// @param endIt Iterator pointing to past-the-last element of range.
    /// @pre valye_type must satisfy CopyInsertable. InputIt must be deferencable to value_type, and incrementing rangeBegin (repeatedly) must reach rangeEnd. Otherwise behaviour is undefined.
    /// @throw Can throw std::bad_alloc, or something from T's CopyConstructor if not NoThrowCopyConstructible.
    /// @exception If any exception is thrown no memory is leaked and program remains in a valid state. (Basic exception guarantee).
    /// @details Linear complexity in relation to the size of the range (O(n)).
    /// @note Behavior is undefined if elements in range are not valid.
    template<typename InputIt,typename = std::enable_if_t<std::is_convertible<typename std::iterator_traits<InputIt>::value_type,value_type>::value>>
    ring_buffer(InputIt beginIt, InputIt endIt, const allocator_type& alloc = allocator_type())
        : base(alloc, std::distance<InputIt>(beginIt,endIt) + allocBuffer), m_headIndex(std::distance<InputIt>(beginIt, endIt)), m_tailIndex(0)
    {
        std::uninitialized_copy(beginIt, endIt, base::m_data);
    }

    /// @brief Initializer list contructor.
    /// @param init Initializer list to initialize the buffer from.
    /// @pre T must satisfy CopyInsertable.
    /// @throw Can throw std::bad_alloc, or something from T's CopyConstructor if not NoThrowCopyConstructible.
    /// @exception If any exception is thrown no memory is leaked and program remains in a valid state. (Basic exception guarantee).
    /// @details Linear complexity in relation to initializer list size (O(n)).
    ring_buffer(std::initializer_list<T> init) : ring_buffer(init.begin(),init.end())
    {
    }

    /// @brief Copy constructor.
    /// @param rhs Reference to a RingBuffer to create a copy from.
    /// @pre T must meet CopyInsertable.
    /// @post this == ring_buffer(rhs).
    /// @throw Can throw std::bad_alloc, or something from T's CopyConstructor if not NoThrowCopyConstructible.
    /// @except If any exception is thrown, invariants are preserved.(Basic Exception Guarantee).
    /// @details Linear complexity in relation to buffer size.
    ring_buffer(const ring_buffer& rhs) 
    : m_headIndex(rhs.m_headIndex), m_tailIndex(rhs.m_tailIndex), base(alloc_traits::select_on_container_copy_construction(rhs.m_allocator), rhs.capacity())
    {
        std::uninitialized_copy(rhs.begin(), rhs.end(), base::m_data);
    }

    /// @brief Copy constructor with custom allocator.
    /// @param rhs Reference to a RingBuffer to create a copy from.
    /// @param alloc Allocator for the new buffer.
    /// @pre T must meet CopyInsertable.
    /// @post this == ring_buffer(rhs) but with a different allocator.
    /// @throw Can throw std::bad_alloc, or something from T's CopyConstructor if not NoThrowCopyConstructible.
    /// @except If any exception is thrown, invariants are preserved.(Basic Exception Guarantee).
    /// @details Linear complexity in relation to buffer size.
    ring_buffer(const ring_buffer& rhs, const allocator_type& alloc) : base(alloc, rhs.m_capacity), m_headIndex(rhs.m_headIndex), m_tailIndex(rhs.m_tailIndex)
    {
        std::uninitialized_copy(rhs.begin(), rhs.end(), base::m_data);
    }

    /// @brief Move constructor.
    /// @param other Rvalue reference to other buffer.
    /// @details Constant complexity.
    ring_buffer(ring_buffer&& other) noexcept : base(std::move(other)), m_headIndex(std::exchange(other.m_headIndex, 0)), m_tailIndex(std::exchange(other.m_tailIndex,0))
    {
    }

    /// @brief Move constructor with different allocator.
    /// @param other Rvalue reference to other buffer.
    /// @param alloc Allocator for the new ring buffer.
    /// @details Linear complexity in relation to buffer size.
    ring_buffer(ring_buffer&& other, const allocator_type& alloc) : base(alloc, other.m_capacity), m_headIndex(other.m_headIndex), m_tailIndex(other.m_tailIndex)
    {
        size_t first = 0;
        size_t current = 0;

        try
        {
            for (size_t i = 0; i < other.size(); i++)
            {
                alloc_traits::construct(base::m_allocator, base::m_data + current, std::move(other[i]));
                current++;
            }
        }
        catch (...)
        {
            for (; first != current; first++)
            {
                alloc_traits::destroy(base::m_allocator, base::m_data + first);
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
        destroy_elements();
    }

    /// @brief Inserts an element to the buffer.
    /// @param pos Iterator where the the element should be inserted. 
    /// @param value Value to insert.
    /// @return iterator pointing to the inserted value.
    /// @pre T must meet CopyInsertable. 
    /// @throw Might throw std::bad_alloc, or something from T's copy constructor if not NoThrow.
    /// @exception  If any exception is thrown, the function does nothing (Strong exception guarantee).
    /// @details Linear complexity in relation to buffer size and inserted elements. O(n).
    iterator insert(const_iterator pos, const value_type& value)
    {
        return insertBase(pos, 1, value);
    }

    /// @brief Inserts an element to the buffer.
    /// @param pos Iterator where the the element should be inserted
    /// @param value Value to insert.
    /// @return Iterator that pos to the inserted element.
    /// @pre T must meet MoveInsertable.
    /// @throw Might throw std::bad_alloc, or something from T's move/copy constructor.
    /// @exception If any exception is thrown, the function does nothing (Strong exception guarantee).
    /// @details Linear complexity in relation to buffer size and inserted elements. O(n).
    iterator insert(const_iterator pos, value_type&& value)
    {
        return insertBase(pos, 1, std::move(value));
    }

    /// @brief Inserts an element to the buffer.
    /// @param pos Iterator where the the element should be inserted
    /// @param count Amount of T elements to be inserted.
    /// @param value Value to insert.
    /// @pre T must meet the requirements of CopyInsertable.
    /// @return Iterator that pos to the inserted element.
    /// @throw Might throw std::bad_alloc, or something from T's copy constructor if not NoThrow.
    /// @exception  If any exception is thrown, the function does nothing (Strong exception guarantee).
    /// @details Linear complexity in relation to buffer size and inserted elements. O(n).
    iterator insert(const_iterator pos, const size_type count, const value_type& value)
    {
        if(count == 0) return iterator(this, pos.getIndex());
        return insertBase(pos, count, value);
    }

    /// @brief Inserts a range of elements into the buffer to a specific position.
    /// @tparam InputIt Type of iterator for the range.
    /// @param pos A valid dereferenceable iterator to the position where range will be inserted to.
    /// @param sourceBegin Iterator to first element of the range.
    /// @param sourceEnd Iterator past the last element of the range.
    /// @return Returns an iterator to an element in the buffer which is copy of the first element in the range.
    /// @pre T must meet requirements of CopyInsertable. Iterators must point to elements that are implicitly convertible to value_type and sourceEnd must be reachable from sourceBegin. Otherwise behavior is undefined.
    /// @throw Can throw std::bad_alloc or something from value_types constructor and iterator operations. 
    /// @exception If any exceptiong is thrown, the function does nothing (Strong exception guarantee).
    /// @details Linear complexity in relation to buffer size and inserted elements. O(n).
    template <typename InputIt>
    iterator insert(const_iterator pos, InputIt sourceBegin, InputIt sourceEnd)
    {
        if(std::distance(sourceBegin, sourceEnd) == 0) return iterator(this, pos.getIndex() );
        return insertRangeBase(pos, sourceBegin, sourceEnd);
    }

    /// @brief Inserts initializer list into buffer to a specific position.
    /// @param pos Iterator where the list will be inserted.
    /// @param list Initiliazer list to insert.
    /// @pre pos must be a valid dereferenceable iterator within the container. Otherwise behavior is undefined.
    /// @return Returns Iterator to the first element inserted, or the element pointed by pos if the initializer list was empty.
    /// @throw Can throw std::bad_alloc and something from value_types constructor.
    /// @exception If any exceptiong is thrown, the function does nothing (Strong exception guarantee).
    /// @details Linear complexity in relation to buffer size and inserted elements. O(n).
    iterator insert(const_iterator pos, std::initializer_list<T> list)
    {   
        if (list.size() == 0) return iterator(this, pos.getIndex());
        return insertRangeBase(pos, list.begin(), list.end());
    }

    /// @brief Construct an element in place from arguments.
    /// @param pos Iterator before which the new element will be constructed.
    /// @param args Argument pack containing arguments to construct value_type element.
    /// @return Returns an iterator pointing to the element constructed from args.
    /// @pre T must meet EmplaceConstructible, MoveAssignalbe and MoveInsertable.
    /// @post Iterators, pointers and references are invalidated after the emplace point. If more memory is allocated, pointers and references to all elements are invalidated.
    /// @throw Can throw std::bad_alloc if memory is allocated. Can also throw from T's constructor when constructing the element. Additionally, rotate can throw bad_alloc and if T does not provide a noexcept move semantics.
    /// @exception If T's CopyConstructor is NoThrow then in case of any exception invariants are preserved. (Basic exception guarantee). If T's construction throws the behavior is undefined.
    /// @details Amortized linear complexity in relation to distance from pos to end().
    template<class... Args>
    iterator emplace(const_iterator pos, Args&&... args)
    {
        validateCapacity(1);

        iterator it(this, pos.getIndex());

        //Construct temporary
        _alloc_temp<Allocator> tempObj (base::m_allocator, std::forward<Args>(args)...);

        //Provide basic guarantee
        auto last = end();
        alloc_traits::construct(base::m_allocator, &*last, std::move(*(last - 1)));
        increment(m_headIndex);
        std::move_backward(it, last - 1 , last);

        *it = std::move(tempObj._getValue());

        return it;
    }

    /// @brief Constructs an element in place to front from argumets.
    /// @param args Argument pack containing arguments to construct value_type element.
    /// @pre value_type is EmplaceConstructible from args.
    /// @throw Can throw std::bad_alloc if memory is allocated. Can also throw from T's constructor when constructing the element.
    /// @exception If any exception is thrown, function has no effect. (Strong exception guarantee).
    /// @details  Amortized constant complexity.
    template<class... Args>
    void emplace_front(Args&&... args)
    {
        if (base::m_capacity < size() + allocBuffer)
        {
            auto sz = size();

            base temp(base::m_allocator, base::m_capacity * 3 / 2);
            std::uninitialized_copy(begin(), end(), temp.m_data);
            alloc_traits::construct(base::m_allocator, temp.m_data + temp.m_capacity - 1, std::forward<Args>(args)...);

            destroy_elements();

            base::swap(*this, temp);
            m_headIndex = sz;
            m_tailIndex = base::m_capacity - 1;

            return;
        }

        // Decrement temporary index in case constructor throws to retain invariants (elements of the buffer are always initialized).
        auto newIndex = m_tailIndex;
        decrement(newIndex);
        alloc_traits::construct(base::m_allocator, base::m_data + newIndex, std::forward<Args>(args)...);
        m_tailIndex = newIndex;
    }

    /// @brief Constructs an element in place to front from argumets.
    /// @param args Argument pack containing arguments to construct value_type element.
    /// @pre value_type is EmplaceConstructible from args.
    /// @throw Can throw std::bad_alloc if memory is allocated. Can also throw from T's constructor when constructing the element.
    /// @exception If any exception is thrown, function has no effect. (Strong exception guarantee).
    /// @details Amortized constant complexity.
    template<class... Args>
    void emplace_back(Args&&... args)
    {
        if (base::m_capacity < size() + allocBuffer)
        {
            auto sz = size();
            base temp(base::m_allocator, base::m_capacity * 3 / 2);
            std::uninitialized_copy(begin(), end(), temp.m_data);

            try
            {
                alloc_traits::construct(base::m_allocator, temp.m_data + sz, std::forward<Args>(args)...);
            }
            catch (...)
            {
                for (size_t i = 0; i < sz; ++i)
                {
                    alloc_traits::destroy(base::m_allocator, temp.m_data + i);
                }
                throw;
            }

            destroy_elements();

            base::swap(*this, temp);
            m_tailIndex = 0;
            m_headIndex = sz;
            increment(m_headIndex);

            return;
        }

        alloc_traits::construct(base::m_allocator, base::m_data + m_headIndex, std::forward<Args>(args)...);
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
    /// @exception If value_type is nothrow_move_constructible and nothrow_move_assignable function is noexcept. Otherwisde provides no exception guarantee at all.
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
        destroy_elements();

        m_headIndex = 0;
        m_tailIndex = 0;
    }

    /// @brief Replaces the elements in the buffer with copy of [sourceBegin, sourceEnd)
    /// @param sourceBegin Iterator to beginning of the range.
    /// @param sourceEnd Past the end iterator of the range.
    /// @pre value_type is CopyInsertable and elements of [sourceBegin, sourceEnd) are not in *this. InputIt must be dereferenceable to value_type, and incrementing sourceBegin (repeatedly) must reach sourceEnd. Otherwise behaviour is undefined.
    /// @post All existing references, pointers and iterators are to be considered invalid.
    /// @throw Can throw std::bad_alloc or something from value_types constructor if not nothrow. 
    /// @exception If any exception is throw, no memory is leaked but buffer will be in valid but unexpected state (Basic Exception Guarantee). If more memory is allocated, in case of any exception the function does nothing (Strong Exception Guarantee) 
    /// @details Linear Complexity. Calls destructor for each element in buffer and CopyConstructor for the assigned range.
    template <typename InputIt>
    void assign(InputIt sourceBegin, InputIt sourceEnd)
    {
        size_type amount = std::distance(sourceBegin, sourceEnd);
        if (base::m_capacity < amount + allocBuffer)
        {
            base temp{base::m_allocator, amount * 3 / 2 };
            std::uninitialized_copy(sourceBegin, sourceEnd, temp.m_data);
            base::swap(*this, temp);
            m_headIndex = amount;
            m_tailIndex = 0;

            return;
        }

        clear();
        std::uninitialized_copy(sourceBegin, sourceEnd, base::m_data);
        m_headIndex = amount;
    }

    /// @brief Replaces the elements in the buffer with copy of the initializer list.
    /// @param list Source of elements to assign.
    /// @pre value_type is CopyInsertable.
    /// @post All existing references, pointers and iterators are to be considered invalid.
    /// @throw Can throw std::bad_alloc or something from value_types constructor if not nothrow. 
    /// @exception If any exception is throw, no memory is leaked but buffer will be in valid but unexpected state (Basic Exception Guarantee). If more memory is allocated, in case of any exception the function does nothing (Strong Exception Guarantee) 
    /// @details Linear Complexity. Calls destructor for each element in buffer and CopyConstructor for each element in the list.
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
    /// @exception If any exception is throw, no memory is leaked but buffer will be in valid but unexpected state (Basic Exception Guarantee). If more memory is allocated and exception is thrown, function has no effect (Strong exception guarantee)
    /// @details Linear Complexity. Calls destructor for each element in buffer and amount times values constructor.
    void assign(const size_type amount, const value_type& value)
    {

        if (base::m_capacity < amount + allocBuffer)
        {
            base temp{base::m_allocator, amount * 3/2};
            std::uninitialized_fill_n(temp.m_data, amount, value);
            base::swap(*this, temp);
            m_headIndex = amount;
            
            return;
        }
        
        clear();
        std::uninitialized_fill_n(base::m_data, amount, value);
        m_headIndex = amount;
    }

    /// @brief Copy assignment operator.
    /// @param other Ringbuffer to be copy assigned.
    /// @return Returns reference to the assignment target container.
    /// @post *this == other. All iterators, pointers and references of the target container should be considered invalid. Does not guarantee that target containers capacity equals the original.
    /// @throw Can throw std::bad_alloc or something from value_types constructor.
    /// @exception If any exception is thrown, invariants are retained and no memory is leaked (Basic Exception Guarantee).
    /// @details Linear complexity in relation to buffer size. 
    ring_buffer& operator=(const ring_buffer& other)
    {

        if (this == &other) return *this;

        if (alloc_traits::propagate_on_container_copy_assignment::value && (base::m_allocator != other.m_allocator))
        {
            auto temp = ring_buffer(other);

            base::swap(*this, temp);
            std::swap(m_tailIndex, temp.m_tailIndex);
            std::swap(m_headIndex, temp.m_headIndex);
        }
        else
        {
            validateCapacity(other.size() - base::m_capacity);

            auto targetSize = size();
            auto sourceSize = other.size();

            if (targetSize <= sourceSize)
            {
                std::copy(other.begin(), other.begin() + targetSize, begin());
                for (size_type i = size(); i < sourceSize; i++)
                {
                    push_back(other[i]);
                }
            }
            else
            {
                std::copy(other.begin(), other.end(), begin());
                for (size_type i = sourceSize; i < targetSize; i++)
                {
                    pop_back();
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
    /// @exception If value_type is not MoveConstructible, and if value_type provides a throwing CopyConstructor this function has undefined behaviour.
    /// @details Constant complexity.
    ring_buffer& operator=(ring_buffer&& other) noexcept
    {
        if (!alloc_traits::propagate_on_container_move_assignment::value && base::m_allocator != other.m_allocator)
        {
            clear();

            alloc_traits::uninitialized_move(other.begin(), other.end(), base::m_data);
        }
        else
        {
            ring_buffer temp(std::move(other));
            temp.swap(*this);
        }

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
        return base::m_data[(m_tailIndex + logicalIndex) % base::m_capacity];
    }

    /// @brief Index operator.
    /// @param logicalIndex Index of the element used to access n:th element of the buffer.
    /// @details Constant complexity
    /// @note The operator acts as interface that hides the physical memory layout from the user. Logical index neeeds to be added to internal tail index to get actual element address.
    /// @return Returns a const reference the the element ad logicalIndex.
    const_reference operator[](const size_type logicalIndex) const noexcept
    {
        return base::m_data[(m_tailIndex + logicalIndex) % base::m_capacity];
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

        if(base::m_capacity <= index)
        {
            index -= base::m_capacity;
        }
        return base::m_data[index];
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
        if(base::m_capacity <= index)
        {
            index -= base::m_capacity;
        }
        return base::m_data[index];
    }

    /// @brief Member swap implementation. Swaps RingBuffers member to member.
    /// @param other Reference to a ring_buffer to swap with.
    /// @details Constant complexity.
    /// @note Requires allocator_type to return true_type from propagate_on_container_swap, otherwise memory operations 
    void swap(ring_buffer& other) noexcept
    {
        using std::swap;
        if (alloc_traits::propagate_on_container_swap::value)
        {
            swap(base::m_allocator, other.m_allocator);
        }

        swap(base::m_data, other.m_data);
        swap(base::m_capacity, other.m_capacity);
        swap(m_headIndex, other.m_headIndex);
        swap(m_tailIndex, other.m_tailIndex);
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
    /// @exception If T's Move (or copy in case T does not provide Move Semantics) constructor throws, behaviour is undefined. Otherwise if exceptions are thrown (std::bad_alloc) this function has no effect (Strong exception guarantee).
    /// @note Invalidates all existing pointers and references.
    /// @details Linear complexity in relation to buffer size.
    pointer data()
    {
        if(!size())
        {
            m_headIndex = 0;
            m_tailIndex = 0;
            return base::m_data;
        }

        base temp = {base::m_allocator, base::m_capacity};
        _uninitialized_move(begin(), end(), temp.m_data);
        base::swap(*this, temp);

        m_headIndex = size();
        m_tailIndex = 0;

        return base::m_data;
    }

    /// @brief Gets the size of the container.
    /// @return Size of buffer.
    /// @details Constant complexity.
    size_type size() const noexcept
    {
        if(m_headIndex < m_tailIndex)
        {
            return m_headIndex + base::m_capacity - m_tailIndex;
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
        return base::m_capacity;
    }

    /// @brief Allocator getter.
    /// @return Return the allocator used by the container.
    /// @details Constant complexity.
    allocator_type get_allocator() const noexcept
    {
        return base::m_allocator;
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
            if (newCapacity < size() + allocBuffer) return;
        }
        else
        {
            if (newCapacity <= base::m_capacity) return;
        }

        base temp = {base::m_allocator, newCapacity};

        if (std::is_nothrow_move_constructible<value_type>::value)
        {
            size_type first = 0;
            size_type current = 0;

            try
            {
                for (size_t i = 0; i < size(); i++)
                {
                    alloc_traits::construct(base::m_allocator, temp.m_data + current, std::move(this->operator[](i)));
                    current++;
                }
            }
            catch (...)
            {
                for (; first != current; first++)
                {
                    alloc_traits::destroy(base::m_allocator, base::m_data + first);
                }
                m_headIndex = 0;
                m_tailIndex = 0;
                throw;
            }
        }
        else
        {
            std::uninitialized_copy(begin(), end(), temp.m_data);
        }
        
        m_headIndex = this->size();
        m_tailIndex = 0;

        base::swap(*this, temp);
    }

    /// @brief Inserts an element in the back of the buffer. 
    /// @note If buffer would get full after the operation, function allocates more memory.
    /// @throw Can throw std::bad_alloc.
    /// @param val Element to insert.
    /// @pre T must satisfy CopyInsertable.
    /// @post All iterators are invalidated. If more memory is allocated, all pointers and references are invalidated.
    /// @exception If the copy constructor of value_type throws, behaviour is undefined. Otherwise in case of any exception this function has no effect (Strong Exception Guarantee).
    /// @details Amortized constant complexity.
    void push_front(const value_type& val)
    {
        emplace_front(val);
    }

    /// @brief Inserts an element in the back of the buffer by move if move constructor is provided by value_type.
    /// @note If buffer would get full after the operation, allocates more memory.
    /// @throw Can throw std::bad_alloc.
    /// @param val Rvalue reference to the element to insert.
    /// @pre value_type needs to satisfy MoveInsertable.
    /// @post All iterators are invalidated. If more memory is allocated, all pointers and references are invalidated.
    /// @exception If the move constructor of value_type throws, behaviour is undefined. Otherwise in case of any exception this function has no effect (Strong Exception Guarantee).
    /// @details Amortized constant complexity.
    void push_front(value_type&& val)
    {
        emplace_front(std::move(val));
    }

    /// @brief Inserts an element in the back of the buffer.
    /// @param val Value of T to be appended.
    /// @note If buffer would get full after the operation, allocates more memory.
    /// @throw Can throw std::bad_alloc.
    /// @exception If the copy constructor of value_type throws, behaviour is undefined. Otherwise in case of exception this function has no effect (Strong Exception Guarantee).
    /// @pre value_type must satisfy CopyInsertable.
    /// @post If more memory is allocated all pointers, iterators and references are invalidated.
    /// @details Amoprtized constant complexity.
    void push_back(const value_type& val)
    {
        emplace_back(val);
    }

    /// @brief Inserts an element in the back of the buffer by move if move constructor is provided for value_type.
    /// @note  If buffer would get full after the operation more memory is allocated.
    /// @param val Rvalue reference to the value to be appended.
    /// @throw Can throw std::bad_alloc.
    /// @exception If the move/copy constructor of value_type throws, behaviour is undefined. Otherwise in case of any exception this function has no effect (Strong Exception Guarantee).
    /// @pre value_type needs to satisfy MoveInsertable.
    /// @post If more memory is allocated all pointers, iterators and references are invalidated.
    /// @details Amortized constant complexity.
    void push_back(value_type&& val)
    {
        emplace_back(std::move(val));
    }

    /// @brief Remove the first element in the buffer.
    /// @pre Buffers size > 0, otherwise behaviour is undefined.
    /// @post All iterators, pointers and references are invalidated.
    /// @details Constant complexity.
    void pop_front() noexcept
    {
        alloc_traits::destroy(base::m_allocator, base::m_data + m_tailIndex);
        increment(m_tailIndex);
    }

    /// @brief Erase an element from the logical back of the buffer.
    /// @pre Buffers size > 0, otherwise behaviour is undefined.
    /// @post All pointers and references are invalidated. Iterators persist except end() - 1 iterator is invalidated (it becomes new past-the-last iterator).
    /// @details Constant complexity.
    void pop_back() noexcept
    {
        decrement(m_headIndex);
        alloc_traits::destroy(base::m_allocator, base::m_data + m_headIndex);

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
        return base::m_data[m_tailIndex];
    }

    /// @brief Returns a reference to the first element in the buffer. Behaviour is undefined for empty buffer.
    /// @return const_reference to the first element.
    /// @details Constant complexity.
    const_reference front() const noexcept
    {
        return base::m_data[m_tailIndex];
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
            return base::m_data[base::m_capacity - 1];
        }
        return base::m_data[m_headIndex-1];
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
            return base::m_data[base::m_capacity - 1];
        }
        return base::m_data[m_headIndex-1];
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
     
    explicit ring_buffer(base&& rBufBase) : base(std::forward<base>(rBufBase)), m_headIndex(0), m_tailIndex(0)
    {
    }

    void destroy_elements() noexcept
    {
        for_each(begin(), end(), [this](T& elem) { alloc_traits::destroy(base::m_allocator, &elem); });
    }


    ///@note MUST HAVE NOTHROW MOVE CONSTRUCTION!!! Otherwise in case of exception leaves container in unspecified state.
    template<typename InputIt, typename NoThrowForwardIt>
    NoThrowForwardIt _uninitialized_move(InputIt first, InputIt last, NoThrowForwardIt dest_first)
    {
        NoThrowForwardIt current = dest_first;

        try
        {
            for (size_t i = 0; first != last; ++first, (void)++current)
            {
                alloc_traits::construct(base::m_allocator, current, std::move(*first));
            }
            
        }
        catch (...)
        {
            for (; dest_first != current; dest_first)
            {
                alloc_traits::destroy(base::m_allocator, dest_first);
            }

            throw;
        }
        return current;
    }

    /// @brief Reserves more memory if needed for an increase in size. If more memory is needed, allocates (capacity * 1.5) or if that is not enough (capacity * 1.5 + increase).
    /// @param increase Expected increase in size of the buffer, based on which memory is allocated.
    /// @details Linear complexity in relation to buffer size if more memory needs to be allocated, otherwise constant complexity.
    /// @exception May throw std::bad_alloc. If any exception is thrown this function does nothing. Strong exception guarantee.
    /// @note This function should be called before increasing the size of the buffer.
    void validateCapacity(size_t increase)
    {
        if (base::m_capacity > size() + increase + allocBuffer) return;

        reserve(base::m_capacity / 2 + base::m_capacity + allocBuffer);
    }

    /// @brief Base function for inserting elements by value and amount.
    /// @tparama U value type of the inserted element.
    /// @param pos Iterator pointing to the element where after insert new element will exist.
    /// @param count Amount of elements to insert.
    /// @param value Universal reference of value to insert.
    /// @pre T Must satisfy CopyInsertable or MoveInsertable.
    /// @return Returns iterator pointing to the first element inserted.
    /// @throw Might throw std::bad_alloc from allocating memory, or something from T's move/copy constructor.
    /// @exception  If any exception is thrown, function has no effect (Strong exception guarantee).
    /// @details Linear complexity in relation to buffer size and inserted elements. O(n).
    template<typename U>
    iterator insertBase(const_iterator pos, const size_type count, U&& value)
    {
        base tempCore(base::m_allocator, base::m_capacity < size() + count + allocBuffer ? base::m_capacity * 3 / 2 : base::m_capacity);
        ring_buffer temp(std::move(tempCore));

        // Copy elements up to pos
        std::uninitialized_copy(cbegin(), pos, temp.m_data);
        temp.m_headIndex = std::distance(cbegin(), pos);
        
        // Insert the element(s)
        for (size_type i = 0; i < count; i++)
        {
            alloc_traits::construct(temp.m_allocator, temp.m_data + temp.m_headIndex, std::forward<U>(value));
            ++temp.m_headIndex;
        }

        // Copy elements after pos
        std::uninitialized_copy(pos, cend(), temp.m_data + temp.m_headIndex);
        temp.m_headIndex += std::distance(pos, cend());

        // Swap whole buffer.
        swap(temp);

        return iterator(this, pos.getIndex());

    }

    /// @brief Base function for inserting elements from a range of [rangeBegin, rangeEnd).
    /// @tparam OutputIt type of the source ranges output iterator.
    /// @param pos Iterator pointing to the element where after insert new element will exist.
    /// @param rangeBegin Iterator pointing to the first element of the range.
    /// @param rangeEnd Iterator pointing past the last element to be inserted.
    /// @return Returns iterator pointing to the first element inserted.
    /// @pre value_type must meet CopyInsertable. InputIt must be deferencable to value_type, and incrementing rangeBegin possibly multiple times should reach rangeEnd. Otherwise behaviour is undefined.
    /// @post Each iterator in [rangeBegin, rangeEnd) is dereferenced once.
    /// @throw Might throw std::bad_alloc from allocating memory and rotate(), or something from T's move/copy constructor.
    /// @exception  If any exception is thrown, invariants are retained. (Basic Exception guarantee).
    /// @details Linear Complexity in relation to amount of inserted elements (O(n*2) from element construction and rotation).
    template<typename OutputIt>
    iterator insertRangeBase(const_iterator pos, OutputIt rangeBegin, OutputIt rangeEnd)
    {
        const auto amount = std::distance<OutputIt>(rangeBegin, rangeEnd);

        base tempCore(base::m_allocator, base::m_capacity < size() + amount + allocBuffer ? base::m_capacity * 3 / 2 : base::m_capacity);
        ring_buffer temp(std::move(tempCore));

        std::uninitialized_copy(cbegin(), pos, temp.m_data);
        temp.m_headIndex = std::distance(cbegin(), pos);


        for (; rangeBegin != rangeEnd; ++rangeBegin)
        {
            alloc_traits::construct(temp.m_allocator, temp.m_data + temp.m_headIndex, *rangeBegin);
            ++temp.m_headIndex;
        }

        // Copy elements after pos
        std::uninitialized_copy(pos, cend(), temp.m_data + temp.m_headIndex);
        temp.m_headIndex += std::distance(pos, cend());

        // Swap whole buffer.
        swap(temp);

        return iterator(this, pos.getIndex());
    }

    /// @brief Base function for erasing elements from the buffer. Erases always as a range.
    /// @param first Iterator pointing to the first element of the range to erase.
    /// @param last Iterator pointing to past the last element to erase.
    /// @return Returns an iterator pointing to the element immediately after the erased elements.
    /// @pre First and last must be valid iterators to *this.
    /// @exception If value_types swap is NoThrow, function is noexcept. If swap throws, behavior is undefined.
    iterator eraseBase(const_iterator first, const_iterator last)
    {
        iterator returnIt(this, first.getIndex());

        const auto diff = std::distance(first, last);

        if (diff > 0)
        {
            auto tempIt(returnIt);

            // Swap to be erased elements to the end...
            for (; tempIt + diff < end(); tempIt++)
            {
                std::swap(*(tempIt + diff), *tempIt);
            }

            // ... and destroy them.
            for (; tempIt < end(); tempIt++)
            {
                alloc_traits::destroy(base::m_allocator, &*(tempIt));
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
        if(index >= base::m_capacity)
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
            index = base::m_capacity - 1;
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