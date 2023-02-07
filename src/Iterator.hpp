//Forward declarations
//============================================================================
template<typename _rBuf>
class _rBuf_iterator;
//============================================================================

/// @brief Custom iterator class.
/// @tparam _rBuf Ringbuffer class type.
template<class _rBuf>
class _rBuf_const_iterator
{

public:
    using iterator_category = std::random_access_iterator_tag;

    using value_type = typename _rBuf::value_type;
    using difference_type = size_t;
    using pointer = typename _rBuf::const_pointer;
    using reference = const value_type&;
    //using _Tptr = typename _rBuf::pointer;

public:

    /// @brief Constructor.
    /// @param index Index representing the logical element of the.
    _rBuf_const_iterator(const _rBuf* container = nullptr, int index = -1): m_container(container), m_logicalIndex(index) {}

    /// @brief Conversion constructor
    /// @param const_iterator const iterator to construct from.
    _rBuf_const_iterator(_rBuf_iterator<_rBuf>& iterator) : m_container(iterator.m_container), m_logicalIndex(iterator.m_logicalIndex) {}

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
    _rBuf_const_iterator& operator++()
    {
        //TODO: check value-initialization and over end() increment.
        
        m_logicalIndex++;
        return (*this);
    }

    /// @brief Postfix increment
    /// @param  int empty parameter to guide overload resolution.
    _rBuf_const_iterator operator++(int)
    {
        //TODO: check value-initialization and over end() increment.
        auto temp (*this);
        ++m_logicalIndex;
        return temp;
    }

    /// @brief Prefix decrement
    _rBuf_const_iterator& operator--()
    {
        //TODO:    check value initialization and "under" begin() decrement.
        --m_logicalIndex;
        return(*this);
    }

    /// @brief Postfix decrement
    /// @param  int empty parameter to guide overload resolution.
    _rBuf_const_iterator operator--(int)
    {
        //TODO:    check value initialization and "under" begin() decrement.
        auto temp (*this);
        --m_logicalIndex;
        return temp;
    }

    /// @brief Moves iterator forward.
    /// @param movement Amount of elements to move.
    _rBuf_const_iterator& operator+=(difference_type offset)
    {
        m_logicalIndex += offset;
        return (*this);
    }

    // "Deprecated" for now, nos apparent use case and causes ambiguous function call with some C++ internal operator+ overload.
    /// @brief Move iterator forward by specified amount.
    /// @param movement Amount of elements to move the iterator.
    // _rBuf_const_iterator operator+(const difference_type offset)
    // {
    //     _rBuf_const_iterator temp = *this;
    //     return (temp + offset);
    // }

    /// @brief Moves iterator backwards.
    /// @param movement Amount of elements to move.
    _rBuf_const_iterator& operator-=(const difference_type offset)
    {
        return (*this += -offset);
    }

    /// @brief Gets distance between two iterators.
    /// @param iterator Iterator to get distance to.
    /// @return Amount of elements between the iterators.
    difference_type operator-(const _rBuf_const_iterator& iterator)
    {
        //TODO: compatibility check
        return (m_logicalIndex - iterator.m_logicalIndex);
    }

    /// @brief index operator
    /// @param index index
    /// @return reference to the element at index
    reference operator[](const difference_type index) const
    {
        return m_container[m_logicalIndex + index];
    }

