#ifndef DYNAMIC_RINGBUFFER_HPP
#define DYNAMIC_RINGBUFFER_HPP

#include <memory>
#include <algorithm>
#include <limits>
#include <vector>

// Forward declaration of _rBuf_const_iterator.
template<class _rBuf>
class _rBuf_const_iterator;


/// @brief Dynamic Ringbuffer is a dynamically growing std::container with support for queue, stack and priority queue adaptor functionality. 
/// @tparam T Type of the elements. Must meet the requirement EmplaceCostructible.
/// @tparam Allocator Allocator used for (de)allocation and (de)construction. Defaults to std::allocator<T>
template<typename T, typename Allocator = std::allocator<T>> 
class ring_buffer
{

public:

    using value_type = T;
    using allocator_type = Allocator;
    using reference = T&;
    using const_reference= const T&;
    using pointer = typename T*;
    using const_pointer= typename const T*;

    using difference_type = ptrdiff_t;
    using size_type = std::size_t;

//========================================
// Iterators
//========================================

    /// @brief Custom iterator class.
    /// @tparam T Type of the element what iterator points to.
    template<class _rBuf>
    class _rBuf_iterator
    {

    public:
        using iterator_category = std::random_access_iterator_tag;

        using value_type = typename _rBuf::value_type;
        using difference_type = typename _rBuf::difference_type;
        using pointer = typename _rBuf::pointer;
        using reference = value_type&;

    public:

        /// @brief  Const iterator is friend to enable conversion.
        friend class _rBuf_const_iterator<_rBuf>;

        /// @brief Default constructor
        _rBuf_iterator() : m_container(nullptr), m_logicalIndex(0) {}

        /// @brief Constructor.
        /// @param container Pointer to the ring_buffer element which owns this iterator.
        /// @param index Index pointing to the logical element of the ring_buffer.
        explicit _rBuf_iterator(_rBuf* container, difference_type index) : m_container(container), m_logicalIndex(index) {}

        /// @brief Dereference operator
        /// @return  Returns the object the iterator is currently pointing to.
        reference operator*() const
        {
            //TODO: bounds checking and value-initialization.
            return (*m_container)[m_logicalIndex];
        }

        /// @brief Pointer operator.
        /// @return Returns a pointer to the object the iterator is currently pointing to.
        pointer operator->() const
        {
            //TODO: check value initialization and bounds.
            return &(*m_container)[m_logicalIndex];
        }

        /// @brief Prefix increment
        /// @note The index of the iterator can be incremented over the border of its owning buffer. The buffer converts logical index to correct element.
        /// See Iterator::operator* and ring_buffer::operator[].
        _rBuf_iterator& operator++()
        {
            ++m_logicalIndex;
            return (*this);
        }

        /// @brief Postfix increment
        /// @param  int empty parameter to guide overload resolution.
        /// @note The index of the iterator can be incremented over the border of its owning buffer. The buffer converts logical index to correct element.
        ///  See Iterator::operator* and ring_buffer::operator[].
        _rBuf_iterator operator++(int)
        {
            auto temp(*this);
            ++m_logicalIndex;
            return temp;
        }

        /// @brief prefix decrement
        _rBuf_iterator& operator--()
        {
            --m_logicalIndex;
            if (m_logicalIndex < 0)
            {
                m_logicalIndex = m_container->capacity() - 1;
            }
            return(*this);
        }

        /// @brief Postfix decrement
        /// @param  int empty parameter to guide overload resolution.
        _rBuf_iterator operator--(int)
        {
            auto temp(*this);
            --m_logicalIndex;
            if (m_logicalIndex < 0)
            {
                m_logicalIndex = m_container->capacity() - 1;
            }
            return temp;
        }

        /// @brief Moves iterator forward.
        /// @param offset Amount of elements to move.
        _rBuf_iterator& operator+=(difference_type offset)
        {
            const auto capacity = m_container->capacity();
            // TODO : make this better. Absolutely terrible
            if (offset < 0)
            {
                offset = abs(offset) % capacity;
                if (offset > m_logicalIndex)
                {
                    m_logicalIndex = capacity - (offset - m_logicalIndex);
                }
                else
                {
                    m_logicalIndex = m_logicalIndex - offset;
                }
            }
            else
            {
                offset = offset % capacity;
                m_logicalIndex += offset;
                if (m_logicalIndex >= capacity)
                {
                    m_logicalIndex -= capacity;
                }
            }
            return (*this);
        }

