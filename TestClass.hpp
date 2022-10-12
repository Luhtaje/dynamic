#ifndef TEST_HPP
#define TEST_HPP

#include <vector>

template<typename T>
class TestClass
{
public:

    TestClass();
    
    T* ptr;
    int number;
    private:

};

template<typename T>
TestClass<T>::TestClass(){

}

#endif /*TEST_HPP*/