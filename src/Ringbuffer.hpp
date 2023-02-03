#ifndef MAIN_HPP
#define MAIN_HPP

#include <memory>
#include <vector>
#include <algorithm>
#include <deque>
#include "Iterator.hpp"

//Forward decl
template<typename T, typename Allocator = std::allocator<T>>
class RingBuffer;


//template<typename T>
//using RingBufferSharedPtr = shared_ptr<RingBuffer<value_type>>;

/// @brief Dynamic Ringbuffer is a dynamically growing std::container with support for queue, stack and priority queue adaptor functionality. 
/// @tparam T type of the ringbuffer
/// @tparam Allocator optional allocator for underlying vector. Defaults to std::allocator<T>
template<typename T, typename Allocator> 
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

    //TODO::are reverse iterators needed? Probably, hard to compete with stl container algorithms without reverse iterator.Right?
    //using reverse_iterator = reverse_iterator<iterator>;
    //using const_reverse_iterator = const_reverse_iterator<const_iterator>;

    using difference_type = ptrdiff_t;
    using size_type = std::size_t;

public:
    // TODO Implement user-defined constructors and what not. Need to implement destructor that destroys all the elements to take care of dynamic elements.
    //
    /// @brief Default constructor.
    RingBuffer()
    {
        m_headIndex = 0;
        m_tailIndex = 0;
        m_data.reserve(2);
    }

    /// @brief Initializer list contructor.
    RingBuffer(std::initializer_list<T> init):m_data(init), debug_count(init.size()), m_headIndex(init.size()), m_tailIndex(0) 
    {
        m_data.reserve(size() * 1.4);
    }

    /// @brief Custom constructor.
    /// @param size Size of the buffer to initialize.
    /// @param val Value to set every element to.
    RingBuffer(size_type size, T val = 0) : debug_count(size), m_headIndex(size), m_tailIndex(0)
    {
        m_data = std::vector<T>(size,val);
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
        return iterator(this, debug_count);
    }

    /// @brief Construct const_iterator at end.
    /// @return Const_iterator pointing past last element.
    const_iterator end() const noexcept
    {
        return const_iterator(this, debug_count);
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
        return const_iterator(this, debug_count);
    }

    reference operator[](const size_type logicalIndex)
    {
        // if sum of tailIndex (physical first element) and logical index(logical first element) is larger than vector capacity, 
        // wrap index around to start.
        auto index(m_tailIndex + logicalIndex);
        auto capacity(m_data.capacity());

        if(capacity <= index)
        {
            index -= capacity;
        }

        return m_data.data()[index];
    }

    /// @brief Returns an element from the buffer corresponding to a logical index.
    /// @param logicalIndex Index of the element to fetch.
    /// @return Const reference the the object at logical index.
    const_reference operator[](const size_type logicalIndex) const
    {
        auto index(m_tailIndex + logicalIndex);
        auto capacity(m_data.capacity());

        if(capacity <= index)
        {
            index -= capacity;
        }

        return m_data.data()[index];
    }

	/// @brief Sorts ringbuffer so that logical tail matches the first element in physical memory.
    /// @return Pointer to the first element.
    //TODO need to do spinnerino. Now returns vector physical first element.
    pointer data(){
        return m_data.data();
    }

    /// @brief Gets the size of the container.
    /// @return Size of buffer.
    size_type size() const
    {
        if(m_headIndex < m_tailIndex)
        {
            return m_headIndex + m_data.capacity() - m_tailIndex;
        }

        return m_headIndex - m_tailIndex;
    }

    size_type capacity() const
    {
        return m_data.capacity();
    }

    size_type max_size() const noexcept
    {
        return m_data.max_size();
    }

    /// @brief Check if buffer is empty
    /// @return True if buffer is empty
    bool empty() const noexcept
    {
        return m_tailIndex == m_headIndex;
    }

    /// @brief Resizes the container so that it contains n elements.
    /// @param n Size to resize to.
    void resize(size_type n)
    {
        m_data.resize(n);
    }

    /// @brief Resize overload.
    /// @param n Size to resize to.
    /// @param val Value to initialize new elements to, if N is larger than current container capacity. Checked implicitly in the vector implementation.
    void resize(size_type n, const T& val)
    {
        m_data.resize(n,val);
    }

    void reserve(size_type newsize)
    {
        m_data.reserve(newsize);
    }

    // Insert element to tail. The logical front of the buffer.
    void push_front(value_type val)
    {
        debug_count++;

        // Pushing to tail grows the buffer backwards.
        decrement(m_tailIndex);
        if(m_tailIndex == m_headIndex)
        {
            // TODO Current implementation requires the buffer to be rotated to match physical layout before reserve.
            // Otherwise buffer will end up cut in half if it has wrapped around. Wonder how vector would deal with uncontinuous structure.
            m_data.reserve(floor(m_data.capacity()* 1.3) + 2);
        }
        // Pushing to front proposes a different problem with std::vector. Pushing back can be solved by resizing,
        // but pushing to front should push the element possibly to the physical back of the vector. To make vector recognize this element,
        // The resize should take up the whole capacity. TODO
        T* _ptr = new(&m_data + sizeof(T) * m_tailIndex) T(val);
    }

    /// @brief Inserts an element in the back of the buffer. If buffer is full, allocates more memory.
    /// @param val Value of type T to be inserted in to the buffer.
    // TODO corner cases : first element, buffer full, buffer full and index at border. Poor design atm
    void push_back(value_type val)
    {
        // Empty buffer case.
        if(m_data.capacity() == m_data.size())
        {
            m_data.reserve(m_data.capacity() * 1.5 + 1 );
            T* _ptr = new(&m_data.data()[m_headIndex]) T(val);
            increment(m_headIndex);
        }
        else
        {
            // Adds element, and after checks for if more capacity is needed. This pre-reserves memory for next element.
            // The buffer is full for only a brief moment.
            m_data.resize(size() + 1);
            T* _ptr = new(&m_data.data()[m_headIndex]) T(val);
            increment(m_headIndex);
            if(m_tailIndex == m_headIndex)
            {
                decrement(m_headIndex);
                m_data.reserve(m_data.capacity() * 1.5);
                increment(m_headIndex);
            }
        }
    }

    // Erases an element from logical front of the buffer.
    void pop_front()
    {
        --debug_count;
        (&m_data.data()[m_tailIndex])->~T();
        increment(m_tailIndex);
    }

    // Erase an element from the logical back of the buffer.
    void pop_back()
    {
        --debug_count;
        decrement(m_headIndex);
        (&m_data.data()[m_headIndex-1])->~T();
    }

