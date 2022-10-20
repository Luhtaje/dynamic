#ifndef MAIN_HPP
#define MAIN_HPP

#include <memory>
#include <iterator>
#include <vector>
#include <algorithm>
¤include <iterator>
//template<typename T>
//using RingBufferSharedPtr = shared_ptr<RingBuffer<T, Allocator>>;
using size_type = size_t;

/// @brief Dynamic Ringbuffer is a dynamically growing std::container with support for queue, stack and priority queue adaptor functionality. 
/// @tparam T type of the ringbuffer
/// @tparam Allocator optional allocator for underlying vector. Defaults to std::allocator<T>
template<typename T, typename Allocator = std::allocator<T>> 
class RingBuffer
{

public:
    /// @brief Default constructor.
    RingBuffer() = default;

    /// @brief Custom constructor.
    /// @param size Size of the buffer to initialize.
    /// @param val Value to set every element to.
    RingBuffer(size_type size, T val = 0)
    {
        m_data.resize(size);

        for(auto elem : m_data){
            elem = val;
        }
    }

    /// @brief Default move constructor.
    /// @param  RinfBuffer&&  Rval reference to a RingBuffer
    RingBuffer(RingBuffer&&)=default;

    /// @brief Default copy constructor.
    /// @param b Buffer to construct from.
    RingBuffer(const RingBuffer& b)=default;
	

    ~RingBuffer()=default;

	/// @brief Sorts ringbuffer so that logical head matches the first element in physical memory.
    /// @return Pointer to the first element.
    RingBuffer data();

    /// @brief Gets the size of the underlying container.
    /// @return Size of buffer.
    size_type size(){
        return m_data.size();
    }

    /// @brief Check if buffer is empty
    /// @return True if buffer is empty
    bool empty(){
        return m_data.empty();
    }

    /// @brief Resizes the container so that it containes n elements.
    /// @param n Size to resize to.
    void resize(size_type n){
        m_data.resize(n);
    }

    /// @brief Resize overload.
    /// @param n Size to resize to.
    /// @param val Value to initialize new elements to, if N is larger than current container capacity.
    void resize(size_type n, const T& val){
        m_data.resize(n,val);
    }
    
    struct Iterator<T>{
        using iterator_category = std::
    }


private:

    std::vector <T,Allocator> m_data;/*< Underlying vector to store the data in the buffer*/
    size_t m_beginIndex; /*< Index to the first element in the buffer*/
    size_t m_endIndex;/*< Index to the last element in the buffer*/

};

#endif /*MAIN_HPP*/