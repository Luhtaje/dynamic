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
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @note Allocates memory for 2 elements.
    RingBuffer() : m_headIndex(0), m_tailIndex(0), m_capacity(2)
    {
        m_data = m_allocator.allocate(2);
    }

    /// @brief Initializer list contructor.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @note Allocates memory for 2 extra elements.
    RingBuffer(std::initializer_list<T> init): m_headIndex(init.size()), m_tailIndex(0), m_capacity(init.size() + 2)
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
        
        // This should copy the whole buffer correctly in every way, because the iterator implementation is *smuh*.
        std::copy(rhs.begin(), rhs.end(), begin());
    }

    /// @brief Custom constructor.
    /// @param size Size of the buffer to initialize.
    /// @param val Value to set every element to.
    RingBuffer(size_type size, T val = 0) : m_headIndex(size), m_tailIndex(0)
    {
        m_capacity = size + 2;
        m_data = m_allocator.allocate(m_capacity);
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

    /// @brief Move constructor.
    /// @param other 
    RingBuffer(RingBuffer&& other) noexcept
    {
        m_data = std::exchange(other.m_data, nullptr);
        m_capacity = std::exchange(other.m_capacity, 0);
        m_headIndex = std::exchange(other.m_headIndex, 0);
        m_tailIndex = std::exchange(other.m_tailIndex, 0);
    }

    // Move assignment
    RingBuffer& operator=(RingBuffer& other)
    {
        RingBuffer copy(std::move(other));
        copy.swap(*this);
        return *this;
    }

    //Destructor
    ~RingBuffer()
    {
        // Destroy initialized elements before deallocating.

        m_allocator.deallocate(m_data, m_capacity);
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

    /// @brief Friend swap implementation
    friend void swap(RingBuffer& a, RingBuffer& b) noexcept
    {
        a.swap(b);
    }

    /// @brief Construct iterator at begin.
    /// @return Iterator pointing to first element.
    iterator begin() noexcept
    {
        return iterator(this, m_tailIndex);
    }

    /// @brief Construct const_iterator at begin.
    /// @return Const_iterator pointing to first element.
    const_iterator begin() const noexcept
    {
        return const_iterator(this, m_tailIndex);
    }

    /// @brief Construct iterator at end.
    /// @return Iterator pointing past last element.
    iterator end() noexcept
    {
        return iterator(this, m_headIndex);
    }

    /// @brief Construct const_iterator at end.
    /// @return Const_iterator pointing past last element.
    const_iterator end() const noexcept
    {
        return const_iterator(this, m_headIndex);
    }

    /// @brief Construct const_iterator at begin.
    /// @return Const_iterator pointing to first element.
    const_iterator cbegin() const noexcept
    {
        return const_iterator(this, m_tailIndex);
    }

    /// @brief Construct const_iterator.
    /// @return Const_iterator pointing past last element.
    const_iterator cend() const noexcept
    {
        return const_iterator(this, m_headIndex);
    }

    reference operator[](const size_type logicalIndex)
    {
        // if sum of tailIndex (physical first element) and logical index(logical first element) is larger than vector capacity, 
        // wrap index around to start.
        auto index(m_tailIndex + logicalIndex);

        if(m_capacity <= index)
        {
            index -= m_capacity;
        }

        return m_data[index];
    }

    /// @brief Returns an element from the buffer corresponding to a logical index.
    /// @param logicalIndex Index of the element to fetch.
    /// @return Const reference the the object at logical index.
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
    /// @return Pointer to the first element.
    //TODO need to do spinnerino. Now returns vector physical first element.
    pointer data(){
        return m_data;
    }

    /// @brief Gets the size of the container.
    /// @return Size of buffer.
    size_type size() const
    {
        if(m_headIndex < m_tailIndex)
        {
            return m_headIndex + m_capacity - m_tailIndex;
        }

        return m_headIndex - m_tailIndex;
    }

    /// @brief Capacity getter.
    /// @return m_capacity Returns how many elements have been allocated for the buffers use. 
    size_type getCapacity() const
    {
        return m_capacity;
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
    }

    /// @brief Resize overload.
    /// @param n Size to resize to.
    /// @param val Value to initialize new elements to, if N is larger than current container capacity. Checked implicitly in the vector implementation.
    void resize(size_type n, const T& val)
    {
    }

    void reserve(size_type newsize)
    {
        // TODO
    }

    // Insert element to tail. The logical front of the buffer.
    void push_front(value_type val)
    {

        // Pushing to tail grows the buffer backwards.
        decrement(m_tailIndex);
        if(m_tailIndex == m_headIndex)
        {
            // TODO Current implementation requires the buffer to be rotated to match physical layout before reserve.
            // Otherwise buffer will end up cut in half if it has wrapped around. Wonder how vector would deal with uncontinuous structure.
            reserve(floor(m_capacity* 1.3) + 2);
        }
        // Pushing to front proposes a different problem with std::vector. Pushing back can be solved by resizing,
        // but pushing to front should push the element possibly to the physical back of the vector. To make vector recognize this element,
        // The resize should take up the whole capacity. TODO
        T* _ptr = new(m_data + sizeof(T) * m_tailIndex) T(val);
    }

    /// @brief Inserts an element in the back of the buffer. If buffer is full, allocates more memory.
    /// @param val Value of type T to be inserted in to the buffer.
    // TODO corner cases : first element, buffer full, buffer full and index at border. Poor design atm
    void push_back(value_type val)
    {
        // Empty buffer case.
        if(m_capacity == size())
        {
            reserve(m_capacity * 1.5 + 1 );
            T* _ptr = new(&m_data[m_headIndex]) T(val);
            increment(m_headIndex);
        }
        else
        {
            // Adds element, and after checks for if more capacity is needed. This pre-reserves memory for next element.
            // The buffer is full for only a brief moment.
            //m_data.resize(size() + 1);
            T* _ptr = new(&m_data[m_headIndex]) T(val);
            increment(m_headIndex);
            if(m_tailIndex == m_headIndex)
            {
                decrement(m_headIndex);
                reserve(m_capacity * 1.5);
                increment(m_headIndex);
            }
        }
    }

    // Erases an element from logical front of the buffer.
    void pop_front()
    {
        m_allocator.destroy(&m_data[m_tailIndex]);
        increment(m_tailIndex);
    }

    // Erase an element from the logical back of the buffer.
    void pop_back()
    {
        decrement(m_headIndex);
        m_allocator.destroy(m_data[m_headIndex-1]);
    }

//===========================================================
//  std::queue adaptor functions
//===========================================================

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
        // Since head points to next-to-last element, it needs to be decremented once to get the correct element. 
        // If the index is at the beginning border of the allocated memory area it needs to be wrapped around. 
        if (m_headIndex == 0)
        {
            return m_data[m_capacity - 1];
        }
        return m_data[m_headIndex-1];
    }

    const_reference back() const
    {
        // Since head points to next-to-last element, it needs to be decremented once to get the correct element. 
        // If the index is at the beginning border of the allocated memory area it needs to be wrapped around. 
        if (m_headIndex == 0)
        {
            return *m_data[m_capacity - 1];
        }
        return *m_data[m_headIndex-1];
    }

private:
    void increment(size_t& index)
    {   
        ++index;
        // Reaching equal is past the last element, then wraps around.
        if(index >= m_capacity)
        {
            index = 0;
        }
    }

    void decrement(size_t& index)
    {

        if(index == 0)
        {
            index = m_capacity - 1;
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
    size_type m_headIndex; /* Index of the last element in the buffer.*/ 
    size_type m_tailIndex;/* Index of the first element in the buffer.*/

    size_type m_capacity;
    T* m_data;
    Allocator m_allocator;
    //std::vector <T,Allocator> m_data;/* Underlying vector to store the data*/
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