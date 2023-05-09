#ifndef MAIN_HPP
#define MAIN_HPP

#include <memory>
#include <algorithm>
#include <limits>
#include "Iterator.hpp"

/// @brief Dynamic Ringbuffer is a dynamically growing std::container with support for queue, stack and priority queue adaptor functionality. 
/// @tparam T Type of the elements. Must meet the requirement EmplaceCostructible.
/// @tparam Allocator Allocator used for (de)allocation and (de)construction. Defaults to std::allocator<T>
template<typename T, typename Allocator = std::allocator<T>> 
class RingBuffer
{

public:
    using value_type = T;
    using allocator_type = Allocator;
    using reference = T&;
    using const_reference= const T&;
    using pointer = typename T*;
    using const_pointer= typename const T*;
    using const_iterator = _rBuf_const_iterator<RingBuffer<T>>;
    using iterator = _rBuf_iterator<RingBuffer<T>>;

    using difference_type = ptrdiff_t;
    using size_type = std::size_t;

    /// @brief Default constructor.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @note Allocates memory for 2 elements. The buffer works on a principle that it never gets full.
    RingBuffer() : m_headIndex(0), m_tailIndex(0), m_capacity(2)
    {
        m_data = m_allocator.allocate(m_capacity);
    }

    /// @brief Custom constructor. Initializes a buffer to a capacity without constructing any elements.
    /// @param capacity Capacity of the buffer.
    RingBuffer(size_type capacity) : m_headIndex(0), m_tailIndex(0), m_capacity(capacity)
    {
        m_data = m_allocator.allocate(m_capacity);
    }

    /// @brief Custom constructor. Constructs the buffer and initializes all of its elements to a given value.
    /// @param size Amount of elements to be initialized in the buffer.
    /// @param val Value which the elements are initialized to.
    RingBuffer(size_type size, T val) : m_headIndex(0), m_tailIndex(0), m_capacity(size + 2)
    {
        m_data = m_allocator.allocate(m_capacity);
        for(int i = 0 ; i < size ;  i++)
        {
            push_back(val);
        }
    }

    /// @brief Custom constructor. Construct the buffer from range [begin,end).
    /// @param begin Iterator to first element of range.
    /// @param end Iterator past-the-last element of range.
    /// @note Behavior is undefined if elements in range are not initialized.
    RingBuffer(const_iterator beginIt, const_iterator endIt)
    {
        const auto size = endIt - beginIt;
        m_capacity = size + 2;
        m_data = m_allocator.allocate(m_capacity);
        m_headIndex = size;
        m_tailIndex = 0;

        copy(beginIt, endIt, begin());
    }

    /// @brief Initializer list contructor.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @note Allocates memory for 2 extra elements.
    explicit RingBuffer(std::initializer_list<T> init): m_headIndex(0), m_tailIndex(0), m_capacity(init.size() + 2)
    {
        m_data = m_allocator.allocate(m_capacity);
        for(const auto& element : init)
        {
            push_back(element);
        }
    }

    /// @brief Copy constructor.
    /// @throw Might throw std::bad_alloc if there is not enough memory for memory allocation.
    /// @param rhs Reference to a RingBuffer to create a copy from.
    RingBuffer(const RingBuffer& rhs) : m_capacity(rhs.m_capacity), m_headIndex(rhs.m_headIndex), m_tailIndex(rhs.m_tailIndex)
    {
        m_data = m_allocator.allocate(m_capacity);
        
        // Copies the buffer by calling copyconstructor on each element.
        copy(rhs.cbegin(), rhs.cend(), begin());
    }

    /// @brief Move constructor.
    /// @param other Rvalue reference to other buffer.
    RingBuffer(RingBuffer&& other) noexcept
    {
        m_data = std::exchange(other.m_data, nullptr);
        m_capacity = std::exchange(other.m_capacity, 0);
        m_headIndex = std::exchange(other.m_headIndex, 0);
        m_tailIndex = std::exchange(other.m_tailIndex, 0);
    }

    ~RingBuffer()
    {
        // Calls destructor for each element in the buffer.
        if(size()) for_each(begin(),end(),[](T elem) { elem.~T(); });

        // After destruction deallocate the memory.
        m_allocator.deallocate(m_data, m_capacity);
    }

    /// @brief Inserts an element to the buffer.
    /// @param pos Iterator where the the element should be inserted. 
    /// @param value Value to insert.
    /// @return Iterator that points to the inserted element.
    /// @exception Can throw std::bad_alloc, or 
    iterator insert(const_iterator pos, const value_type& value)
    {
        while(m_capacity - 1 == size())
        {
            reserve(m_capacity * 1.5);
        }

        shift(pos, 1);
        m_allocator.construct(&(*pos), value); 

        return iterator(this, pos.getIndex());
    }

