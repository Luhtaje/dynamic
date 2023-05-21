#ifndef ITERATOR_HPP
#define ITERATOR_HPP

#include <iterator>
//Forward declarations
//============================================================================
template<typename _rBuf>
class _rBuf_iterator;
//============================================================================

/// @brief Custom iterator class.
/// @tparam _rBuf ring_buffer class type.
template<class _rBuf>
class _rBuf_const_iterator
{

public:
    using iterator_category = std::random_access_iterator_tag;

    using value_type = typename _rBuf::value_type;
    using difference_type = typename _rBuf::difference_type;
    using pointer = typename _rBuf::const_pointer;
    using reference = const value_type&;

public:
    _rBuf_const_iterator(): m_container(nullptr), m_logicalIndex(0) {}

    /// @brief Constructor.
    /// @param index Index representing the logical element of the.
    explicit _rBuf_const_iterator(const _rBuf* container, difference_type index): m_container(container), m_logicalIndex(index) {}

    /// @brief Conversion constructor
    /// @param const_iterator const iterator to construct from.
    _rBuf_const_iterator(const _rBuf_iterator<_rBuf>& iterator) : m_container(iterator.m_container), m_logicalIndex(iterator.m_logicalIndex) {}

    /// @brief Conversion assingment from non-const iterator
    /// @param iterator non-const iterator.
    /// @return Returns the new object by reference
    _rBuf_const_iterator& operator=(_rBuf_iterator<_rBuf>& iterator)
    {
        if(m_logicalIndex == iterator.m_logicalIndex && m_container == iterator.m_container)
        {
            return *this;
        }

        m_logicalIndex = iterator.m_logicalIndex;
        m_container = iterator.m_container;
        return *this;
    }

    /// @brief Arrow operator.
    /// @return pointer.
    pointer operator->() const
    {
        //TODO: check value initialization and bounds.
        return &(*m_container)[m_logicalIndex];
    }

    /// @brief Postfix increment
    /// @note The index of the iterator can be incremented over the border of its owning buffer. The buffer converts logical index to correct element.
    /// See Iterator::operator* and ring_buffer::operator[].
    _rBuf_const_iterator& operator++()
    {
        m_logicalIndex++;

        return (*this);
    }

    /// @brief Postfix increment
    /// @param  int empty parameter to guide overload resolution.
    /// @note The index of the iterator can be incremented over the border of its owning buffer. The buffer converts logical index to correct element.
    /// See Iterator::operator* and ring_buffer::operator[].
    _rBuf_const_iterator operator++(int)
    {
        auto temp (*this);
        ++m_logicalIndex;

        return temp;
    }

    /// @brief Prefix decrement
    _rBuf_const_iterator& operator--()
    {
        --m_logicalIndex;
        if(m_logicalIndex < 0)
        {
            m_logicalIndex = m_container->capacity() - 1;
        }
        return(*this);
    }

    /// @brief Postfix decrement
    /// @param  int empty parameter to guide overload resolution.
    _rBuf_const_iterator operator--(int)
    {
        //TODO:    check value initialization and "under" begin() decrement.
        auto temp (*this);
        --m_logicalIndex;
        if(m_logicalIndex < 0)
        {
            m_logicalIndex = m_container->capacity() - 1;
        }
        return temp;
    }

    /// @brief Moves iterator.
    /// @param offset Amount of elements to move. Negative values move iterator backwards.
    _rBuf_const_iterator& operator+=(difference_type offset)
    {
        const auto capacity = m_container->capacity();
        // TODO : make this better. Absolutely terrible
        if(offset < 0)
        {
            offset = abs(offset) % capacity;
            if(offset > m_logicalIndex)
            {
                m_logicalIndex = capacity - (offset - m_logicalIndex);
            }
            else
            {
                m_logicalIndex = m_logicalIndex - offset;
            }
        }
        else 
        {
            offset = offset % capacity;
            m_logicalIndex += offset;
            if(m_logicalIndex >= capacity)
            {
                m_logicalIndex -= capacity;
            }
        }
        return (*this);
    }