        /// @brief Move iterator forward by specified amount.
        /// @param movement Amount of elements to move the iterator.
        _rBuf_iterator operator+(const difference_type offset) const
        {
            _rBuf_iterator temp(m_container, m_logicalIndex);
            return (temp += offset);
        }

        // TODO
        friend _rBuf_iterator operator+(difference_type offset, _rBuf_iterator iter)
        {
            auto temp = iter;
            temp += offset;
            return temp;
        }

        /// @brief Returns an iterator that points to an element, which is the current element decremented by the given offset.
        /// @param offset The number of positions to move the iterator backward.
        /// @return An iterator pointing to the element that is offset positions before the current element.
        /// @note Undefined behavior for negative offset.
        _rBuf_iterator& operator-=(const difference_type offset)
        {
            return (*this += -offset);
        }

        /// @brief Returns an iterator that points to an element, which is the current element decremented by the given offset.
        /// @param offset The number of positions to move the iterator backward.
        /// @return An iterator pointing to the element that is offset positions before the current element.
        _rBuf_iterator& operator-(const difference_type offset) const
        {
            _rBuf_iterator temp(m_container, m_logicalIndex);
            return (temp -= offset);
        }

        /// @brief Gets distance between two iterators.
        /// @param iterator Iterator to get distance to.
        /// @return Amount of elements between the iterators.
        difference_type operator-(const _rBuf_iterator& other) const
        {
            //TODO: compatibility check
            return (m_logicalIndex - other.m_logicalIndex);
        }

        /// @brief Index operator.
        /// @param offset The offset from iterator.
        /// @return Return object pointer by the iterator with an offset.
        reference operator[](const difference_type offset) const
        {
            return m_container->operator[](m_logicalIndex + offset);
        }

        /// @brief Comparison operator== overload
        /// @param other iterator to compare
        /// @return True if iterators point to same element in same container. Does not compare c-qualification.
        /// @note As a matter of fact, just blatantly calls const-iterators operator, to access private members via friendness. All non-const iterators are converted to const for this comparison.
        template<typename iterator>
        bool operator==(const iterator& other) const
        {
            return _rBuf_const_iterator<_rBuf>(*this).operator==(other);
        }

        /// @brief Comparison operator != overload
        /// @param other iterator to compare
        /// @return ture if underlying pointers are not the same
        template<typename iterator>
        bool operator!=(const iterator& other) const
        {

            return !(*this == other);
        }

        /// @brief Comparison operator < overload
        /// @param other iterator to compare
        /// @return true if other is larger.
        bool operator<(const _rBuf_iterator& other) const
        {
            //TODO:compatibility check
            return (m_logicalIndex < other.m_logicalIndex);
        }

        /// @brief Comparison operator > overload
        /// @param other iterator to compare
        /// @return true if other is smaller.
        bool operator>(const _rBuf_iterator& other) const
        {
            //TODO:compatibility check
            return (other.m_logicalIndex < m_logicalIndex);
        }

        bool operator<=(const _rBuf_iterator& other) const
        {
            //TODO:compatibility check
            return: (!(other < m_logicalIndex));
        }

        bool operator>=(const _rBuf_iterator& other) const
        {
            return (!(m_logicalIndex < other.m_logicalIndex));
        }

        /// @brief Custom assingment operator overload.
        /// @param index Logical index of the element to set the iterator to.
        /// @note Undefined behaviour for negative index.
        _rBuf_iterator& operator=(size_t index)
        {
            m_logicalIndex = index;
            return (*this);
        };

        difference_type getIndex() noexcept
        {
            return m_logicalIndex;
        }

    private:
        // The parent container.
        _rBuf* m_container;

        // The iterator does not point to any memory location, but is interfaced to the Ring Buffer via an index which is the logical index
        // to an element. Logical index 0 is the first element in the buffer and last is size - 1.
        difference_type m_logicalIndex;
    };

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

        /// @brief Conversion constructor
        /// @param const_iterator const iterator to construct from.
        _rBuf_const_iterator(const _rBuf_iterator<_rBuf>& iterator) : m_container(iterator.m_container), m_logicalIndex(iterator.m_logicalIndex) {}

        /// @brief Conversion assingment from non-const iterator
        /// @param iterator non-const iterator.
        /// @return Returns the new object by reference
        _rBuf_const_iterator& operator=(_rBuf_iterator<_rBuf>& iterator)
        {
            if (m_logicalIndex == iterator.m_logicalIndex && m_container == iterator.m_container)
            {
                return *this;
            }

            m_logicalIndex = iterator.m_logicalIndex;
            m_container = iterator.m_container;
            return *this;
        }

