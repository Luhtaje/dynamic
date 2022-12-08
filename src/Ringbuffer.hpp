#ifndef MAIN_HPP
#define MAIN_HPP

#include <memory>
#include <vector>
#include <algorithm>

//Forward declarations
//============================================================================
template<typename _rBuf>
class _rBuf_iterator;

template<typename T, typename Allocator = std::allocator<T>>
class RingBuffer;
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
    _rBuf_const_iterator(pointer ptr = nullptr)
    {
        m_ptr = ptr;
    }

    /// @brief Conversion constructor
    /// @param const_iterator const iterator to construct from.
    _rBuf_const_iterator(_rBuf_iterator<_rBuf>& iterator)
    {
        m_ptr = iterator.m_ptr;
    }

    /// @brief Conversion assingment from non-const iterator
    /// @param iterator non-const iterator.
    /// @return Returns the new object by reference
    _rBuf_const_iterator& operator=(_rBuf_iterator<_rBuf>& iterator)
    {
        if(m_ptr == iterator.m_ptr)
        {
            return *this;
        }
        m_ptr = iterator.m_ptr;
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
        //TODO:compatibility check
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
    _rBuf_iterator(pointer ptr = nullptr)
    {
        m_ptr = ptr;
    }

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

    value_type* m_ptr;
};


//template<typename T>
//using RingBufferSharedPtr = shared_ptr<RingBuffer<value_type>>;

/// @brief Dynamic Ringbuffer is a dynamically growing std::container with support for queue, stack and priority queue adaptor functionality. 
/// @tparam T type of the ringbuffer
/// @tparam Allocator optional allocator for underlying vector. Defaults to std::allocator<T>
template<typename T, typename Allocator> 
class RingBuffer
{

public:
    using value_type = T;
    using allocator_type = Allocator;
    using reference = T&;
    using const_reference= const T&;
    using pointer = typename T*;
    using const_pointer= typename const T*;
    using const_iterator = _rBuf_const_iterator<RingBuffer<T>>;
    using iterator = _rBuf_iterator<RingBuffer<T>>;

    //TODO::are reverse iterators needed? Probably, hard to compete with stl container algorithms without reverse iterator.Right?
    //using reverse_iterator = reverse_iterator<iterator>;
    //using const_reverse_iterator = const_reverse_iterator<const_iterator>;

    using difference_type = ptrdiff_t;
    using size_type = std::size_t;

public:
    /// @brief Default constructor.
    RingBuffer() =default;

    /// @brief Default copy constructor.
    /// @param b Buffer to construct from.
    RingBuffer(const RingBuffer& b) =default;

    /// @brief Initializer list contructor.
    RingBuffer(std::initializer_list<T> init):m_data(init){};

    /// @brief Custom constructor.
    /// @param size Size of the buffer to initialize.
    /// @param val Value to set every element to.
    RingBuffer(size_type size, T val = 0)
    {
        m_data.resize(size);
        m_endIndex = size;
        for(std::vector<T>::iterator it = m_data.begin() ; m_data.end() != it; it++) {
            *it = val;
        }
    }

    /// @brief Default move constructor.
    /// @param  RingBuffer&&  Rval reference to a RingBuffer
    RingBuffer(RingBuffer&&) =default;

    /// @brief Default move assignment operator
    /// @param other Rval ref of RingBuffer
    /// @return RingBuffer
    RingBuffer& operator=(RingBuffer&& other) = default;

    /// @brief Default copy assignment operator
    /// @param other Lval ref of RingBuffer
    /// @return RingBuffer
    RingBuffer& operator=(const RingBuffer& other)= default;

    /// @brief Defaut destructor.
    ~RingBuffer()=default;

    /// @brief Contruct iterator at begin.
    /// @return Iterator pointing to first element.
    iterator begin() noexcept
    {
        return iterator(m_data.data());
    }

