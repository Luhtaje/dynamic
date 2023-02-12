#ifndef MAIN_HPP
#define MAIN_HPP

#include <memory>
#include <algorithm>
#include "Iterator.hpp"

//Forward decl
template<typename T, typename Allocator = std::allocator<T>>
class RingBuffer;

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

    using difference_type = ptrdiff_t;
    using size_type = std::size_t;

public:


    /// @brief Default constructor.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @note Allocates memory for 2 elements. The buffer works on a principle that it never gets full.
    RingBuffer() : m_headIndex(0), m_tailIndex(0), m_capacity(2)
    {
        m_data = m_allocator.allocate(2);
    }

    /// @brief Custom constructor. Initializes a buffer to a size without constructing any elements.
    /// @param size Capacity of the buffer.
    RingBuffer(size_type size) : m_headIndex(0), m_tailIndex(0)
    {
        m_capacity = size + 2;
        m_data = m_allocator.allocate(m_capacity);
    }

    /// @brief Custom constructor. Constructs the buffer and initializes all of its elements to a given value.
    /// @param size Size of the buffer.
    /// @param val Value which the elements are initialized to.
    RingBuffer(size_type size, T val) : m_headIndex(0), m_tailIndex(0)
    {
        m_capacity = size + 2;
        m_data = m_allocator.allocate(m_capacity);
        for(int i = 0 ; i < size ;  i++)
        {
            push_back(val);
        }
    }

    /// @brief Initializer list contructor.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @note Allocates memory for 2 extra elements.
    RingBuffer(std::initializer_list<T> init): m_headIndex(0), m_tailIndex(0), m_capacity(init.size() + 2)
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
        
        // Copies the buffer by calling copyconstructor on each element. If T is triviallyCopyable memmove or something similiar might be used.
        std::copy(rhs.cbegin(), rhs.cend(), begin());
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
    RingBuffer& operator=(RingBuffer&& other)
    {
        RingBuffer copy(std::move(other));
        copy.swap(*this);
        return *this;
    }

    //Destructor
    ~RingBuffer()
    {
        // Calls destructor for each element in the buffer.
        for_each(begin(),end(),[](T elem) { elem.~T(); });

        // After destruction deallocate the memory.
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

    /// @brief Allocates more memory and copies the existing buffer to the new memory location.
    /// @throw Throws std::bad_alloc if there is not enough memory for allocation. Throws std::bad_array_new_lenght if std::numeric_limits<std::size_t>::max() / sizeof(T) < newsize.
    /// @param newsize Amount of memory to allocate.
    void reserve(size_type newsize)
    {
        if(newsize <= m_capacity) return;

        // Temporary buffer to help with copying the values to new memory location. -2 because the constructor adds 2 elements by default. This is far from optimal.
        auto temp =  RingBuffer<T>(newsize - 2);
        temp.m_headIndex = m_headIndex;
        temp.m_tailIndex = m_tailIndex;

        // Copy the buffer if it has wrapped around, ends needs to touch borders with the allocated memory area. No floating head or tail is allowed.
        if(m_headIndex < m_tailIndex)
        {
            // Copy the elements at physical start only if such exists.
            if(m_headIndex)
            {
                // Iterator refer to physical begin, not logical as the begin() member function does.
                auto sourceBeginIt = const_iterator(this, 0);
                auto destBeginIt = iterator(&temp, 0);

                // Copy beginning sequence of wrapped buffer.
               std::copy(sourceBeginIt, cend(), destBeginIt);
            }

            // Copy end sequence of buffer.
            auto sourceEndIt = const_iterator(this, m_capacity);
            // This iterator refers to the first element in the seuqence which touches the end border of the new memory area.
            auto destEndIt = iterator(&temp, newsize - m_capacity + m_tailIndex);

            // Copy the ending sequence of a wrapped buffer
            std::copy(cbegin(), sourceEndIt, destEndIt);
        }
        else
        {
            // Simple copy to new location.
            std::copy(cbegin(), cend(), temp.begin());
        }

        // Assings the data from temp to original buffer. The resources from temp will be released when function goes out of scope.
        this->swap(temp);
    }

    /// @brief Inserts an element to the front of the buffer.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @param val Element to insert.
    void push_front(value_type val)
    {
        if(m_capacity - 1 == size())
        {
            reserve(m_capacity* 1.5);
        }
        decrement(m_tailIndex);
        T* _ptr = new(&m_data[m_tailIndex]) T(val);
    }

    /// @brief Inserts an element in the back of the buffer. If buffer is full, allocates more memory.
    /// @throw Might throw std::bad_alloc if there is not enough memory for allocation.
    /// @param val Value of type T to be inserted in to the buffer.
    void push_back(value_type val)
    {
        // Empty buffer case.
        if(m_capacity - 1 == size())
        {
            reserve(m_capacity * 1.5);
        }

        T* _ptr = new(&m_data[m_headIndex]) T(val);
        increment(m_headIndex);
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