        /// @brief Arrow operator.
        /// @return pointer.
        pointer operator->() const
        {
            //TODO: check value initialization and bounds. Shhhh......
            return &(*m_container)[m_logicalIndex];
        }

        /// @brief Postfix increment
        /// @note The index of the iterator can be incremented over the border of its owning buffer. The buffer converts logical index to correct element.
        /// See Iterator::operator* and ring_buffer::operator[].
        _rBuf_const_iterator& operator++()
        {
            m_logicalIndex++;

            return (*this);
        }

        /// @brief Postfix increment
        /// @param  int empty parameter to guide overload resolution.
        /// @note The index of the iterator can be incremented over the border of its owning buffer. The buffer converts logical index to correct element.
        /// See Iterator::operator* and ring_buffer::operator[].
        _rBuf_const_iterator operator++(int)
        {
            auto temp(*this);
            ++m_logicalIndex;

            return temp;
        }

        /// @brief Prefix decrement
        _rBuf_const_iterator& operator--()
        {
            --m_logicalIndex;
            if (m_logicalIndex < 0)
            {
                m_logicalIndex = m_container->capacity() - 1;
            }
            return(*this);
        }

        /// @brief Postfix decrement
        /// @param  int empty parameter to guide overload resolution.
        _rBuf_const_iterator operator--(int)
        {
            //TODO:    check value initialization and "under" begin() decrement.
            auto temp(*this);
            --m_logicalIndex;
            if (m_logicalIndex < 0)
            {
                m_logicalIndex = m_container->capacity() - 1;
            }
            return temp;
        }

        /// @brief Moves iterator.
        /// @param offset Amount of elements to move. Negative values move iterator backwards.
        _rBuf_const_iterator& operator+=(difference_type offset)
        {
            const auto capacity = m_container->capacity();
            // TODO : make this better. Absolutely terrible
            if (offset < 0)
            {
                offset = abs(offset) % capacity;
                if (offset > m_logicalIndex)
                {
                    m_logicalIndex = capacity - (offset - m_logicalIndex);
                }
                else
                {
                    m_logicalIndex = m_logicalIndex - offset;
                }
            }
            else
            {
                offset = offset % capacity;
                m_logicalIndex += offset;
                if (m_logicalIndex >= capacity)
                {
                    m_logicalIndex -= capacity;
                }
            }
            return (*this);
        }

        /// @brief Move iterator forward by specified amount.
        /// @param movement Amount of elements to move the iterator.
        _rBuf_const_iterator operator+(const difference_type offset) const
        {
            _rBuf_const_iterator temp(m_container, m_logicalIndex);
            return (temp += offset);
        }

        friend _rBuf_const_iterator operator+(const difference_type offset, _rBuf_const_iterator iter) noexcept
        {
            auto temp = iter;
            temp += offset;
            return temp;
        }

        /// @brief Returns an iterator that points to an element, which is the current element decremented by the given offset.
        /// @param offset The number of positions to move the iterator backward.
        /// @return An iterator pointing to the element that is offset positions before the current element.
        /// @note Undefined behaviour for negative offset.
        _rBuf_const_iterator& operator-=(const difference_type offset)
        {
            return (*this += -offset);
        }

        /// @brief Returns an iterator that points to an element, which is the current element decremented by the given offset.
        /// @param offset The number of positions to move the iterator backward.
        /// @return An iterator pointing to the element that is offset positions before the current element.
        /// @note Undefined behaviour for negative offset.
        _rBuf_const_iterator operator-(const difference_type offset) const
        {
            _rBuf_const_iterator temp(m_container, m_logicalIndex);
            return (temp -= offset);
        }

        /// @brief Gets distance between two iterators.
        /// @param iterator Iterator to get distance to.
        /// @return Amount of elements between the iterators.
        difference_type operator-(const _rBuf_const_iterator& other) const
        {
            return (m_logicalIndex - other.m_logicalIndex);
        }

        /// @brief Index operator.
        /// @param offset The offset from iterator.
        /// @return Return reference to element pointed by the iterator with offset.
        reference operator[](const difference_type offset) const
        {
            return m_container->operator[](m_logicalIndex + offset);
        }

