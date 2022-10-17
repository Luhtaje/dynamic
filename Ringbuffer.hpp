#ifndef MAIN_HPP
#define MAIN_HPP

#include <memory>
#include <iterator>
#include <vector>

//template<typename T>
//using RingBufferSharedPtr = shared_ptr<RingBuffer<T, Allocator>>;

/// @brief Dynamic Ringbuffer is a dynamically growing std::container with support for queue, stack and priority queue adaptor functionality. 
/// @tparam T type of the ringbuffer
/// @tparam Allocator optional allocator for underlying vector. Defaults to std::allocator<T>
template<typename T, typename Allocator = std::allocator<T>> 
class RingBuffer
{
public:
    
    RingBuffer(T a)
    {
        
    }

    RingBuffer(T a, Allocator alloc)
    {
        
    }

    RingBuffer(const RingBuffer& b)
    {
        m_beginIndex = b.m_beginIndex;
        m_endIndex = b.m_endIndex;
        m_data = b.m_data;
    }
	
	//tester
    void increment(T, int);
	
	/// @brief Sorts ringbuffer so that logical head matches the first element in physical memory. 
    /// @return Pointer to the first element
    RingBuffer data();
    //size_type size();

private:
    std::vector <T,Allocator> m_data;/*< Underlying vector to store the data in the buffer*/
    size_t m_beginIndex; /*< Index to the first element in the buffer*/
    size_t m_endIndex;/*< Index to the last element in the buffer*/
};


template<typename T, typename Allocator>
void RingBuffer<T,Allocator>::increment(T type, int steps)
{
    std::vector<T>::iterator = m_data.begin();
}
#endif /*MAIN_HPP*/