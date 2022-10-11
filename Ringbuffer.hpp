#ifndef MAIN_HPP
#define MAIN_HPP

#include <memory>
#include <vector>

//template<template <typename T, typename Allocator = std::allocator<T>> class RingBuffer>
//using RingBufferSharedPtr = shared_ptr<RingBuffer<T, Allocator>>;

template<typename T, typename Allocator = std::allocator<T>> 
class RingBuffer
{
public:
   
    //RingBuffer();

    RingBuffer data();
    //size_type size();

private:
    std::vector <T,Allocator> m_data;
    size_t m_beginIndex;
    size_t m_endIndex;
};
#endif /*MAIN_HPP*/