//===========================================================
//  std::queue adaptor functions
//===========================================================

    reference front()
    {
        return m_data.data()[m_tailIndex];
    }

    const_reference front() const
    {
        return m_data.data()[m_tailIndex];
    }

    ///@brief Access the last element in the buffer.
    ///@return Reference to the last element in the buffer. Undefined behaviour is size of buffer is 0.
    reference back()
    {
        // Since head points to next-to-last element, it needs to be decremented once to get the correct element. 
        // If the index is at the beginning border of the allocated memory area it needs to be wrapped around. 
        if (m_headIndex == 0)
        {
            return m_data.data()[m_data.capacity() - 1];
        }
        return m_data.data()[m_headIndex-1];
    }

    const_reference back() const
    {
        // Since head points to next-to-last element, it needs to be decremented once to get the correct element. 
        // If the index is at the beginning border of the allocated memory area it needs to be wrapped around. 
        if (m_headIndex == 0)
        {
            return m_data.data()[m_data.capacity() - 1];
        }
        return m_data.data()[m_headIndex-1];
    }

private:
    void increment(size_t& index)
    {   
        ++index;
        // Reaching equal is past the last element, then wraps around.
        if(index >= m_data.capacity())
        {
            index = 0;
        }
    }

    void decrement(size_t& index)
    {

        if(index == 0)
        {
            index = m_data.capacity() - 1;
        }
        else{
            --index;
        }
    }


//==========================================
// Members 
//==========================================
    
private:
    // Count tracks the amount of elements in the buffer.
    size_t debug_count = 0;
    size_t m_headIndex = 0; /* Index of the last element in the buffer.*/ 
    size_t m_tailIndex = 0;/* Index of the first element in the buffer.*/

    std::vector <T,Allocator> m_data;/* Underlying vector to store the data*/
};

// Modify equality operators to compare also logical order. Part of THE.

/// @brief Equality comparator (compare- operator = comparator)
/// @tparam T Value type
/// @tparam Alloc Optional custom allocator. Defaults to std::allocator<T>.
/// @param lhs Left hand side operand
/// @param rhs right hand side operand
/// @return returns true if the compared buffers have equal underlying data containers.
template<typename T , typename Alloc>
inline bool operator==(const RingBuffer<T,Alloc>& lhs, const RingBuffer<T,Alloc>& rhs)
{
    return (lhs.size() == rhs.size());
}

/// @brief Not-equal comparator
/// @tparam T Value type
/// @tparam Alloc Optional custom allocator. Defaults to std::allocator<T>.
/// @param lhs Left hand side operand.
/// @param rhs Right hand side operand.
/// @return returns true if the compared buffers dont have equal underlying data containers.
template<typename T,typename Alloc>
inline bool operator!=(const RingBuffer<T,Alloc>& lhs, const RingBuffer<T,Alloc>& rhs)
{
    return !(lhs == rhs);
}

#endif /*MAIN_HPP*/