        /// @brief Comparison operator== overload
        /// @param other iterator to compare
        /// @return True if iterators point to same element in same container. Does not compare c-qualification.
        bool operator==(const _rBuf_const_iterator& other) const
        {
            const auto pointsToSame = (m_logicalIndex == other.m_logicalIndex);
            return pointsToSame && m_container == other.m_container;
        }

        /// @brief Comparison operator != overload
        /// @param other iterator to compare
        /// @return ture if underlying pointers are not the same
        bool operator!=(const _rBuf_const_iterator& other) const
        {
            return !(m_logicalIndex == other.m_logicalIndex && m_container == other.m_container);
        }

        /// @brief Comparison operator < overload
        /// @param other iterator to compare
        /// @return true if other is larger.
        bool operator<(const _rBuf_const_iterator& other) const
        {
            return (m_logicalIndex < other.m_logicalIndex);
        }

        /// @brief Comparison operator > overload
        /// @param other iterator to compare
        /// @return true if other is smaller.
        bool operator>(const _rBuf_const_iterator& other) const
        {
            //TODO:compatibility check
            return (other.m_logicalIndex < m_logicalIndex);
        }

        bool operator<=(const _rBuf_const_iterator& other) const
        {
            //TODO:compatibility check
            return: (!(other < m_logicalIndex));
        }

        bool operator>=(const _rBuf_const_iterator& other) const
        {
            return (!(m_logicalIndex < other.m_logicalIndex));
        }

        /// @brief Custom assingment operator overload.
        /// @param index Logical index of the element which point to.
        _rBuf_const_iterator& operator=(size_t index) const
        {
            m_logicalIndex = index;
            return (*this);
        };

        /// @brief Dereference operator.
        /// @return Object pointed by iterator.
        reference operator*() const
        {
            return (*m_container)[m_logicalIndex];
        }

        /// @brief Returns the logical index of the element the iterator is pointing to.
        difference_type getIndex() noexcept
        {
            return m_logicalIndex;
        }

    private:
        // The parent container.
        const _rBuf* m_container;

        // The iterator does not point to any memory location, but is interfaced to the Ring Buffer via an index which is the logical index
        // to an element. Logical index 0 is the first element in the buffer and last is size - 1.
        difference_type m_logicalIndex;
    };


