#ifndef MAIN_HPP
#define MAIN_HPP

#include <memory>
#include <iterator>
#include <vector>
#include <iostream>

template<typename T>
using ringBuffer = std::vector<T>;

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

    void increment(T, int);

    RingBuffer data();
    //size_type size();

private:
    std::shared_ptr<RingBuffer<T,Allocator>> next;
    std::vector <T,Allocator> m_data;
    size_t m_beginIndex;
    size_t m_endIndex;
};

//Testing syntax stuff
template<typename T, typename Allocator>
void RingBuffer<T,Allocator>::increment(T type, int steps)
{
    std::vector<_Ty>::iterator = m_data.begin();
}

#endif /*MAIN_HPP*/