    /// @brief Move iterator forward by specified amount.
    /// @param movement Amount of elements to move the iterator.
    _rBuf_const_iterator operator+(const difference_type offset) const
    {
        _rBuf_const_iterator temp(m_container, m_logicalIndex);
        return (temp += offset);
    }

    /// @brief Returns an iterator that points to an element, which is the current element decremented by the given offset.
    /// @param offset The number of positions to move the iterator backward.
    /// @return An iterator pointing to the element that is offset positions before the current element.
    /// @note Undefined behaviour for negative offset.
    _rBuf_const_iterator& operator-=(const difference_type offset)
    {
        return (*this += -offset);
    }

    /// @brief Returns an iterator that points to an element, which is the current element decremented by the given offset.
    /// @param offset The number of positions to move the iterator backward.
    /// @return An iterator pointing to the element that is offset positions before the current element.
    /// @note Undefined behaviour for negative offset.
    _rBuf_const_iterator operator-(const difference_type offset) const
    {
        _rBuf_const_iterator temp(m_container, m_logicalIndex);
        return (temp -= offset);
    }

    /// @brief Gets distance between two iterators.
    /// @param iterator Iterator to get distance to.
    /// @return Amount of elements between the iterators.
    difference_type operator-(const _rBuf_const_iterator& other) const
    {
        return (m_logicalIndex - other.m_logicalIndex);
    }

    /// @brief Index operator.
    /// @param offset The offset from iterator.
    /// @return Return reference to element pointed by the iterator with offset.
    reference operator[](const difference_type offset) const
    {
        return m_container->operator[](m_logicalIndex + offset);
    }

    /// @brief Comparison operator== overload
    /// @param other iterator to compare
    /// @return True if iterators point to same element in same container. Does not compare c-qualification.
    bool operator==(const _rBuf_const_iterator& other) const
    {
        const auto pointsToSame = (m_logicalIndex == other.m_logicalIndex);
        return pointsToSame && m_container == other.m_container;
    }

    /// @brief Comparison operator != overload
    /// @param other iterator to compare
    /// @return ture if underlying pointers are not the same
    bool operator!=(const _rBuf_const_iterator& other) const
    {
        return !(m_logicalIndex == other.m_logicalIndex && m_container == other.m_container);
    }

    /// @brief Comparison operator < overload
    /// @param other iterator to compare
    /// @return true if other is larger.
    bool operator<(const _rBuf_const_iterator& other) const
    {
        return (m_logicalIndex < other.m_logicalIndex);
    }

    /// @brief Comparison operator > overload
    /// @param other iterator to compare
    /// @return true if other is smaller.
    bool operator>(const _rBuf_const_iterator& other) const
    {
        //TODO:compatibility check
        return (other.m_logicalIndex < m_logicalIndex);
    }

    bool operator<=(const _rBuf_const_iterator& other) const
    {
        //TODO:compatibility check
        return: (!(other < m_logicalIndex));
    }

    bool operator>=(const _rBuf_const_iterator& other) const
    {
        return (!(m_logicalIndex < other.m_logicalIndex));
    }

    /// @brief Custom assingment operator overload.
    /// @param index Logical index of the element which point to.
    _rBuf_const_iterator& operator=(size_t index) const
    {
        m_logicalIndex = index;
        return (*this);
    };

    /// @brief Dereference operator.
    /// @return Object pointed by iterator.
    reference operator*() const
    {
        return (*m_container)[m_logicalIndex];
    }

    difference_type getIndex() noexcept
    {
        return m_logicalIndex;
    }
    
private:
    // The parent container.
    const _rBuf* m_container;

    // The iterator does not point to any memory location, but is interfaced to the Ring Buffer via an index which is the logical index
    // to an element. Logical index 0 is the first element in the buffer and last is size - 1.
    difference_type m_logicalIndex;
};


/// @brief Custom iterator class.
/// @tparam T Type of the element what iterator points to.
template<class _rBuf>
class _rBuf_iterator
{

public:
    using iterator_category = std::random_access_iterator_tag;