    /// @brief Inserts an element to the buffer.
    /// @param pos Iterator where the the element should be inserted
    /// @param value Value to insert.
    /// @return Iterator that pos to the inserted element.
    /// @exception Can throw std::bad_alloc, or something from element construction. 
    iterator insert(const_iterator pos, value_type&& value)
    {
        while(m_capacity - 1 == size())
        {
            reserve(m_capacity * 1.5);
        }

        shift(pos, 1);

        m_allocator.construct(&(*pos), std::forward<const value_type>(value));
        return iterator(this, pos.getIndex());
    }

    /// @brief Inserts an element to the buffer.
    /// @param pos Iterator where the the element should be inserted
    /// @param value Value to insert. T must meet the requirements of CopyInsertable.
    /// @return Iterator that pos to the inserted element.
    /// @exception Can throw std::bad_alloc, or something from element construction. Allocation failure is critical, construction not as much. TODO improve exception safety.
    iterator insert(iterator pos, const size_type amount, const value_type& value)
    {
        while(m_capacity - 1 <= size() + amount)
        {
            reserve(m_capacity * 1.5);
        }

        shift(pos, amount);
        for(int i = 0; i < amount; i++)
        {
            m_allocator.construct(&pos[i], value);
        }

        return pos;
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
        const auto amount = sourceEnd - sourceBegin;
        while(m_capacity - 1 <= size() + amount)
        {
            reserve(m_capacity * 1.5);
        }

        shift(pos, amount);
        for(int i = 0; i < amount; i++)
        {
            m_allocator.construct(&pos[i], *(sourceBegin + i));
        }

        auto destIt = iterator(this, pos.getIndex());
        copy(sourceBegin, sourceEnd, destIt);

        return destIt;
    }

    /// @brief Inserts initializer list to buffer.
    /// @param pos Iterator where the list will be inserted.
    /// @param list Initiliazer list to insert.
    /// @pre pos must be a valid dereferenceable const_iterator within the container. Otherwise behavior is undefined.
    /// @return Returns Iterator to the first element inserted, or the element pointed by pos if the initializer list was empty.
    iterator insert(const_iterator pos, std::initializer_list<T> list)
    {
        const auto amount = list.size();
        while(m_capacity - 1 <= size() + amount)
        {
            reserve(m_capacity * 1.5);
        }

        auto destIt = iterator(this, pos.getIndex());

        for(auto i= 0; i < amount; i++)
        {
            m_allocator.construct(&destIt[i], *(list.begin() + i));
        }

        return destIt;
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
        for(auto i = 0; i + posIndex + 1 < endIndex; i++)
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
        for(auto i = 0; i <= rangeSize; i++)
        {
            it = erase(first);
        }

        return it;
    }

