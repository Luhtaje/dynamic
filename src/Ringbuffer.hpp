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
    // TODO Implement user-defined constructors and what not. Need to implement destructor that destroys all the elements, which are not
    //
    /// @brief Default constructor.
    RingBuffer() = default;

    /// @brief Initializer list contructor.
    RingBuffer(std::initializer_list<T> init):m_data(init), debug_count(init.size()), m_headIndex(init.size()), m_tailIndex(0) {}

    /// @brief Custom constructor.
    /// @param size Size of the buffer to initialize.
    /// @param val Value to set every element to.
    RingBuffer(size_type size, T val = 0) : debug_count(size), m_headIndex(size), m_tailIndex(0)
    {
        m_data = std::vector<T>(size,val);
    }

    /// @brief Contruct iterator at begin.
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

        if(capacity < index)
        {
            index -= capacity;
        }

        return m_data[index];
    }

    /// @brief Returns an element from the buffer corresponding to a logical index.
    /// @param logicalIndex Index of the element to fetch.
    /// @return Const reference the the object at 
    const_reference operator[](const size_type logicalIndex) const
    {
        auto index(m_tailIndex + logicalIndex);
        auto capacity(m_data.capacity());

        if(capacity <= index)
        {
            index -= capacity;
        }

        return m_data[index];
    }

	/// @brief Sorts ringbuffer so that logical tail matches the first element in physical memory.
    /// @return Pointer to the first element.
    //To be implemented! Requires knowledge of logical ends which are not implemented yet.
    RingBuffer data();

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

    void reserve(size_t newsize)
    {
        m_data.reserve(newsize);
    }

    // Insert element to tail. The logical front of the buffer.
    void push_front(value_type val)
    {
        debug_count++;
        // Special case when buffer is empty. Moves head instead of tail. This makes pushing to front always a little slower.
        if(m_headIndex == m_tailIndex)
        {
            push_back(val);
        }
        else
        {
            // Pushing to tail grows the buffer backwards.
            decrement(m_tailIndex);
            if(m_tailIndex == m_headIndex)
            {
                // TODO Current implementation requires the buffer to be rotated to match physical layout before reserve.
                // Otherwise buffer will end up cut in half if it has wrapped around. Wonder how vector would deal with uncontinuous structure.
                m_data.reserve(floor(m_data.capacity()* 1.3) + 2);
            }
            T* _ptr = new(&m_data + sizeof(T) * m_tailIndex) T(val);
            
        }
    }

    /// @brief Inserts an element in the back of the buffer. If buffer is full, allocates more memory.
    /// @param val Value of type T to be inserted in to the buffer.
    // TODO corner cases : first element, buffer full, buffer full and index at border. Poor design atm
    void push_back(value_type val)
    {
        // Checks for empty buffer.
        if(m_headIndex == m_tailIndex)
        {
            m_data.reserve(m_data.capacity() + 5);
            T* _ptr = new(&m_data.data()[0]) T(val);
            if(!_ptr)
            {
                throw;
            }
            // If buffer was empty, re-increment the index.
            increment(m_headIndex);
        }
        else
        {
            m_data.reserve(m_data.capacity() * 1.5);
            T* _ptr = new(&m_data.data()[m_headIndex]) T(val);
            if(!_ptr)
            {

                throw;
            }
            increment(m_headIndex);
        }
    }

    // Erases an element from logical front of the buffer. Moves tail forward. Leaves the object in the vector. When could it be removed?
    void pop_front()
    {
        debug_count--;
        (&m_data[m_tailIndex])->~T();
        increment(m_tailIndex);
    }

    // Erase at from head.
    void pop_back()
    {
        debug_count--;
        (&m_data[m_tailIndex])->~T();
        decrement(m_headIndex);
    }

//===========================================================
//  std::queue adaptor functions
//===========================================================

    // TODO: the indexes point to over-the-element, need to adjust how they are retrieved. Would have to take wrap-around into consideration.
    reference front()
    {
        return m_data[m_tailIndex];
    }

    const_reference front() const
    {
        return m_data[m_tailIndex];
    }

    ///@brief Access the last element in the buffer.
    ///@return Reference to the last element in the buffer. Undefined behaviour is size of buffer is 0.
    reference back()
    {
        return m_data.data()[(m_headIndex -1)];
    }

    const_reference back() const
    {
        return m_data[m_headIndex-1];
    }

private:
    void increment(size_t& index)
    {   
        index++;
        // Reaching equal is past the last element, wraps around.
        if(index >= m_data.capacity())
        {
            index = 0;
        }
    }

    void decrement(size_t& index)
    {
        index--;
        if(index<0)
        {
            index = m_data.capacity() - 1;
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