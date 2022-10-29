#ifndef MAIN_HPP
#define MAIN_HPP

#include <memory>
#include <vector>
#include <algorithm>


/// @brief Custom iterator class.
/// @tparam T Type of the element what iterator points to.
template<class _rBuf>
class _rBuf_const_iterator
{

public:
    using iterator_category = std::random_access_iterator_tag;

    using value_type = typename _rBuf::value_type;
    using pointer = typename _rBuf::const_pointer;
    using reference = const value_type&;

public:
    /// @brief Constructor.
    /// @param ptr Raw pointer to an element in container of type T.
    _rBuf_const_iterator(_rBuf* ptr = nullptr){m_ptr = ptr};

    /// @brief default copy constructor.
    /// @param iterator
    _rBuf_const_iterator(const _rBuf_const_iterator<_rBuf>& iterator) = default;

    /// Destructor
    ~_rBuf_const_iterator(){};

    /// @brief Default assingment operator overload.
    /// @param iterator Source iterator to assign from
    _rBuf_const_iterator<_rBuf>& operator=(const _rBuf_const_iterator<_rBuf>& iterator) =default;

    /// @brief Custom assingment operator overload.
    /// @param ptr Raw source pointer to assign from.
    _rBuf_const_iterator<_rBuf>& operator=(_rBuf* ptr) const
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

    bool operator==(const _rBuf_const_iterator& other) const
    {
        return m_ptr == other.m_ptr;
    }

    bool operator!=(const _rBuf_const_iterator& other) const
    {
        return m_ptr != other.m_ptr;
    }

    /// @brief Moves iterator forward.
    /// @param movement Amount of elements to move.
    _rBuf_const_iterator<_rBuf>& operator+=(const ptrdiff_t& movement)
    {
        m_ptr += movement
        return (*this);    
    }

    /// @brief Moves iterator backwards.
    /// @param movement Amount of elements to move.
    _rBuf_const_iterator<_rBuf>& operator-=(const ptrdiff_t&  movement)
    {
        m_ptr -= movement;
        return (*this);
    }

    /// @brief Move iterator forward by ony element.
    _rBuf_const_iterator<_rBuf>& operator++()
    {
        ++m_ptr;
        return (*this);
    }

    /// @brief Move iterator backward by one element.
    _rBuf_const_iterator<_rBuf>& operator--()
    {
        --m_ptr;
        return(*this);
    }

    /// @brief Move iterator forward by one element.
    /// @param  int empty parameter to guide overload resolution.
    _rBuf_const_iterator<_rBuf> operator++(int)
    {
        auto temp (*this);
        ++m_ptr;
        return temp; 
    }

    /// @brief Move iterator backwards by one element.
    /// @param  int empty parameter to guide overload resolution.
    _rBuf_const_iterator<_rBuf> operator--(int)
    {
        auto temp (*this);
        --m_ptr;
        return temp;
    }

    /// @brief Move iterator forward by specified amount.
    /// @param movement Amount of elements to move the iterator.
    _rBuf_const_iterator<_rBuf> operator+(const int& movement)
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
    ptrdiff_t operator-(const _rBuf_const_iterator<_rBuf>& iterator)
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
    const _rBuf* m_ptr;
};


/// @brief Custom iterator class.
/// @tparam T Type of the element what iterator points to.
template<class _rBuf>
class _rBuf_iterator
{

public:
    using iterator_category = std::random_access_iterator_tag;
    using base = _rBuf_const_iterator<_rBuf>;

    using value_type = typename _rBuf::value_type;
    using pointer = typename _rBuf::const_pointer;
    using reference = const value_type&;

public:
    /// @brief Constructor.
    /// @param ptr Raw pointer to an element in container of type T.
    _rBuf_iterator(_rBuf* ptr = nullptr){m_ptr = ptr};

    /// @brief default copy constructor.
    /// @param iterator
    _rBuf_iterator(const _rBuf_iterator<_rBuf>& iterator) = default;

    /// Destructor
    ~_rBuf_iterator(){};

    /// @brief Default assingment operator overload.
    /// @param iterator Source iterator to assign from
    _rBuf_iterator<_rBuf>& operator=(const _rBuf_iterator<_rBuf>& iterator) =default;

    /// @brief Custom assingment operator overload.
    /// @param ptr Raw source pointer to assign from.
    _rBuf_iterator<_rBuf>& operator=(_rBuf* ptr)
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

