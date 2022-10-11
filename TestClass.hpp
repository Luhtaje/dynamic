#ifndef TEST_HPP
#define TEST_HPP

#include <vector>
template<typename T>
class TestClass
{
public:

    TestClass(std::vector<T>);
    T* ptr;
    std::vector<T> m_data;
    private:

};

#endif /*TEST_HPP*/