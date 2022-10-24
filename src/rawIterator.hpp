#ifndef RAWITERATOR_HPP
#define RAWITERATOR_HPP

#include <iterator>

/// @brief Custom iterator class to help implement FIFO and LIFO functionality.
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
        /// @return Target iterator who was assigned to.
        Rawiterator<T>& operator=(const RawIterator<T>& Iterator) =default;

        /// @brief Custom assingment operator overload.
        /// @param ptr Raw source pointer to assign from.
        /// @return Target iterator who was assigned to.
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

    protected:
        T* m_ptr;
}

#endif RAWITERATOR_HPP          