#ifndef MAIN_HPP
#define MAIN_HPP

#include <memory>

template<typename T>
using RingBufferSharedPtr = shared_ptr<RingBuffer<T>>;

template<typename T> 
class RingBuffer
{

private:

public:

    RingBufferSharedPtr getBuffer();
};
#endif /*MAIN_HPP*/