    bool operator==(const _rBuf_iterator& other) const
    {
        return m_ptr == other.m_ptr;
    }

    bool operator!=(const _rBuf_iterator& other) const
    {
        return m_ptr != other.m_ptr;
    }

    /// @brief Moves iterator forward.
    /// @param movement Amount of elements to move.
    _rBuf_iterator<_rBuf>& operator+=(const ptrdiff_t& movement)
    {
        m_ptr += movement
        return (*this);    
    }

    /// @brief Moves iterator backwards.
    /// @param movement Amount of elements to move.
    _rBuf_iterator<_rBuf>& operator-=(const ptrdiff_t&  movement)
    {
        m_ptr -= movement;
        return (*this);
    }

    /// @brief Move iterator forward by ony element.
    _rBuf_iterator<_rBuf>& operator++()
    {
        ++m_ptr;
        return (*this);
    }

    /// @brief Move iterator backward by one element.
    _rBuf_iterator<_rBuf>& operator--()
    {
        --m_ptr;
        return(*this);
    }

    /// @brief Move iterator forward by one element.
    /// @param  int empty parameter to guide overload resolution.
    _rBuf_iterator<_rBuf> operator++(int)
    {
        auto temp (*this);
        ++m_ptr;
        return temp; 
    }

    /// @brief Move iterator backwards by one element.
    /// @param  int empty parameter to guide overload resolution.
    _rBuf_iterator<_rBuf> operator--(int)
    {
        auto temp (*this);
        --m_ptr;
        return temp;
    }

    /// @brief Move iterator forward by specified amount.
    /// @param movement Amount of elements to move the iterator.
    _rBuf_iterator<_rBuf> operator+(const int& movement)
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
    ptrdiff_t operator-(const _rBuf_iterator<_rBuf>& iterator)
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
    _rBuf* m_ptr;
};


//template<typename T>
//using RingBufferSharedPtr = shared_ptr<RingBuffer<T, Allocator>>;

/// @brief Dynamic Ringbuffer is a dynamically growing std::container with support for queue, stack and priority queue adaptor functionality. 
/// @tparam T type of the ringbuffer
/// @tparam Allocator optional allocator for underlying vector. Defaults to std::allocator<T>
template<typename T, typename Allocator = std::allocator<T>> 
class RingBuffer
{

public:
    using value_type = T;
    using reference = T&;
    using const_reference= const T&;
    using difference_type = ptrdiff_t;
    using size_type = std::size_t;

    using const_iterator = _rBuf_const_iterator<RingBuffer<T>>;
    //using iterator = _rBuf_iterator<RingBuffer<T>>;

public:
    /// @brief Default constructor.
    RingBuffer() =default;

    /// @brief Default copy constructor.
    /// @param b Buffer to construct from.
    RingBuffer(const RingBuffer& b){
        m_data = b.m_data;
        m_beginIndex = b.m_beginIndex;
        m_endIndex = b.m_endIndex;
    }

    /// @brief Custom constructor.
    /// @param size Size of the buffer to initialize.
    /// @param val Value to set every element to.
    RingBuffer(size_type size, T val = 0)
    {
        m_data.resize(size);

        for(auto elem : m_data){
            elem = val;
        }
    }

    /// @brief Default move constructor.
    /// @param  RingBuffer&&  Rval reference to a RingBuffer
    //RingBuffer(RingBuffer&&)=default;

	

    ~RingBuffer()=default;

	/// @brief Sorts ringbuffer so that logical head matches the first element in physical memory.
    /// @return Pointer to the first element.
    RingBuffer data();

    /// @brief Gets the size of the underlying container.
    /// @return Size of buffer.
    size_type size(){
        return m_data.size();
    }

    /// @brief Check if buffer is empty
    /// @return True if buffer is empty
    bool empty(){
        return m_data.empty();
    }

    /// @brief Resizes the container so that it containes n elements.
    /// @param n Size to resize to.
    void resize(size_type n){
        m_data.resize(n);
    }

    /// @brief Resize overload.
    /// @param n Size to resize to.
    /// @param val Value to initialize new elements to, if N is larger than current container capacity.
    void resize(size_type n, const T& val){
        m_data.resize(n,val);
    }

private:

    std::vector <T,Allocator> m_data;/*< Underlying vector to store the data in the buffer*/
    size_t m_beginIndex; /*< Index to the first element in the buffer*/
    size_t m_endIndex;/*< Index to the last element in the buffer*/

};

#endif /*MAIN_HPP*/