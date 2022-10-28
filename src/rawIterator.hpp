#ifndef RAWITERATOR_HPP
#define RAWITERATOR_HPP

#include <iterator>

/// @brief Custom iterator class.
/// @tparam T Type of the element what iterator points to.
template<typename T>
class RawIterator : public std::<std::random_access_iterator_tag,
                                T,
                                Distance = std::ptrdiff_t,
                                Pointer = T*,
                                Reference = T&>
{
    public:
        /// @brief Constructor.
        /// @param ptr Raw pointer to an element in container of type T.
        RawIterator(T* ptr = nullptr){m_ptr = ptr};

        /// @brief default copy constructor.
        /// @param rawIterator 
        RawIterator(const RawIterator<T>& rawIterator) = default;

        /// Destructor
        ~RawIterator(){};

        /// @brief Default assingment operator overload.
        /// @param Iterator Source iterator to assign from
        Rawiterator<T>& operator=(const RawIterator<T>& Iterator) =default;

        /// @brief Custom assingment operator overload.
        /// @param ptr Raw source pointer to assign from.
        RawIterator<T>& operator=(T* ptr)
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
        RawIterator<T>& operator+=(const ptrdiff_t& movement)
        {
            m_ptr += movement
            return (*this);    
        }

        /// @brief Moves iterator backwards.
        /// @param movement Amount of elements to move.
        RawIterator<T>& operator-=(const ptrdiff_t&  movement)
        {
            m_ptr -= movement;
            return (*this);
        }

        /// @brief Move iterator forward by ony element.
        RawIterator<T>& operator++()
        {
            ++m_ptr;
            return (*this);
        }

        /// @brief Move iterator backward by one element.
        RawIterator<T>& operator--()
        {
            --m_ptr;
            return(*this);
        }

        /// @brief Move iterator forward by one element.
        /// @param  int empty parameter to guide overload resolution.
        RawIterator<T> operator++(int)
        {
            auto temp (*this);
            ++m_ptr;
            return temp; 
        }

        /// @brief Move iterator backwards by one element.
        /// @param  int empty parameter to guide overload resolution.
        RawIterator<T> operator--(int)
        {
            auto temp (*this);
            --m_ptr;
            return temp;
        }

        /// @brief Move iterator forward by specified amount.
        /// @param movement Amount of elements to move the iterator.
        RawIterator<T> operator+(const int& movement)
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
        ptrdiff_t operator-(const RawIterator<T>& iterator)
        {
            return std::distance(iterator.getPtr(),this->getPtr());
        }

        /// @brief Dereference operator.
        /// @return Object pointed by iterator.
        T& operator*()
        {
            return *m_ptr;
        }

        /// @brief Const dereference operator
        /// @return  Const object pointed by iterator.
        const T& operator*() const
        {
            return *m_ptr;    
        }

        /// @brief 
        /// @return 
        T& operator->()
        {
            return m_ptr;
        }

        T* getPtr()
        {
            return m_ptr;
        }

        const T* getConstPtr()
        {
            return m_ptr;
        }
    protected:
        T* m_ptr;
}

#endif RAWITERATOR_HPP          