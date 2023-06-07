#ifndef DYNAMIC_RINGBUFFER_HPP
#define DYNAMIC_RINGBUFFER_HPP

#include <memory>
#include <algorithm>
#include <limits>
#include "Iterator.hpp"

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
    using const_iterator = _rBuf_const_iterator<ring_buffer<T>>;
    using iterator = _rBuf_iterator<ring_buffer<T>>;

    using difference_type = ptrdiff_t;
    using size_type = std::size_t;

    /// @brief Default constructor.
    /// @throw Might throw std::bad_alloc if there is not enough memory available for allocation.
    /// @exception If an exception is thrown, this function has no effect. Strong exception guarantee.
    /// @note Allocates memory for 2 elements but buffer is initialized to 0 size.
    /// @details Constant complexity.
    ring_buffer() : ring_buffer(0)
    {
    }

    /// @brief Constructs the container with a custom allocator.
    /// @param alloc Custom allocator for the buffer.
    /// @throw Might throw std::bad_alloc if there is not enough memory available for allocation.
    /// @exception If an exception is thrown, this function has no effect. Strong exception guarantee.
    /// @note Allocates memory for 2 elements but buffer is initialized to 0 size.
    /// @details Constant complexity.
    explicit ring_buffer(const allocator_type& alloc) : ring_buffer(0, alloc)
    {
    }

    /// @brief Constructs the buffer to a size with given values and optionally a custom allocator.
    /// @param size Amount of elements to be initialized in the buffer.
    /// @param val Reference to a value which the elements are initialized to.
    /// @param alloc Custom allocator.
    /// @throw Might throw std::bad_alloc if there is not enough memory available for allocation.
    ring_buffer(size_type count, const_reference val, const allocator_type& alloc = allocator_type()) : m_headIndex(0), m_tailIndex(0), m_capacity(count + 2), m_allocator(alloc)
    {
        m_data = m_allocator.allocate(m_capacity);
        for (size_t i = 0; i < count; i++)
        {
            push_back(val);
        }
    }

    /// @brief Custom constructor. Initializes a buffer to a capacity without constructing any elements.
    /// @param capacity Capacity of the buffer.
    /// @throw Might throw std::bad_alloc if there is not enough memory available for allocation, or some exception from T's constructor.
    /// @exception If an exception is thrown, this function has no effect. Strong exception guarantee.
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
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @note Allocates memory for 2 extra elements.
    explicit ring_buffer(std::initializer_list<T> init): ring_buffer(init.begin(),init.end())
    {
    }

    /// @brief Copy constructor.
    /// @throw Might throw std::bad_alloc if there is not enough memory for memory allocation.
    /// @param rhs Reference to a RingBuffer to create a copy from.
    ring_buffer(const ring_buffer& rhs) : m_capacity(rhs.m_capacity), m_headIndex(rhs.m_headIndex), m_tailIndex(rhs.m_tailIndex)
    {
        m_data = m_allocator.allocate(m_capacity);
        
        // Copies the buffer by calling copyconstructor on each element.
        copy(rhs.cbegin(), rhs.cend(), begin());
    }

    /// @brief Move constructor.
    /// @param other Rvalue reference to other buffer.
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
        while(m_capacity - 1 <= size() + 1)
        {
            reserve(m_capacity * 1.5);
        }

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
        while(m_capacity - 1 <= size() + 1)
        {
            reserve(m_capacity * 1.5);
        }

        return insertBase(pos, 1, std::move(value));
    }

    /// @brief Inserts an element to the buffer.
    /// @param pos Iterator where the the element should be inserted
    /// @param value Value to insert. T must meet the requirements of CopyInsertable.
    /// @return Iterator that pos to the inserted element.
    /// @throw Can throw std::bad_alloc, or something from element construction.
    iterator insert(const_iterator pos, const size_type amount, const value_type& value)
    {
        while(m_capacity - 1 <= size() + amount)
        {
            reserve(m_capacity * 1.5);
        }

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
        auto rangeSize = last - first -1;
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
    void clear()
    {
        for(m_tailIndex; m_tailIndex < m_headIndex ; m_tailIndex++)
        {
            m_allocator.destroy(&m_data[m_tailIndex]);
        }

        m_headIndex = 0;
        m_tailIndex = 0;
    }

    /// @brief Replaces the elements in the buffer with copy of [sourceBegin, sourceEnd)
    /// @pre T is CopyConstructible and [sourceBegin, sourceEnd) are not in the buffer.
    /// @post All existing references, pointers and iterators are to be considered invalid.
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
    /// @post All existing references, pointers and iterators are to be considered invalid.
    /// @note Leaves capacity of the buffer unchanged.
    void assign(std::initializer_list<T> list)
    {
        clear();
        for(size_t i = 0; i < list.size(); i++)
        {
            push_back(*(list.begin() + i));
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
    /// @note Strong exception guarantee. 
    ring_buffer& operator=(const ring_buffer& other)
    {
        ring_buffer copy(other);
        copy.swap(*this);
        return *this;
    }

    /// @brief Move assignment operator.
    /// @param other Rvalue ref to other buffer.
    /// @return Reference to the buffer.
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
    /// @post All existing iterators and pointers have undefined behaviour after the operation. 
    /// @note Internally calls assign(), which destroys all elements before Copy Inserting from initializer list.
    ring_buffer& operator=(std::initializer_list<T> init)
    {
        assign(init);
        return *this;
    }

    /// @brief Index operator. The operator acts as interface that hides the physical layout from the user.
    /// @param logicalIndex Index of the element.
    /// @return Returns a reference to the element.
    /// @note Behaviour for accessing index larger than size() - 1 is undefined.
    reference operator[](const size_type logicalIndex)
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

    /// @brief Index operator. The operator acts as interface that hides the physical layout from the user.
    /// @param logicalIndex Index of the element.
    /// @return Returns a const reference the the element.
    /// @note Behaviour for accessing index larger than size() - 1 is undefined.
    const_reference operator[](const size_type logicalIndex) const
    {
        auto index(m_tailIndex + logicalIndex);

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
    friend void swap(ring_buffer& a, ring_buffer& b) noexcept
    {
        a.swap(b);
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

	/// @brief Sorts ringbuffer so that logical tail matches the first element in physical memory.
    /// @return Returns a pointer to the first element.
    /// @note Invalidates all existing pointers to buffer elements. Iterators are not invalidated. Expensive function because of allocation and multiple copies. No exception guarantee.
    /// @throw Can throw exceptions from memory allocation and constructors when copying elements.
    pointer data()
    {
        if(!size())
        {
            return m_data;
        }

        // This function rotates the buffer by doing a double copy: copies the buffer to temporary location and then back to original but matching the first
        // element to the beginning of the allocated area. Inefficient but linear in complexity related to the length of the buffer, and consecutive calls to data() does not invalidate previous pointer.

        // Create a temporary buffer and copy existing buffers elements to the start of the temporary memory.
        auto temp = ring_buffer<T>(capacity());
        copy(cbegin(), cend(), temp.begin());
        // Set the head index. Whole point of this copy is to slide the buffer to the start, so tail is correctly at 0 after constructing the temporary buffer.
        temp.m_headIndex = size();

        // Pummel the original buffer with destructor calls.
        if(size())
        {
            for_each(begin(),end(),[this](T& elem) { m_allocator.destroy(&elem); });
        }

        // Set up index values on original buffer to match rotated ones.
        m_headIndex = temp.m_headIndex;
        m_tailIndex = 0;
        copy(temp.cbegin(), temp.cend(), begin());
        return m_data;
    }

    /// @brief Gets the size of the container.
    /// @return Size of buffer.
    size_type size() const noexcept
    {
        if(m_headIndex < m_tailIndex)
        {
            return m_headIndex + m_capacity - m_tailIndex;
        }

        return m_headIndex - m_tailIndex;
    }

    size_type max_size() const noexcept
    {
        constexpr auto maxSize = std::numeric_limits<std::size_t>::max();
        return maxSize / sizeof(T);
    }

    /// @brief Capacity getter.
    /// @return m_capacity Returns how many elements have been allocated for the buffers use. 
    size_type capacity() const noexcept
    {
        return m_capacity;
    }

    /// @brief Check if buffer is empty
    /// @return True if buffer is empty
    bool empty() const noexcept
    {
        return m_tailIndex == m_headIndex;
    }

    /// @brief Allocates more memory and copies the existing buffer to the new memory location.
    /// @throw Throws std::bad_alloc if there is not enough memory for allocation. Throws std::bad_array_new_lenght if std::numeric_limits<std::size_t>::max() / sizeof(T) < newsize.
    /// @param newCapacity Amount of memory to allocate. if newCapacity is less than or equal to m_capacity, function does nothing.
    /// @exception If any exception is thrown, this function has no effect. Strong exception guarantee.
    void reserve(size_type newCapacity)
    {
        if(newCapacity <= m_capacity) return;

        // Temporary buffer to take hits if exceptions occur.
        auto temp =  ring_buffer<T>(newCapacity);
        temp.m_headIndex = m_headIndex;
        temp.m_tailIndex = m_tailIndex;

        // If buffer is wrapped, move the tail. 
        if(m_headIndex < m_tailIndex)
        {
            // Moves the tail so that the distance from the end border of allocated memory stays the same.
            temp.m_tailIndex += newCapacity - m_capacity;
        }

        // Copy to temp memory.
        temp.assign(cbegin(), cend());

        // Assings the data from temp to original buffer. The resources from temp will be released when function goes out of scope.
        this->swap(temp);
    }

    /// @brief Inserts an element to the front of the buffer.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @param val Element to insert.  Needs to be CopyConstructible.
    /// @note Allocates memory before the insertion if the buffer would be full after the operation.
    /// @exception If any exception is thrown, this function has no effect. Strong exception guarantee.
    void push_front(value_type val)
    {
        if(m_capacity - 1 == size())
        {
            reserve(m_capacity* 1.5);
        }

        // Decrement temporary index incase constructol throws to retain invariants (elements of the buffer are always initialized).
        auto newIndex = m_tailIndex;
        decrement(newIndex);
        m_allocator.construct(&m_data[newIndex], val);
        m_tailIndex = newIndex;
    }

    /// @brief Inserts an element to the front of the buffer.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @param val Rvalue reference to the element to insert.
    /// @note Allocates memory before the insertion if the buffer would be full after the operation.
    /// @pre T needs to satisfy MoveConstoructible
    /// @exception If any exception is thrown, this function has no effect. Strong exception guarantee.
    void push_front(value_type&& val)
    {
        if(m_capacity - 1 == size())
        {
            reserve(m_capacity* 1.5);
        }

        // Decrement temporary index incase constructor throws to retain invariants (elements of the buffer are always initialized).
        auto newIndex = m_tailIndex;
        decrement(newIndex);
        m_allocator.construct(&m_data[newIndex], std::forward(val));
        m_tailIndex = newIndex;
    }

    /// @brief Inserts an element in the back of the buffer. If buffer is full, allocates more memory.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @param val Value of type T to be appended.
    /// @note Allocates memory before the insertion if the buffer would be full after the operation.
    /// @exception If any exception is thrown, this function retains invariants. Basic exception guarantee.
    /// @pre T is CopyInsertable.
    /// @post If more memory is allocated due to the buffer getting full, all pointers and references are invalidated.
    void push_back(const value_type& val)
    {

        if(m_capacity - 1 == size())
        {
            reserve(m_capacity * 1.5);
        }

        m_allocator.construct(&m_data[m_headIndex], val);
        increment(m_headIndex);
    }

    /// @brief Inserts an element in the back of the buffer. If buffer is full, allocates more memory.
    /// @param val Rvalue reference to the value to be appended.
    /// @note Allocates memory before the insertion if the buffer would be full after the operation.
    /// @exception If any exception is thrown, this function retains invariants. Basic exception guarantee.
    /// @pre T is EmplaceConstructible.
    /// @post If more memory is allocated due to the buffer getting full, all pointers and references are invalidated.
    void push_back(value_type&& val)
    {
        if(m_capacity - 1 == size())
        {
            reserve(m_capacity * 1.5);
        }
    
        m_allocator.construct(&m_data[m_headIndex], std::forward<value_type>(val));
        increment(m_headIndex);
    }

    /// @brief Remove the first element in the buffer.
    void pop_front() noexcept
    {
        m_allocator.destroy(&m_data[m_tailIndex]);
        increment(m_tailIndex);
    }

    // Erase an element from the logical back of the buffer.
    void pop_back() noexcept
    {

        decrement(m_headIndex);
        m_allocator.destroy(&m_data[m_headIndex]);

    }

//===========================================================
//  std::queue adaptor functions
//===========================================================

    /// @brief Returns a reference to the first element in the buffer. Behaviour is undefined for empty buffer.
    /// @return Reference to the first element.
    reference front() noexcept
    {
        return m_data[m_tailIndex];
    }

    /// @brief Returns a reference to the first element in the buffer. Behaviour is undefined for empty buffer.
    /// @return const-Reference to the first element.
    const_reference front() const noexcept
    {
        return m_data[m_tailIndex];
    }

    ///@brief Returns a reference to the last element in the buffer. Behaviour is undefined for empty buffer.
    ///@return Reference to the last element in the buffer.
    reference back() noexcept
    {
        // Since head points to next-to-last element, it needs to be decremented once to get the correct element. 
        // If the index is at the beginning border of the allocated memory area it needs to be wrapped around. 
        if (m_headIndex == 0)
        {
            return m_data[size() - 1];
        }
        return m_data[m_headIndex-1];
    }

    /// @brief Returns a const-reference to the last element in the buffer. Behaviour is undefined for empty buffer.
    /// @return const-reference to the last element in the buffer.
    const_reference back() const noexcept
    {
        // Since head points to next-to-last element, it needs to be decremented once to get the correct element. 
        // If the index is at the beginning border of the allocated memory area it needs to be wrapped around. 
        if (m_headIndex == 0)
        {
            return *m_data[size() - 1];
        }
        return *m_data[m_headIndex-1];
    }

// Ugly testing solution, to enable tests for private methods enable "TEST_INTERNALS" from ring_buffer_tests and comment out the private identifier.
private:

    template<typename T>
    iterator insertBase(const_iterator pos, const size_type amount, T&& value)
    {
        // check amount and reserve for that.
        while (m_capacity - 1 <= size() + 1)
        {
            reserve(m_capacity * 1.5);
        }

        for (size_type i = 0; i < amount; i++)
        {
            // Construct element at the end.
            m_allocator.construct(&(*end()), std::forward<T>(value));
            increment(m_headIndex);
        }

        iterator cUnqualifiedIt(this, pos.getIndex());
        // Rotate elements from the back into pos.
        std::rotate(cUnqualifiedIt, end() - amount, end());

        return cUnqualifiedIt;
    }

    template<typename InputIt>
    iterator insertRangeBase(const_iterator pos, InputIt rangeBegin, InputIt rangeEnd)
    {
        const auto amount = std::distance<InputIt>(rangeBegin, rangeEnd);
        // check amount and reserve for that.
        reserve(m_capacity * 1.5);
        
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

    /// @brief Copies elements by calling allocators construct().
    /// @param sourceBegin Iterator to begin of source data.
    /// @param sourceEnd Iterator to past-the-end element of of source data.
    /// @param destBegin Iterator to beginning of destination range.
    /// @note Copies all elements in range [sourceBegin, sourceEnd), from sourceBegin to  sourceEnd - 1. The behaviour is undefined if destBegin overlaps the range [sourceBegin, sourceEnd). Custom copy is required to copy elements into uninitialized memory.
    /// @pre T is CopyConstructible.
    /// @exception If any exception is thrown, this function has no effect. Strong exception guarantee.
    void copy(const_iterator sourceBegin, const_iterator sourceEnd, iterator destBegin)
    {
        size_t size = sourceEnd - sourceBegin;

        for(size_t i = 0; i != size ; i++)
        {
            m_allocator.construct(&destBegin[i], sourceBegin[i]);
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