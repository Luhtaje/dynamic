#ifndef TEST_HPP
#define TEST_HPP

#include <vector>

template<typename T,typename Allocator = std::allocator<T>>
class TestClass
{
public:
    // Some constructor 
    TestClass(T,Allocator);
    void foo();
};

template<typename T, typename Allocator>
TestClass<T,Allocator>::TestClass(T a, Allocator alloc){

}

template<typename T, typename Allocator>
void TestClass<T,Allocator>::foo()
{
    int i;
    i++;
}


#endif /*TEST_HPP*/