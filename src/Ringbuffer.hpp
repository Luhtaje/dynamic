#ifndef MAIN_HPP
#define MAIN_HPP

#include <memory>
#include <algorithm>
#include "Iterator.hpp"

/// @brief Dynamic Ringbuffer is a dynamically growing std::container with support for queue, stack and priority queue adaptor functionality. 
/// @tparam T type of the ringbuffer
/// @tparam Allocator Defaults to std::allocator<T>
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
    /// @param other 
    RingBuffer(RingBuffer&& other) noexcept
    {
        m_data = std::exchange(other.m_data, nullptr);
        m_capacity = std::exchange(other.m_capacity, 0);
        m_headIndex = std::exchange(other.m_headIndex, 0);
        m_tailIndex = std::exchange(other.m_tailIndex, 0);
    }

    //Destructor
    ~RingBuffer()
    {
        // Calls destructor for each element in the buffer.
        if(size()) for_each(begin(),end(),[](T elem) { elem.~T(); });

        // After destruction deallocate the memory.
        m_allocator.deallocate(m_data, m_capacity);
    }

    /// @brief Copy assignment operator.
    /// @param copy A temporary RingBuffer created by a copy constructor.
    /// @return Returns reference to the left hand side RungBuffer after swap.
    RingBuffer& operator=(const RingBuffer& other)
    {
        RingBuffer copy(other);
        copy.swap(*this);
        return *this;
    }

    // Move assignment
    RingBuffer& operator=(RingBuffer&& other) noexcept
    {
        RingBuffer copy(std::move(other));
        copy.swap(*this);
        return *this;
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

	/// @brief Sorts ringbuffer so that logical tail matches the first element in physical memory.
    /// @return Returns a pointer to the first element.
    /// @note Invalidates all existing pointers to buffer elements. Iterators are not invalidated. Expensive function because of allocation and multiple copies.
    pointer data()
    {
        if(!size())
        {
            return m_data;
        }

        // This function rotates the buffer by doing a double copy: copies the buffer to temporary location and then back to original but matching the first
        // element to the beginning of the allocated area. Inefficient but linear in complexity related to the length of the buffer.

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
    /// @note If any exception is thrown, this function has no effect. Strong exception guarantee.
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
    void push_front(value_type val)
    {
        if(m_capacity - 1 == size())
        {
            reserve(m_capacity* 1.5);
        }
        decrement(m_tailIndex);
        m_allocator.construct(&m_data[m_tailIndex], val);
    }

    /// @brief Inserts an element in the back of the buffer. If buffer is full, allocates more memory.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @param val Value of type T to be inserted in to the buffer. Needs to be CopyConstructible.
    /// @note Allocates memory before the insertion if the buffer would be full after the operation.
    void push_back(value_type val)
    {
        // Empty buffer case.
        if(m_capacity - 1 == size())
        {
            reserve(m_capacity * 1.5);
        }

        m_allocator.construct(&m_data[m_headIndex], val);
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
        m_allocator.destroy(&m_data[m_headIndex-1]);
    }

//===========================================================
//  std::queue adaptor functions
//===========================================================

    /// @brief Returns a reference to the first element in the buffer. Behaviour is undefined for empty buffer.
    /// @return Reference to the first element.
    reference front()
    {
        return m_data[m_tailIndex];
    }

    /// @brief Returns a reference to the first element in the buffer. Behaviour is undefined for empty buffer.
    /// @return const-Reference to the first element.
    const_reference front() const
    {
        return m_data[m_tailIndex];
    }

    ///@brief Returns a reference to the last element in the buffer. Behaviour is undefined for empty buffer.
    ///@return Reference to the last element in the buffer.
    reference back()
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
    const_reference back() const
    {
        // Since head points to next-to-last element, it needs to be decremented once to get the correct element. 
        // If the index is at the beginning border of the allocated memory area it needs to be wrapped around. 
        if (m_headIndex == 0)
        {
            return *m_data[size() - 1];
        }
        return *m_data[m_headIndex-1];
    }

private:

    /// @brief  Increments an index. If the index reaches capacity, set index to 0.
    /// @param index The index to increment.
    void increment(size_t& index) noexcept
    {   
        ++index;
        // Reaching equal is past the last element, then wraps around.
        if(index >= m_capacity)
        {
            index = 0;
        }
    }

    /// @brief Decrements an index. If the index is at 0, set index to m_capacity - 1.
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

    /// @brief Copies elements by calling allocators construct() to ensure deep copy.
    /// @param sourceBegin Iterator to begin of source data.
    /// @param sourceEnd Iterator to past-the-end element of of source data.
    /// @param destBegin Iterator to beginning of destination range.
    /// @note Copies all elements in range [sourceBegin, sourceEnd), from sourceBegin to  sourceEnd - 1. The behaviour is undefined destBegin overlaps the range [sourceBegin, sourceEnd).
    void copy(const_iterator sourceBegin, const_iterator sourceEnd, iterator destBegin)
    {
        size_t size = sourceEnd - sourceBegin;

        for(ptrdiff_t i = 0; i != size ; i++)
        {
            m_allocator.construct(&destBegin[i], sourceBegin[i]);
        }
    }

//==========================================
// Members 
//==========================================

private:
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