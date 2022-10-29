#ifndef RAWITERATOR_HPP
#define RAWITERATOR_HPP

#if 0
#include <iterator>

/// @brief Custom iterator class.
/// @tparam T Type of the element what iterator points to.
template<class _rBuf>
class Iterator
{

public:
    using

public:
    /// @brief Constructor.
    /// @param ptr Raw pointer to an element in container of type T.
    Iterator(_rBuf* ptr = nullptr){m_ptr = ptr};

    /// @brief default copy constructor.
    /// @param Iterator 
    Iterator(const Iterator<_rBuf>& Iterator) = default;

    /// Destructor
    ~Iterator(){};

    /// @brief Default assingment operator overload.
    /// @param Iterator Source iterator to assign from
    Iterator<_rBuf>& operator=(const Iterator<_rBuf>& Iterator) =default;

    /// @brief Custom assingment operator overload.
    /// @param ptr Raw source pointer to assign from.
    Iterator<_rBuf>& operator=(_rBuf* ptr)
    {
        m_ptr = ptr;
        return (*this);
    };

    /// @brief Conversion operator, allows iterator to be converted to typename bool.
    operator bool() const
    {
        if(m_ptr)
            return true;
        else
            return false;
    }

    /// @brief Moves iterator forward.
    /// @param movement Amount of elements to move.
    Iterator<_rBuf>& operator+=(const ptrdiff_t& movement)
    {
        m_ptr += movement
        return (*this);    
    }

    /// @brief Moves iterator backwards.
    /// @param movement Amount of elements to move.
    Iterator<_rBuf>& operator-=(const ptrdiff_t&  movement)
    {
        m_ptr -= movement;
        return (*this);
    }

    /// @brief Move iterator forward by ony element.
    Iterator<_rBuf>& operator++()
    {
        ++m_ptr;
        return (*this);
    }

    /// @brief Move iterator backward by one element.
    Iterator<_rBuf>& operator--()
    {
        --m_ptr;
        return(*this);
    }

    /// @brief Move iterator forward by one element.
    /// @param  int empty parameter to guide overload resolution.
    Iterator<_rBufT> operator++(int)
    {
        auto temp (*this);
        ++m_ptr;
        return temp; 
    }

    /// @brief Move iterator backwards by one element.
    /// @param  int empty parameter to guide overload resolution.
    Iterator<_rBuf> operator--(int)
    {
        auto temp (*this);
        --m_ptr;
        return temp;
    }

    /// @brief Move iterator forward by specified amount.
    /// @param movement Amount of elements to move the iterator.
    Iterator<_rBuf> operator+(const int& movement)
    {
        auto oldPtr = this->m_ptr;
        this->m_ptr-=movement;
        auto temp(*this);
        this->m_ptr = oldPtr;
        return temp;
    }

    /// @brief Gets distance between two iterators.
    /// @param iterator Iterator to get distance to.
    /// @return Amount of elements between the iterators.
    ptrdiff_t operator-(const Iterator<_rBuf>& iterator)
    {
        return std::distance(iterator.getPtr(),this->getPtr());
    }

    /// @brief Dereference operator.
    /// @return Object pointed by iterator.
    _rBuf& operator*()
    {
        return *m_ptr;
    }

    /// @brief Const dereference operator
    /// @return  Const object pointed by iterator.
    const _rBuf& operator*() const
    {
        return *m_ptr;    
    }

    /// @brief 
    /// @return 
    _rBuf& operator->()
    {
        return m_ptr;
    }

    _rBuf* getPtr()
    {
        return m_ptr;
    }

    const _rBuf* getConstPtr()
    {
        return m_ptr;
    }
protected:
    T* m_ptr;
}
#endif
#endif RAWITERATOR_HPP          