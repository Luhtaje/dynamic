#ifndef TEST_HPP
#define TEST_HPP

#include <vector>
#include <iostrem>  

/// @brief Clas Template for testing purposes. This is just a test block for doxygen documentation generation. 
/// @tparam T Class has typename T pointer member
template<typename T>
class TestClass
{
public:
    
    TestClass(); /*< Default constructor*/

    T* ptr; /*< Typename T pointer*/
    int number; /*< Member integer named number. Testing after-block documentation*/
    private:

};

template<typename T>
TestClass<T>::TestClass(){

}

#endif /*TEST_HPP*/