    using value_type = typename _rBuf::value_type;
    using difference_type = typename _rBuf::difference_type;
    using pointer = typename _rBuf::pointer;
    using reference = value_type&;

public:

    /// @brief  Const iterator is friend to enable conversion.
    friend class _rBuf_const_iterator<_rBuf>;

    /// @brief Default constructor
    _rBuf_iterator(): m_container(nullptr), m_logicalIndex(0) {}

    /// @brief Constructor.
    /// @param container Pointer to the ring_buffer element which owns this iterator.
    /// @param index Index pointing to the logical element of the ring_buffer.
    explicit _rBuf_iterator(_rBuf* container, difference_type index): m_container(container), m_logicalIndex(index) {}

    /// @brief Dereference operator
    /// @return  Returns the object the iterator is currently pointing to.
    reference operator*() const
    {
        //TODO: bounds checking and value-initialization.
        return (*m_container)[m_logicalIndex];
    }

    /// @brief Pointer operator.
    /// @return Returns a pointer to the object the iterator is currently pointing to.
    pointer operator->() const
    {
        //TODO: check value initialization and bounds.
        return &(*m_container)[m_logicalIndex];
    }

    /// @brief Prefix increment
    /// @note The index of the iterator can be incremented over the border of its owning buffer. The buffer converts logical index to correct element.
    /// See Iterator::operator* and ring_buffer::operator[].
    _rBuf_iterator& operator++()
    {
        ++m_logicalIndex;
        return (*this);
    }

    /// @brief Postfix increment
    /// @param  int empty parameter to guide overload resolution.
    /// @note The index of the iterator can be incremented over the border of its owning buffer. The buffer converts logical index to correct element.
    ///  See Iterator::operator* and ring_buffer::operator[].
    _rBuf_iterator operator++(int)
    {
        auto temp (*this);
        ++m_logicalIndex;
        return temp; 
    }

    /// @brief prefix decrement
    _rBuf_iterator& operator--()
    {
        --m_logicalIndex;
        if(m_logicalIndex < 0)
        {
            m_logicalIndex = m_container->capacity() - 1;
        }
        return(*this);
    }

    /// @brief Postfix decrement
    /// @param  int empty parameter to guide overload resolution.
    _rBuf_iterator operator--(int)
    {
        auto temp (*this);
        --m_logicalIndex;
        if(m_logicalIndex < 0)
        {
            m_logicalIndex = m_container->capacity() - 1;
        }
        return temp;
    }

    /// @brief Moves iterator forward.
    /// @param offset Amount of elements to move.
    _rBuf_iterator& operator+=(difference_type offset)
    {
        const auto capacity = m_container->capacity();
        // TODO : make this better. Absolutely terrible
        if(offset < 0)
        {
            offset = abs(offset) % capacity;
            if(offset > m_logicalIndex)
            {
                m_logicalIndex = capacity - (offset - m_logicalIndex);
            }
            else
            {
                m_logicalIndex = m_logicalIndex - offset;
            }
        }
        else
        {
            offset = offset % capacity;
            m_logicalIndex += offset;
            if(m_logicalIndex >= capacity)
            {
                m_logicalIndex -= capacity;
            }
        }
        return (*this);
    }

    /// @brief Move iterator forward by specified amount.
    /// @param movement Amount of elements to move the iterator.
    _rBuf_iterator operator+(const difference_type offset) const
    {
        _rBuf_iterator temp(m_container, m_logicalIndex);
        return (temp += offset);
    }

    /// @brief Returns an iterator that points to an element, which is the current element decremented by the given offset.
    /// @param offset The number of positions to move the iterator backward.
    /// @return An iterator pointing to the element that is offset positions before the current element.
    /// @note Undefined behavior for negative offset.
    _rBuf_iterator& operator-=(const difference_type offset)
    {
        return (*this += -offset);
    }