    /// @brief Construct const_iterator at begin.
    /// @return Const_iterator pointing to first element.
    const_iterator begin() const noexcept
    {
        return const_iterator(m_data.data());
    }

    /// @brief Construct iterator at end.
    /// @return Iterator pointing past last element.
    iterator end() noexcept
    {
        return iterator(m_data.data() + m_data.size());
    }

    /// @brief Construct const_iterator at end.
    /// @return Const_iterator pointing past last element.
    const_iterator end() const noexcept
    {
        return const_iterator(m_data.data() + m_data.size());
    }

    /// @brief Construct const_iterator at begin.
    /// @return Const_iterator pointing to first element.
    const_iterator cbegin() const noexcept
    {
        return const_iterator(m_data.data());
    }

    /// @brief Construct const_iterator.
    /// @return Const_iterator pointing past last element.
    const_iterator cend() const noexcept
    {
        return const_iterator(m_data.data() + m_data.size());
    }

    reference operator[](const size_type index)
    {
        return m_data[index];
    }

    const_reference operator[](const size_type index) const
    {
        return m_data[index];
    }

	/// @brief Sorts ringbuffer so that logical head matches the first element in physical memory.
    /// @return Pointer to the first element.
    //To be implemented! Requires knowledge of logical ends which are not implemented yet.
    RingBuffer data();

    /// @brief Gets the size of the data container.
    /// @return Size of buffer.
    size_type size() const
    {
        return std::distance(this->begin(), this->end());
    }

    size_type max_size() const noexcept
    {
        return m_data.max_size();
    }

    /// @brief Check if buffer is empty
    /// @return True if buffer is empty
    bool empty() const noexcept
    {
        return m_data.empty();
    }

    /// @brief Resizes the container so that it contains n elements.
    /// @param n Size to resize to.
    void resize(size_type n)
    {
        m_data.resize(n);
    }

    /// @brief Resize overload.
    /// @param n Size to resize to.
    /// @param val Value to initialize new elements to, if N is larger than current container capacity. Checked implicitly in the vector implementation.
    void resize(size_type n, const T& val)
    {
        m_data.resize(n,val);
    }

    //This bad boi is gonna need some modification. Ideal would be to return rBuf_iterator and not void. This would require explicit emplace implementation or 
    //learn to convert vector iterator to rbuf iterator. TODO
    void emplace_back(value_type val)
    {
        m_data.emplace_back(val);
    }

private:
    std::vector <T,Allocator> m_data;/*< Underlying vector to store the data in the buffer*/
    size_t m_beginIndex = 0; /*< Index to the first logical element in the buffer. Key part in Tail Head Expansion*/
    size_t m_endIndex = 0;/*< Index to the last logical element in the buffer. Key part in Tail Head Expansion*/
};

//TODO:
// Modify to non template friend functions.  https://www.drdobbs.com/the-standard-librarian-defining-iterato/184401331
// Modify equality operators to compare also logical order. Part of THE.

/// @brief Equality comparator (compare- operator = comparator)
/// @tparam T Value type
/// @tparam Alloc Optional custom allocator. Defaults to std::allocator<T>.
/// @param lhs Left hand side operand
/// @param rhs right hand side operand
/// @return returns true if the compared buffers have equal underlying data containers.
template<typename T , typename Alloc>
inline bool operator==(const RingBuffer<T,Alloc>& lhs, const RingBuffer<T,Alloc>& rhs)
{
    return (lhs.size() == rhs.size()) && std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

/// @brief Not-equal comparator
/// @tparam T Value type
/// @tparam Alloc Optional custom allocator. Defaults to std::allocator<T>.
/// @param lhs Left hand side operand.
/// @param rhs Right hand side operand.
/// @return returns true if the compared buffers dont have equal underlying data containers.
template<typename T,typename Alloc>
inline bool operator!=(const RingBuffer<T,Alloc>& lhs, const RingBuffer<T,Alloc>& rhs)
{
    return !(lhs == rhs);
}

#endif /*MAIN_HPP*/