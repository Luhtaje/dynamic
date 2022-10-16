#ifndef MAIN_HPP
#define MAIN_HPP

#include <memory>
#include <vector>

//template<template <typename T, typename Allocator = std::allocator<T>> class RingBuffer>
//using RingBufferSharedPtr = shared_ptr<RingBuffer<T, Allocator>>;

/// @brief Dynamic Ringbuffer is a dynamically growing std::container with support for queue, stack and priority queue adaptor functionality. 
/// @tparam T type of the ringbuffer
/// @tparam Allocator optional allocator for underlying vector. Defaults to std::allocator<T>
template<typename T, typename Allocator = std::allocator<T>> 
class RingBuffer
{
public:
   
    //RingBuffer();
    
    /// @brief Sorts ringbuffer so that logical head matches the first element in physical memory. 
    /// @return Pointer to the first element
    T* data();
    //size_type size();

private:
    std::vector <T,Allocator> m_data;/*< Underlying vector to store the data in the buffer*/
    size_t m_beginIndex; /*< Index to the first element in the buffer*/
    size_t m_endIndex;/*< Index to the last element in the buffer*/
};
#endif /*MAIN_HPP*/