    /// @brief Comparison operator== overload
    /// @param other iterator to compare
    /// @return true if underlying pointers are the same.
    bool operator==(const _rBuf_const_iterator& other) const
    {
        return m_logicalIndex == other.m_logicalIndex && m_container == other.m_container;
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

    /// @brief Default assingment operator overload.
    /// @param iterator Source iterator to assign from
    _rBuf_const_iterator& operator=(const _rBuf_const_iterator<_rBuf>& iterator) =default;

    /// @brief Custom assingment operator overload.
    /// @param index Logical index of the element which point to.
    _rBuf_const_iterator& operator=(size_t index) const
    {
        m_logicalIndex = index;
        return (*this);
    };

    /// @brief Dereference operator.
    /// @return Object pointed by iterator.
    reference operator*()
    {
        return (*m_container)[m_logicalIndex];
    }
private:
    //What container instance is this iterator for.
    const _rBuf* m_container;
    
    // The iterator does not point to any memory location, but is interfaced to the Ring Buffer via an index which is the logical index
    // to an element. Logical index 0 is the first element in the buffer. 
    int m_logicalIndex;
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

    /// @brief Constructor.
    /// @param index Index pointing to the logical element of the RingBuffer.
    _rBuf_iterator(_rBuf* container = nullptr, int index = -1): m_container(container), m_logicalIndex(index) {}

    /// @brief Const dereference operator
    /// @return  Const object pointed by iterator.
    reference operator*() const
    {
        //TODO: bounds checking and value-initialization.
        return m_container[m_logicalIndex];
    }

    /// @brief
    /// @return 
    pointer operator->() const
    {
        //TODO: check value initialization and bounds.
        return &(*m_container)[m_logicalIndex];
    }

    /// @brief Prefix increment
    _rBuf_iterator& operator++()
    {
        //TODO: check value-initialization and over end() increment.
        ++m_logicalIndex;
        return (*this);
    }

    /// @brief Postfix increment
    /// @param  int empty parameter to guide overload resolution.
    _rBuf_iterator operator++(int)
    {
        //TODO: check value-initialization and over end() increment.
        auto temp (*this);
        ++m_logicalIndex;
        return temp; 
    }

    /// @brief prefix decrement
    _rBuf_iterator& operator--()
    {
        //TODO:    check value initialization and "under" begin() decrement
        --m_logicalIndex;
        return(*this);
    }

    /// @brief Postfix decrement
    /// @param  int empty parameter to guide overload resolution.
    _rBuf_iterator operator--(int)
    {
        //TODO:    check value initialization and "under" begin() decrement
        auto temp (*this);
        --m_logicalIndex;
        return temp;
    }

    /// @brief Moves iterator forward.
    /// @param movement Amount of elements to move.
    _rBuf_iterator& operator+=(difference_type offset)
    {
        m_logicalIndex += offset;
        return (*this);
    }
    // "Deprecated" for now, no apparent use case and causes ambiguous function call with some C++ internal operator+ overload.
    /// @brief Move iterator forward by specified amount.
    /// @param movement Amount of elements to move the iterator.
    // _rBuf_iterator operator+(const difference_type offset)
    // {
    //     _rBuf_iterator temp = *this;
    //     return (temp + offset);
    // }

    /// @brief Moves iterator backwards.
    /// @param movement Amount of elements to move.
    _rBuf_iterator& operator-=(const difference_type offset)
    {
        return (*this += -offset);
    }

    /// @brief Gets distance between two iterators.
    /// @param iterator Iterator to get distance to.
    /// @return Amount of elements between the iterators.
    difference_type operator-(const _rBuf_iterator& iterator)
    {
        //TODO: compatibility check
        return (m_logicalIndex - iterator.m_logicalIndex);
    }

    /// @brief Index operator
    /// @param offset 
    /// @return 
    reference operator[](const difference_type index) const
    {
        return m_container[m_logicalIndex + index];
    }

    /// @brief Comparison operator== overload
    /// @param other iterator to compare
    /// @return true if underlying pointers are the same.
    bool operator==(const _rBuf_iterator& other) const
    {
        return m_logicalIndex == other.m_logicalIndex && m_container == other.m_container;
    }

    /// @brief Comparison operator != overload
    /// @param other iterator to compare
    /// @return ture if underlying pointers are not the same
    bool operator!=(const _rBuf_iterator& other) const
    {
        return !(m_logicalIndex == other.m_logicalIndex && m_container == other.m_container);
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

    /// @brief Default assingment operator overload.
    /// @param iterator Source iterator to assign from
    _rBuf_iterator& operator=(const _rBuf_iterator<_rBuf>& iterator) =default;

    /// @brief Custom assingment operator overload.
    /// @param index Logical index of the element to set the iterator to.
    _rBuf_iterator& operator=(difference_type index) const
    {
        m_logicalIndex = index;
        return (*this);
    };

    /// @brief Dereference operator.
    /// @return Object pointed by iterator.
    reference operator*()
    {
        return (*m_container)[m_logicalIndex];
    }

    //What container is this iterator for.
    _rBuf* m_container;
    
    // Offset from physical start from the buffer
    int m_logicalIndex;
};
