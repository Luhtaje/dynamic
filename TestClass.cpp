#include "TestClass.hpp"


template <typename T> TestClass<T>::TestClass(std::vector<T> data)
{
    m_data = data;
}