    /// @brief Returns an iterator that points to an element, which is the current element decremented by the given offset.
    /// @param offset The number of positions to move the iterator backward.
    /// @return An iterator pointing to the element that is offset positions before the current element.
    _rBuf_iterator& operator-(const difference_type offset) const
    {
        _rBuf_iterator temp(m_container, m_logicalIndex);
        return (temp -= offset);
    }

    /// @brief Gets distance between two iterators.
    /// @param iterator Iterator to get distance to.
    /// @return Amount of elements between the iterators.
    difference_type operator-(const _rBuf_iterator& other) const
    {
        //TODO: compatibility check
        return (m_logicalIndex - other.m_logicalIndex);
    }

    /// @brief Index operator.
    /// @param offset The offset from iterator.
    /// @return Return object pointer by the iterator with an offset.
    reference operator[](const difference_type offset) const
    {
        return m_container->operator[](m_logicalIndex + offset);
    }

    /// @brief Comparison operator== overload
    /// @param other iterator to compare
    /// @return True if iterators point to same element in same container. Does not compare c-qualification.
    /// @note As a matter of fact, just blatantly calls const-iterators operator, to access private members via friendness. All non-const iterators are converted to const for this comparison.
    template<typename iterator>
    bool operator==(const iterator& other) const
    {
        return _rBuf_const_iterator<_rBuf>(*this).operator==(other);
    }

    /// @brief Comparison operator != overload
    /// @param other iterator to compare
    /// @return ture if underlying pointers are not the same
    template<typename iterator>
    bool operator!=(const iterator& other) const
    {

        return !(*this == other);
    }

    /// @brief Comparison operator < overload
    /// @param other iterator to compare
    /// @return true if other is larger.
    bool operator<(const _rBuf_iterator& other) const
    {
        //TODO:compatibility check
        return (m_logicalIndex < other.m_logicalIndex);
    }

    /// @brief Comparison operator > overload
    /// @param other iterator to compare
    /// @return true if other is smaller.
    bool operator>(const _rBuf_iterator& other) const
    {
        //TODO:compatibility check
        return (other.m_logicalIndex < m_logicalIndex);
    }

    bool operator<=(const _rBuf_iterator& other) const
    {
        //TODO:compatibility check
        return: (!(other < m_logicalIndex));
    }

    bool operator>=(const _rBuf_iterator& other) const
    {
        return (!(m_logicalIndex < other.m_logicalIndex));
    }

    /// @brief Custom assingment operator overload.
    /// @param index Logical index of the element to set the iterator to.
    /// @note Undefined behaviour for negative index.
    _rBuf_iterator& operator=(size_t index)
    {
        m_logicalIndex = index;
        return (*this);
    };

    difference_type getIndex() noexcept
    {
        return m_logicalIndex;
    }

private:
    // The parent container.
    _rBuf* m_container;

    // The iterator does not point to any memory location, but is interfaced to the Ring Buffer via an index which is the logical index
    // to an element. Logical index 0 is the first element in the buffer and last is size - 1.
    difference_type m_logicalIndex;
};


/// @brief Non-member addition operator to match the expression (n + a) where a is the iterator and n an integer offset.
/// @tparam T Value type.
/// @param offset Integral type offset. Can be negative.
/// @param iterator Base iterator.
/// @return Returns an iterator that points to an element of the base iterator + offset.
template <typename T>
_rBuf_iterator<T> operator+(typename std::iterator_traits<_rBuf_iterator<T>>::difference_type offset, _rBuf_iterator<T>& iterator)
{
    auto temp = iterator;
    temp += offset;
    return temp;
}

/// @brief Non-member addition operator to match the expression (n + a) where a is the iterator and n an integer offset.
/// @tparam T Value type.
/// @param offset Integral type offset. Can be negative.
/// @param iterator Base iterator.
/// @return Returns a const-iterator that points to an element of the base iterator + offset.
template <typename T>
_rBuf_const_iterator<T> operator+(typename std::iterator_traits<_rBuf_const_iterator<T>>::difference_type offset, _rBuf_const_iterator<T>& iterator)
{
    auto temp = iterator;
    temp += offset;
    return temp;
}

#endif /*ITERATOR_HPP*/