    /// @brief Destroys all elements in a buffer.
    /// @note All references, pointers and iterators are invalidated. Leaves capacity unchanged.
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
    /// @pre T is EmplaceConstructible and [sourceBegin, sourceEnd) are not in the buffer.
    /// @post All existing pointers, references and iterators are to be considered invalid.
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
    /// @pre T is EmplaceConstructible.
    /// @note Leaves capacity of the buffer unchanged.
    void assign(std::initializer_list<T> list)
    {
        clear();
        for(auto i = 0; i < list.size(); i++)
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
    /// @param copy A temporary RingBuffer created by a copy constructor.
    /// @return Returns reference to the left hand side RungBuffer after swap.
    /// @note Strong exception guarantee. 
    RingBuffer& operator=(const RingBuffer& other)
    {
        RingBuffer copy(other);
        copy.swap(*this);
        return *this;
    }

    /// @brief Move assignment operator.
    /// @param other Rvalue ref to other buffer.
    /// @return Reference to the buffer.
    RingBuffer& operator=(RingBuffer&& other) noexcept
    {
        RingBuffer copy(std::move(other));
        copy.swap(*this);
        return *this;
    }

    /// @brief Index operator. The operator acts as interface that hides the physical layout from the user.
    /// @param logicalIndex Index of the element.
    /// @return Returns a reference to the element.
    /// @note Does not check bounds, and behaviour for accessing index larger than size() - 1 is undefined.
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
    /// @note Does not check bounds, and behaviour for accessing index larger than size() - 1 is undefined.
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
    /// @param other Reference to a RingBuffer to swap with.
    void swap(RingBuffer& other) noexcept
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
    friend void swap(RingBuffer& a, RingBuffer& b) noexcept
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
        auto temp = RingBuffer<T>(capacity());
        copy(cbegin(), cend(), temp.begin());
        // Set the head index. Whole point of this copy is to slide the buffer to the start, so tail is correctly at 0 after constructing the temporary buffer.
        temp.m_headIndex = size();

        // Pummel the original buffer with destructor calls.
        if(size())
        {
            for_each(begin(),end(),[](T elem) { elem.~T(); });
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
        const auto maxSize = std::numeric_limits<std::size_t>::max();
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
        auto temp =  RingBuffer<T>(newCapacity);
        temp.m_headIndex = m_headIndex;
        temp.m_tailIndex = m_tailIndex;

        // If buffer is wrapped, move the tail. 
        if(m_headIndex < m_tailIndex)
        {
            // Moves the tail so that the distance from the end border of allocated memory stays the same.
            temp.m_tailIndex += newCapacity - m_capacity;
        }

        // Copy to temp memory.
        copy(cbegin(), cend(), temp.begin());

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
    /// @pre T is EmplaceConstructible.
    /// @post If more memory is allocated due to the buffer getting full, all pointers and references are invalidated.
    void push_back(value_type val)
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
    
        m_allocator.construct(&m_data[m_headIndex], std::forward(val));
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

    /// @brief Increment an index.
    /// @param index The index to increment.
    void increment(size_t& index) noexcept
    {   
        ++index;
        // Reaching equal is past the last element, then wraps around.
        // if(index >= m_capacity)
        // {
        //     index = 0;
        // }
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
    /// @note Copies all elements in range [sourceBegin, sourceEnd), from sourceBegin to  sourceEnd - 1. The behaviour is undefined destBegin overlaps the range [sourceBegin, sourceEnd).
    /// @exception If any exception is thrown, this function has no effect. Strong exception guarantee.
    void copy(const_iterator sourceBegin, const_iterator sourceEnd, iterator destBegin)
    {
        size_t size = sourceEnd - sourceBegin;

        for(ptrdiff_t i = 0; i != size ; i++)
        {
            m_allocator.construct(&destBegin[i], sourceBegin[i]);
        }
    }

    /// @brief Shifts a range of elements forward or backwards with the copy and swap idiom. Deduces shifts direction based on distance to each border from the given iterator.
    /// @param shiftPoint Iterator to the shift point. The shift point is an element, which is the first index that will be empty after shift. "Where to spawn empty elements".
    /// @param offset Size of shift. How many steps each element will be shifted, eg, "How many empty elements".
    /// @exception Might throw exceptions from memory allocation and element constructors. If exceptions are thrown, nothing happens (Strong exception safety guarantee).
    /// @note Undefined behaviour if buffer does not have enough memory allocated for the shift.
    void shift(const_iterator shiftPoint, difference_type offset)
    {
        const auto endIt = end();
        const auto beginIt = begin();

        RingBuffer<T> temp(m_capacity);
        temp.m_tailIndex = m_tailIndex;
        temp.m_headIndex = m_headIndex;

        // Distance to the shift-point iterator from each end.
        const auto fromEnd = abs(shiftPoint - endIt);
        const auto fromBegin = abs(shiftPoint - beginIt);

        // Shift the elements in the direction based on distance from borders.
        if(fromBegin >= fromEnd)
        {
            increment(temp.m_headIndex, offset);
            // Iterator to first element after the "cut off" caused by shifting.
            auto destCutOff = RingBuffer<T>::iterator(&temp, shiftPoint.getIndex() + offset);
            copy(shiftPoint, endIt, destCutOff);

            copy(beginIt, shiftPoint, temp.begin());
        }
        else
        {
            // Iterator to first element after the "cut off" caused by shifting.
            auto destCutOff = RingBuffer<T>::iterator(&temp, shiftPoint.getIndex());
            copy(beginIt, shiftPoint, temp.begin() - offset);

            copy(shiftPoint, endIt, destCutOff);
            // Decrementing the tail pointer changes what element the sourceBegin pointer deferences to so it has to be done last.
            decrement(temp.m_tailIndex, offset);

        }

        this->swap(temp);
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
inline bool operator==(const RingBuffer<T,Alloc>& lhs, const RingBuffer<T,Alloc>& rhs)
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
inline bool operator!=(const RingBuffer<T,Alloc>& lhs, const RingBuffer<T,Alloc>& rhs)
{
    return !(lhs == rhs);
}

#endif /*MAIN_HPP*/