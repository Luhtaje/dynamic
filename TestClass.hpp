#ifndef TEST_HPP
#define TEST_HPP

#include <vector>
#include <iostrem>  

/// @brief Clas Template for testing purposes. This is just a test block for doxygen documentation generation. 
/// @tparam T Class has typename T pointer member
template<typename T,typename Allocator = std::allocator<T>>
class TestClass
{
public:
	
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