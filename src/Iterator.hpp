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
    using difference_type = typename _rBuf::difference_type;
    using pointer = typename _rBuf::const_pointer;
    using reference = const value_type&;
    //using _Tptr = typename _rBuf::pointer;

public:

    /// @brief Constructor.
    /// @param ptr Raw pointer to an element in container of type T.
    _rBuf_const_iterator(const _rBuf* container = nullptr, pointer ptr = nullptr): m_container(container), m_ptr(ptr) {}

    /// @brief Conversion constructor
    /// @param const_iterator const iterator to construct from.
    _rBuf_const_iterator(_rBuf_iterator<_rBuf>& iterator) : m_container(iterator.m_container), m_ptr(iterator.m_ptr) {}

    /// @brief Conversion assingment from non-const iterator
    /// @param iterator non-const iterator.
    /// @return Returns the new object by reference
    _rBuf_const_iterator& operator=(_rBuf_iterator<_rBuf>& iterator)
    {
        if(m_ptr == iterator.m_ptr && m_container == iterator.m_container)
        {
            return *this;
        }
        m_ptr = iterator.m_ptr;
        m_container = iterator.m_container;
        return *this;
    }

    /// @brief Dereference operator
    /// @return  Object pointed by iterator.
    reference operator*() const
    {
        //TODO: bounds checking and value-initialization.
        return *m_ptr;
    }

    /// @brief Arrow operator.
    /// @return pointer.
    pointer operator->() const
    {
        //TODO: check value initialization and bounds.
        return m_ptr;
    }

    /// @brief Postfix increment
    _rBuf_const_iterator& operator++()
    {
        //TODO: check value-initialization and over end() increment.
        
        m_ptr++;
        return (*this);
    }

    /// @brief Postfix increment
    /// @param  int empty parameter to guide overload resolution.
    _rBuf_const_iterator operator++(int)
    {
        //TODO: check value-initialization and over end() increment.
        auto temp (*this);
        ++m_ptr;
        return temp;
    }

    /// @brief Prefix decrement
    _rBuf_const_iterator& operator--()
    {
        //TODO:    check value initialization and "under" begin() decrement.
        --m_ptr;
        return(*this);
    }

    /// @brief Postfix decrement
    /// @param  int empty parameter to guide overload resolution.
    _rBuf_const_iterator operator--(int)
    {
        //TODO:    check value initialization and "under" begin() decrement.
        auto temp (*this);
        --m_ptr;
        return temp;
    }

    // Part of THE
    // void _Verify_offset(const difference_type offset) const
    // {
    //     //TODO: test this as well. Thoroughly
    //     const auto container = static_cast<const _rBuf*>(_rBuf.m_data);
    //     _STL_VERIFY(offset == 0 || m_ptr, "cannot seek value-initialized ringbuffer iterator")
    //     if(offset < 0)
    //     {
    //         _STL_VERIFY(offset >= container->at(0) - m_ptr, "cannot seek ringbuffer iterator before begin");
    //     }
    //     if(offset > 0)
    //     {
    //         _STL_VERIFY(offset <= container->at(0) - m_ptr, "cannot seek ringbuffer iterator after end")
    //     }


    /// @brief Moves iterator forward.
    /// @param movement Amount of elements to move.
    _rBuf_const_iterator& operator+=(difference_type offset)
    {
        m_ptr += offset;
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

    /// @brief Get iterator - offset.
    /// @param offset offset.
    /// @return iterator at this - offset.
    _rBuf_const_iterator operator-(const difference_type offset)
    {
        _rBuf_const_iterator temp = *this;
        return (temp - offset);
    }

    /// @brief Gets distance between two iterators.
    /// @param iterator Iterator to get distance to.
    /// @return Amount of elements between the iterators.
    difference_type operator-(const _rBuf_const_iterator& iterator)
    {
        //TODO: compatibility check
        return (m_ptr - iterator.m_ptr);
    }

    /// @brief index operator
    /// @param index index
    /// @return reference to the element at index
    reference operator[](const difference_type index) const
    {
        return (*(*this + index));
    }

    /// @brief Comparison operator== overload
    /// @param other iterator to compare
    /// @return true if underlying pointers are the same.
    bool operator==(const _rBuf_const_iterator& other) const
    {
        return m_ptr == other.m_ptr;
    }

    /// @brief Comparison operator != overload
    /// @param other iterator to compare
    /// @return ture if underlying pointers are not the same
    bool operator!=(const _rBuf_const_iterator& other) const
    {
        return m_ptr != other.m_ptr;
    }

    /// @brief Comparison operator < overload
    /// @param other iterator to compare
    /// @return true if other is larger.
    bool operator<(const _rBuf_const_iterator& other) const
    {
        return (m_ptr < other.m_ptr);
    }

    /// @brief Comparison operator > overload
    /// @param other iterator to compare
    /// @return true if other is smaller.
    bool operator>(const _rBuf_const_iterator& other) const
    {
        //TODO:compatibility check
        return (other.m_ptr < m_ptr);
    }

    bool operator<=(const _rBuf_const_iterator& other) const
    {
        //TODO:compatibility check
        return: (!(other < m_ptr));
    }

    bool operator>=(const _rBuf_const_iterator& other) const
    {
        return (!(m_ptr < other.m_ptr));
    }

    //Implement _Compat? TODO

    /// @brief Default assingment operator overload.
    /// @param iterator Source iterator to assign from
    _rBuf_const_iterator& operator=(const _rBuf_const_iterator<_rBuf>& iterator) =default;

    /// @brief Custom assingment operator overload.
    /// @param ptr Raw source pointer to assign from.
    _rBuf_const_iterator& operator=(_rBuf* ptr) const
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

    /// @brief Dereference operator.
    /// @return Object pointed by iterator.
    reference operator*()
    {
        return *m_ptr;
    }

    //What container instance is this iterator for.
    const _rBuf* m_container;
    
    //Pointer to an element.
    const value_type* m_ptr;
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
    /// @param ptr Raw pointer to an element in container of type T.
    _rBuf_iterator(_rBuf* container = nullptr, pointer ptr = nullptr): m_container(container), m_ptr(ptr) {}

    /// @brief Const dereference operator
    /// @return  Const object pointed by iterator.
    reference operator*() const
    {
        //TODO: bounds checking and value-initialization.
        return *m_ptr;
    }

    /// @brief
    /// @return 
    pointer operator->() const
    {
        //TODO: check value initialization and bounds.
        return m_ptr;
    }

    /// @brief Prefix increment
    _rBuf_iterator& operator++()
    {
        //TODO: check value-initialization and over end() increment.
        ++m_ptr;
        return (*this);
    }

    /// @brief Postfix increment
    /// @param  int empty parameter to guide overload resolution.
    _rBuf_iterator operator++(int)
    {
        //TODO: check value-initialization and over end() increment.
        auto temp (*this);
        ++m_ptr;
        return temp; 
    }

    /// @brief prefix decrement
    _rBuf_iterator& operator--()
    {
        //TODO:    check value initialization and "under" begin() decrement
        --m_ptr;
        return(*this);
    }

    /// @brief Postfix decrement
    /// @param  int empty parameter to guide overload resolution.
    _rBuf_iterator operator--(int)
    {
        //TODO:    check value initialization and "under" begin() decrement
        auto temp (*this);
        --m_ptr;
        return temp;
    }

    //TODO
    // Part of THE (Tail Head Expansion).
    // void _Verify_offset(const difference_type offset) const
    // {
    //     ASSERT(offset == 0 || m_ptr, "cannot seek value-initialized ringbuffer iterator")
    //     if(offset < 0)
    //     {
    //         ASSERT(offset >= container.begin() - m_ptr, "cannot seek ringbuffer iterator before begin");
    //     }
    //     if(offset > 0)
    //     {
    //         ASSERT(offset <= container.end() - m_ptr, "cannot seek ringbuffer iterator after end")
    //     }
    // }

    /// @brief Moves iterator forward.
    /// @param movement Amount of elements to move.
    _rBuf_iterator& operator+=(difference_type offset)
    {
        m_ptr += offset;
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

    /// @brief Get iterator - offset.
    /// @param offset offset.
    /// @return iterator at this - offset.
    _rBuf_iterator operator-(const difference_type offset)
    {
        _rBuf_iterator temp = *this;
        return (temp - offset);
    }

    /// @brief Gets distance between two iterators.
    /// @param iterator Iterator to get distance to.
    /// @return Amount of elements between the iterators.
    difference_type operator-(const _rBuf_iterator& iterator)
    {
        //TODO: compatibility check
        return (m_ptr - iterator.m_ptr);
    }

    /// @brief index operator
    /// @param offset 
    /// @return 
    reference operator[](const difference_type index) const
    {
        return *(this->m_ptr + index);
    }

    /// @brief Comparison operator== overload
    /// @param other iterator to compare
    /// @return true if underlying pointers are the same.
    bool operator==(const _rBuf_iterator& other) const
    {
        return m_ptr == other.m_ptr;
    }

    /// @brief Comparison operator != overload
    /// @param other iterator to compare
    /// @return ture if underlying pointers are not the same
    bool operator!=(const _rBuf_iterator& other) const
    {
        return m_ptr != other.m_ptr;
    }

    /// @brief Comparison operator < overload
    /// @param other iterator to compare
    /// @return true if other is larger.
    bool operator<(const _rBuf_iterator& other) const
    {
        //TODO:compatibility check
        return (m_ptr < other.m_ptr);
    }

    /// @brief Comparison operator > overload
    /// @param other iterator to compare
    /// @return true if other is smaller.
    bool operator>(const _rBuf_iterator& other) const
    {
        //TODO:compatibility check
        return (other.m_ptr < m_ptr);
    }

    bool operator<=(const _rBuf_iterator& other) const
    {
        //TODO:compatibility check
        return: (!(other < m_ptr));
    }

    bool operator>=(const _rBuf_iterator& other) const
    {
        return (!(m_ptr < other.m_ptr));
    }

    //Implement _Compat? TODO

    /// @brief Default assingment operator overload.
    /// @param iterator Source iterator to assign from
    _rBuf_iterator& operator=(const _rBuf_iterator<_rBuf>& iterator) =default;

    /// @brief Custom assingment operator overload.
    /// @param ptr Raw source pointer to assign from.
    _rBuf_iterator& operator=(_rBuf* ptr) const
    {
        m_ptr = ptr;
        return (*this);
    };

    /// @brief Conversion operator, allows iterator to be converted to typename bool or "something convertable to bool".
    operator bool() const
    {
        if(m_ptr)
            return true;
        else
            return false;
    }

    /// @brief Dereference operator.
    /// @return Object pointed by iterator.
    reference operator*()
    {
        return *m_ptr;
    }

    //What container is this iterator for.
    _rBuf* m_container;
    
    //Pointer to an element.
    value_type* m_ptr;
};