//============================
// End of iterators
//============================

    using iterator = _rBuf_iterator<ring_buffer<T>>;
    using const_iterator = _rBuf_const_iterator<ring_buffer<T>>;


    /// @brief Default constructor. Constructs to 0 size and 2 capacity.
    /// @throw Might throw std::bad_alloc if there is not enough memory available for allocation.
    /// @post this->empty() == true.
    /// @exception If an exception is thrown, this function has no effect. Strong exception guarantee.
    /// @details Constant complexity.
    ring_buffer() : ring_buffer(0, allocator_type())
    {
    }

    /// @brief Constructs the container with a custom allocator to 0 size and 2 capacity.
    /// @param alloc Custom allocator for the buffer.
    /// @post this->empty() == true.
    /// @throw Might throw std::bad_alloc if there is not enough memory available for allocation.
    /// @exception If an exception is thrown, this function has no effect. Strong exception guarantee.
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
    /// @exception 
    /// @details Linear complexity in relation to amount of constructed elements.
    ring_buffer(size_type count, const_reference val, const allocator_type& alloc = allocator_type()) : m_headIndex(0), m_tailIndex(0), m_capacity(count + 2), m_allocator(alloc)
    {
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
    /// @exception If an exception is thrown bad_alloc is thrown trong exception guarantee.
    /// @details Linear complexity in relation to count.
    explicit ring_buffer(size_type count, const allocator_type& alloc = allocator_type()) : m_headIndex(0), m_tailIndex(0), m_capacity(count + 2), m_allocator(alloc)
    {
        m_data = m_allocator.allocate(m_capacity);
        for (size_t i = 0; i < count; i++)
        {
            m_allocator.construct(m_data + i);
            increment(m_headIndex);
        }
    }

    /// @brief Construct the buffer from range [begin,end).
    /// @param beginIt Iterator to first element of range.
    /// @param endIt Iterator pointing to past-the-last element of range.
    /// @pre T must satisfy CopyInsertable. InputIt must be dereferenceable to value_type and end must be reachable from begin by (possibly repeatedly) incrementing begin. Otherwise behaviour is undefined.
    /// @exception Might throw std::bad_alloc if there is not enough memory available for allocation.
    /// @details Linear complexity in relation to the size of the range.
    /// @note Behavior is undefined if elements in range are not valid.
    template<typename InputIt>
    ring_buffer(InputIt beginIt, InputIt endIt)
    {
        const auto size = std::distance<InputIt>(beginIt , endIt);
        m_capacity = size + 2;
        m_headIndex = size;
        m_tailIndex = 0;
        m_data = m_allocator.allocate(m_capacity);

        for (size_t i = 0; i < size; i++)
        {
            m_allocator.construct(m_data + i, *(beginIt + i));
        }
    }

    /// @brief Initializer list contructor.
    /// @param init Initializer list to initialize the buffer from.
    /// @pre T must satisfy CopyInsertable.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @note Allocates memory for 2 extra elements.
    /// @details Linear complexity in relation to initializer list size.
    explicit ring_buffer(std::initializer_list<T> init) : ring_buffer(init.begin(),init.end())
    {
    }

    /// @brief Copy constructor.
    /// @param rhs Reference to a RingBuffer to create a copy from.
    /// @pre T must meet CopyInsertable.
    /// @post this == ring_buffer(rhs).
    /// @throw Might throw std::bad_alloc if there is not enough memory for memory allocation.
    /// @details Linear complexity in relation to buffer size.
    ring_buffer(const ring_buffer& rhs) : m_capacity(rhs.m_capacity), m_headIndex(rhs.m_headIndex), m_tailIndex(rhs.m_tailIndex)
    {
        m_data = m_allocator.allocate(m_capacity);

        for (size_t i = 0; i < rhs.size(); i++)
        {
            m_allocator.construct(&this->operator[](i), rhs[i]);
        }
    }

    /// @brief Move constructor.
    /// @param other Rvalue reference to other buffer.
    /// @details Constant complexity.
    ring_buffer(ring_buffer&& other) noexcept
    {
        m_data = std::exchange(other.m_data, nullptr);
        m_capacity = std::exchange(other.m_capacity, 0);
        m_headIndex = std::exchange(other.m_headIndex, 0);
        m_tailIndex = std::exchange(other.m_tailIndex, 0);
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
    /// @throw Can throw std::bad_alloc if allocation is required but not enough free memory available.
    /// @exception 
    iterator insert(const_iterator pos, const value_type& value)
    {
        validateCapacity(1);

        return insertBase(pos, 1, value);
    }

    /// @brief Inserts an element to the buffer.
    /// @param pos Iterator where the the element should be inserted
    /// @param value Value to insert.
    /// @return Iterator that pos to the inserted element.
    /// @throw Can throw std::bad_alloc, or something from element construction. 
    /// @exception 
    iterator insert(const_iterator pos, value_type&& value)
    {
        validateCapacity(1);

        return insertBase(pos, 1, std::move(value));
    }

    /// @brief Inserts an element to the buffer.
    /// @param pos Iterator where the the element should be inserted
    /// @param value Value to insert. T must meet the requirements of CopyInsertable.
    /// @return Iterator that pos to the inserted element.
    /// @throw Can throw std::bad_alloc, or something from element construction.
    iterator insert(const_iterator pos, const size_type amount, const value_type& value)
    {
        validateCapacity(1);

        return insertBase(pos, amount, value);
    }

    /// @brief Inserts a range of elements into the buffer to a specific position.
    /// @tparam sourceIterator Type of iterator for the range.
    /// @param pos Iterator to the position where range will be inserted to.
    /// @param begin Iterator to first element of the range.
    /// @param sourceEnd Iterator past the last element of the range.
    /// @return Returns an iterator to an element in the buffer which is copy of the first element in the range.
    /// @pre pos must be a valid dereferenceable const_iterator within the container. Otherwise behavior is undefined.
    /// @note Type of elements in the range need to be convertible to T. Elements of range must not be part of *this.
    /// @exception Can throw std::bad_alloc, TODO and is this strong or basic or weak guarantee? shift should not throw but... it could?
    iterator insert(const_iterator pos, const_iterator sourceBegin, const_iterator sourceEnd)
    {
        return insertRangeBase(pos,sourceBegin, sourceEnd);
    }

    /// @brief Inserts initializer list to buffer.
    /// @param pos Iterator where the list will be inserted.
    /// @param list Initiliazer list to insert.
    /// @pre pos must be a valid dereferenceable const_iterator within the container. Otherwise behavior is undefined.
    /// @return Returns Iterator to the first element inserted, or the element pointed by pos if the initializer list was empty.
    iterator insert(const_iterator pos, std::initializer_list<T> list)
    {
        return insertRangeBase(pos, list.begin(), list.end());
    }

    template<class... Args>
    iterator emplace(const_iterator pos, Args&&... args)
    {
        // Shift requires that enough memory is allocated, allocate enough for size + the emplaced element.
        while(m_capacity - 1 <= size() + 1)
        {
            reserve(m_capacity * 1.5);
        }

        size_t index = pos.getIndex();

        // If pos is not end iterator.
        if( index < size())
        {
            shift(pos, 1);
        }

        m_allocator.construct(&(*pos), std::forward<Args>(args)...);
        increment(m_headIndex);
        return iterator(this, pos.getIndex());
    }

    /// @brief Erase an element at a given position.
    /// @param pos Pointer to the element to be erased.
    /// @pre pos must be a valid dereferenceable const_iterator within the container. Otherwise behavior is undefined.
    /// @pre T Must be MoveAssignable.
    /// @return Returns an iterator that was immediately following the ereased element. If the erased element was last in the buffer, returns a pointer to the new last element.
    iterator erase(const_iterator pos)
    {
        auto posIndex = pos.getIndex();
        const auto endIndex = end().getIndex();
        iterator it(this, posIndex);

        // Destroy element and move remaining elements to fill the void.
        m_allocator.destroy(&pos);
        decrement(m_headIndex);
        for(size_t i = 0; i + posIndex + 1 < endIndex; i++)
        {
            // Move elements back one by one.
            it[i] = std::move(it[i + 1]);
        }

        if(posIndex == endIndex)
        {
            return end();
        }

        return it;
    }

    /// @brief Erase the specified elements from the container according to the range [first,last).
    /// @param first iterator to the first element to erase.
    /// @param last iterator past the last element to erase.
    /// @return Returns an iterator to the element that was immediately following the erased elements. If last == end(), then end() is returned.
    iterator erase(const_iterator first, const_iterator last)
    {
        auto rangeSize = last - first - 1;
        const auto isLast  = (last == end());
        iterator it(this, first.getIndex());

        // Destroy elements
        for(size_t i = 0; i <= rangeSize; i++)
        {
            it = erase(first);
        }

        return it;
    }

    /// @brief Destroys all elements in a buffer. Does not modify capacity.
    /// @post All existing references, pointers and iterators are to be considered invalid.
    /// @details Linear complexity in relation to size of the buffer.
    void clear() noexcept
    {
        for(; m_tailIndex < m_headIndex ; m_tailIndex++)
        {
            m_allocator.destroy(&m_data[m_tailIndex]);
        }

        m_headIndex = 0;
        m_tailIndex = 0;
    }



    /// @brief Replaces the elements in the buffer with copy of [sourceBegin, sourceEnd)
    /// @pre T is CopyConstructible and [sourceBegin, sourceEnd) are not in the buffer.
    /// @post All existing references, pointers and iterators are to be considered invalid.
    /// @note Undefined behaviour if either source iterator is an iterator to *this.
    void assign(const_iterator sourceBegin, const_iterator sourceEnd)
    {
        clear();
        for (; sourceBegin != sourceEnd; sourceBegin++)
        {
            push_back(*sourceBegin);
        }
    }

    /// @brief Replaces the elements in the buffer.
    /// @param list Initializer list containing the elements to replace the existing ones.
    /// @pre T is CopyInsertable.
    /// @post All existing references, pointers and iterators are invalidated.
    /// @note Leaves capacity of the buffer unchanged.
    void assign(std::initializer_list<T> list)
    {
        clear();
        auto begin = list.begin();
        const auto end = list.end();
        for (; begin != end; begin++)
        {
            push_back(*begin);
        }
    }

    /// @brief Replaces the elements in the buffer with given value.
    /// @param amount Size of the buffer after the assignment.
    /// @param value Value of all elements after the assignment.
    void assign(const size_type amount, value_type value)
    {
        clear();
        for(auto i = 0; i < amount; i++)
        {
            push_back(value);
        }
    }

    /// @brief Copy assignment operator.
    /// @param other Ringbuffer to be copied.
    /// @return Returns reference to the left hand side RungBuffer after swap.
    /// @post *this == other.
    /// @details Constant complexity.
    /// @exception If any exception is thrown, this function has no effect (Strong Exception Guarantee).
    ring_buffer& operator=(const ring_buffer& other)
    {
        ring_buffer copy(other);
        copy.swap(*this);
        return *this;
    }

    /// @brief Move assignment operator.
    /// @param other Rvalue ref to other buffer.
    /// @pre T is MoveConstructible or CopyConstuctible.
    /// @post If T is MoveConstructible, *this has values other had before the assignment. Otherwise *this == other.
    /// @return Reference to the buffer.
    /// @exception If T is not MoveConstructible, and a throwing CopyConstructor which throws this function has no effect (Strong Exception Guarantee).
    /// @details Constant complexity.
    ring_buffer& operator=(ring_buffer&& other)
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
    /// @param logicalIndex Index of the element.
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

    /// @brief Get a specific element of the buffer.
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
    /// @det
    size_type size() const noexcept
    {
        if(m_headIndex < m_tailIndex)
        {
            return m_headIndex + m_capacity - m_tailIndex;
        }

        return m_headIndex - m_tailIndex;
    }

    /// TODO what is this, is this needed somewhere?
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
    /// @throw Can throw std::bad_alloc. 
    /// @exception If T's move (or copy if T has no move) constructor throws, behaviour is undefined. Otherwise Stong Exception Guarantee.
    /// @notes All references and pointers are invalidated (iterators stay valid).
    /// @details Linear complexity in relation to size of the buffer.
    void reserve(size_type newCapacity, bool enableShrink = false)
    {
        if (!enableShrink)
        {
            if (newCapacity <= m_capacity) return;
        }
        else
        {
            if (newCapacity < size() + 2) return;
        }

        // Data pointer for "do stuff and swap" idiom to provide strong excepion guarantee.
        auto tempData = m_allocator.allocate(newCapacity);

        for (size_t i = 0; i < size(); i++)
        {
            m_allocator.construct(tempData + i, std::move(this->operator[](i)));
            m_allocator.destroy(&this->operator[](i));
        }

        m_allocator.deallocate(m_data, size());

        // If memory was allocated, the buffer matches beginning of physical memory.
        m_headIndex = size();
        m_tailIndex = 0;
        m_capacity = newCapacity;

        // Assings the data from temp to original buffer. The resources from temp will be released when function goes out of scope.
        std::swap(tempData, m_data);
    }

    /// @brief Inserts an element in the back of the buffer. If buffer would get full after the operation, allocates more memory.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @param val Element to insert.  Needs to be CopyConstructible.
    /// @note All iterators are invalidated. If allocation happens, all pointers and references are invalidated.
    /// @exception If the move/copy constructor of value_type throws, behaviour is undefined. Otherwise in case of exception this function has no effect (Strong Exception Guarantee).
    /// @details Constant complexity.
    void push_front(value_type val)
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
    /// @pre T needs to satisfy MoveInsertable or CopyInsertable.
    /// @note All iterators are invalidated. If allocation happens, all pointers and references are invalidated.
    /// @exception If the move/copy constructor of value_type throws, behaviour is undefined. Otherwise in case of exception this function has no effect (Strong Exception Guarantee).
    /// @details Constant complexity.
    void push_front(value_type&& val)
    {
        validateCapacity(1);

        // Decrement temporary index incase constructor throws to retain invariants (elements of the buffer are always initialized).
        auto newIndex = m_tailIndex;
        decrement(newIndex);
        m_allocator.construct(&m_data[newIndex], std::forward(val));
        m_tailIndex = newIndex;
    }

    /// @brief Inserts an element in the back of the buffer. If buffer would get full after the operation, allocates more memory.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @param val Value of type T to be appended.
    /// @note Allocates memory before the insertion if the buffer would be full after the operation.
    /// @throw Can throw std::bad_alloc.
    /// @exception If the copy constructor of value_type throws, behaviour is undefined. Otherwise in case of exception this function has no effect (Strong Exception Guarantee).
    /// @pre T needs to satisfy CopyInsertable.
    /// @post If more memory is allocated, all pointers and references are invalidated.
    void push_back(const value_type& val)
    {
        validateCapacity(1);

        m_allocator.construct(&m_data[m_headIndex], val);
        increment(m_headIndex);
    }

    /// @brief Inserts an element in the back of the buffer by move if move constructor is provided for value_type. If buffer would get full after the operation, allocates more memory.
    /// @param val Rvalue reference to the value to be appended.
    /// @note Allocates memory before the insertion if the buffer would be full after the operation.
    /// @throw Can throw std::bad_alloc if more memory is allocated.
    /// @exception If the move/copy constructor of value_type throws, behaviour is undefined. Otherwise in case of exception (std::bad_alloc) this function has no effect (Strong Exception Guarantee).
    /// @pre T needs to satisfy MoveInsertable or CopyInsertable.
    /// @post If more memory is allocated, all pointers and references are invalidated.
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
    /// @note Reduces capacity by allocating a smaller memory area and moving the elements. 
    /// @throw Might throw std::bad_alloc if memory allocation fails.
    /// @exception If T's move (or copy) constructor can and does throw, behaviour is undefined. If any other exception is thrown (bad_alloc) this function has no effect (Strong exception guarantee).
    /// @details Linear complexity in relation to size of the buffer.
    void shrink_to_fit()
    {
        if (m_capacity <= size() + 2)
        { 
            return;
        }

        reserve(size() + 2, true);
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
    /// @return const-Reference to the first element.
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
        // If the index is at the beginning border of the allocated memory area it needs to be wrapped around. 
        if (m_headIndex == 0)
        {
            return m_data[m_capacity - 1];
        }
        return m_data[m_headIndex-1];
    }

    /// @brief Returns a const-reference to the last element in the buffer. Behaviour is undefined for empty buffer.
    /// @return const-reference to the last element in the buffer.
    /// @details Constant complexity.
    const_reference back() const noexcept
    {
        // Since head points to next-to-last element, it needs to be decremented once to get the correct element. 
        // If the index is at the beginning border of the allocated memory area it needs to be wrapped around. 
        if (m_headIndex == 0)
        {
            return m_data[m_capacity - 1];
        }
        return m_data[m_headIndex-1];
    }

    /// @brief Construct iterator at begin.
    /// @return Iterator pointing to first element.
    iterator begin() noexcept
    {
        return iterator(this, 0);
    }

    /// @brief Construct const_iterator at begin.
    /// @return Const_iterator pointing to first element.
    const_iterator begin() const noexcept
    {
        return const_iterator(this, 0);
    }

    /// @brief Construct iterator at end.
    /// @return Iterator pointing past last element.
    iterator end() noexcept
    {
        return iterator(this, size());
    }

    /// @brief Construct const_iterator at end.
    /// @return Const_iterator pointing past last element.
    const_iterator end() const noexcept
    {
        return const_iterator(this, size());
    }

    /// @brief Construct const_iterator at begin.
    /// @return Const_iterator pointing to first element.
    const_iterator cbegin() const noexcept
    {
        return const_iterator(this, 0);
    }

    /// @brief Construct const_iterator.
    /// @return Const_iterator pointing past last element.
    const_iterator cend() const noexcept
    {
        return const_iterator(this, size());
    }

private:

    /// @brief Reserves more memory if needed for an increase in size. If more memory is needed, allocates capacity * 1.5 or if that is not enough (capacity * 1.5 + increase).
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

    template<typename T>
    iterator insertBase(const_iterator pos, const size_type amount, T&& value)
    {
        validateCapacity(amount);

        for (size_type i = 0; i < amount; i++)
        {
            // Construct element at the end.
            m_allocator.construct(&(*end()), std::forward<T>(value));
            increment(m_headIndex);
        }

        iterator it(this, pos.getIndex());

        // Rotate elements from the back into pos.
        std::rotate(it, end() - amount, end());

        return it;
    }

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

    /// @brief Increment an index.
    /// @param index The index to increment.
    void increment(size_t& index) noexcept
    {   
        ++index;
        //Reaching equal is past the last element, then wraps around.
        if(index >= m_capacity)
        {
            index = 0;
        }
    }

    /// @brief Increments an index multiple times.
    /// @param index Index to increment.
    /// @param times Amount of increments.
    void increment(size_t& index, size_t times) noexcept
    {
        while(times > 0)
        {
            increment(index);
            times--;
        }
    }

    /// @brief Decrements an index.
    /// @param index The index to decrement.
    void decrement(size_t& index) noexcept
    {
        if(index == 0)
        {
            index = m_capacity - 1;
        }
        else{
            --index;
        }
    }
    
    /// @brief Decrements an index multiple times.
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

    size_type m_headIndex; /*!< Index of the head. Index past the last element, acts as "back" of the buffer.*/ 
    size_type m_tailIndex; /*!< Index of the tail. Index to the "first" element in the buffer.*/
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

    for(int i = 0; i < lhs.size(); i++)
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