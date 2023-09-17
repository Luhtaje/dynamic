#include <gtest/gtest.h>
#include "ring_buffer.hpp"
#include <utility>
#include <string>
#include <ctime>
#include <string>
#include <type_traits>
#include <vector>

// Set true to enable tests for private functions of the buffer. Also need to remove private identifier from RingBuffer code.
#define TEST_INTERNALS 0

namespace 
{

const static size_t TEST_BUFFER_SIZE = 6;
const static size_t TEST_INT_VALUE = 9;

// Define some factory functions.

//==========================
// Default buffer factories.
//==========================

template<class T>
ring_buffer<T> CreateBuffer();

template <>
ring_buffer<std::pair<int,std::string>> CreateBuffer<std::pair<int,std::string>>()
{
    return ring_buffer < std::pair<int, std::string>>{ {1, "Hello" }, { 2, "World" }, { 3, "I" }, { 4, "Love" }, { 5, "Mackerel"}, { 6, "wow" }};
}

template <>
ring_buffer<std::string> CreateBuffer<std::string>()
{
    return ring_buffer<std::string>{"abc", "def", "ghj", "cjk", "okm", "tyu"};
}

template <>
ring_buffer<char> CreateBuffer<char>()
{
    return ring_buffer<char>{'a','b','c','d','e','f'};
}

//=================================
// Random content buffer factories.
//=================================
template<class T>
ring_buffer<T> CreateBuffer(int /*size*/)
{
    return ring_buffer<T>(TEST_BUFFER_SIZE);
}

template <>
ring_buffer<char> CreateBuffer<char>(int /*size*/)
{
    srand(time(0));
    auto buf = ring_buffer<char>();
    for(size_t i = 0 ; i < TEST_BUFFER_SIZE; i++)
    {
        buf.push_back('a' + char(rand() % 26));
    }
    return buf;
}

template <>
ring_buffer<std::pair<int, std::string>> CreateBuffer<std::pair<int, std::string>>(int /*size*/)
{
    srand(time(0));
    auto buf = ring_buffer<std::pair<int,std::string>>();
    for(size_t i = 0 ; i < TEST_BUFFER_SIZE; i++)
    {
        std::pair<int, std::string> pair(rand() % 26, "hello world");
        buf.push_back(pair);
    }
    return buf;
}

template <>
ring_buffer<std::string> CreateBuffer<std::string>(int /*size*/)
{
    srand(time(0));
    auto buf = ring_buffer<std::string>();
    for(size_t i = 0 ; i < TEST_BUFFER_SIZE; i++)
    {
        std::string someString("");
        for(size_t j = 0 ; j < 3 ; j++)
        {
            someString.push_back(char(rand() % 100));
        }

    buf.push_back(someString);
    }
    return buf;
}

//====================================
// Individual random value generators.
//====================================

template <typename T>
T getValue();

template <>
std::pair<int, std::string> getValue<std::pair<int,std::string>>()
{
    srand(time(0));

    return std::pair<int, std::string>(rand() % 100, "value");
}

template <>
char getValue<char>()
{
    srand(time(0));
    return char(rand()% 26);
}

template <>
std::string getValue<std::string>()
{
    srand(time(0));
    std::string str("");
    for(int i = 0; i < 5; i++)
    {
        str.push_back('a' + char(rand() % 100));
    }
    return str;
}

//====================
// Fixture.
//====================
template<typename T>
class RingBufferTest : public testing::Test
{
public:
    ring_buffer<T> t_buffer;
protected:
    RingBufferTest() : t_buffer(CreateBuffer<T>()) {}
};

using Types = ::testing::Types<char, std::pair<int,std::string>, std::string>;
TYPED_TEST_SUITE(RingBufferTest, Types);

//==================Requirement tests ===================//
// 
// Tests requirement: Container requirement DefaultConstructible expression C u, C u{}, C(), C{}
TYPED_TEST(RingBufferTest, DefaultConstruction)
{
    ring_buffer<TypeParam> defaultInitialized;
    ASSERT_TRUE(defaultInitialized.empty());

    ring_buffer<TypeParam> defaultValueInitialized{};
    ASSERT_TRUE(defaultValueInitialized.empty());

    auto emptyInitialized = ring_buffer<TypeParam>();
    ASSERT_TRUE(emptyInitialized.empty());

    auto emptyAggregateInitialized = ring_buffer<TypeParam>{};
    ASSERT_TRUE(emptyAggregateInitialized.empty());

    ASSERT_TRUE(std::is_default_constructible<ring_buffer<TypeParam>>::value);
}

// Tests requirement: Container requirement CopyConstructible, expression C(a)
TYPED_TEST(RingBufferTest, copyConstruction)
{
    ring_buffer<TypeParam> copy(this->t_buffer);
    ASSERT_EQ(copy, this->t_buffer);
    ASSERT_TRUE(std::is_copy_constructible<ring_buffer<TypeParam>>::value);

}

// Tests requirement: Container expression C(rv)
TYPED_TEST(RingBufferTest, moveConstruction)
{
    ring_buffer<TypeParam> copy(this->t_buffer);
    ASSERT_EQ(this->t_buffer.empty(), false);

    ring_buffer<TypeParam> moved(std::move(this->t_buffer));

    ASSERT_EQ(moved, copy);
    ASSERT_EQ(this->t_buffer.size(), 0);

    ASSERT_TRUE(std::is_move_constructible<ring_buffer<TypeParam>>::value);
}

// Tests requirement: Container expression a = b
TYPED_TEST(RingBufferTest, copyAssignment)
{
    ring_buffer<TypeParam> control = this->t_buffer;
    ASSERT_EQ(control, this->t_buffer);
    ASSERT_TRUE(std::is_copy_assignable<ring_buffer<TypeParam>>::value);
}

// Tests requirement: Container expression a = rv
TYPED_TEST(RingBufferTest, moveAssign)
{
    const auto copy(this->t_buffer);
    const auto moved = std::move(this->t_buffer);

    ASSERT_EQ(copy, moved);
    ASSERT_EQ(this->t_buffer.size(), 0);

    ASSERT_TRUE(std::is_move_assignable<ring_buffer<TypeParam>>::value);
}

// Tests requirement: Container expression a == b, a != b.
TYPED_TEST(RingBufferTest, equalityComparable)
{
    ring_buffer<TypeParam> copy(this->t_buffer);
    ASSERT_TRUE(copy == this->t_buffer);

    copy.pop_back();
    ASSERT_TRUE(copy != this->t_buffer);

    this->t_buffer.pop_back();
    ASSERT_TRUE(copy == this->t_buffer);

    const auto randomBuffer(CreateBuffer<TypeParam>(TEST_BUFFER_SIZE));
    ASSERT_TRUE(randomBuffer != this->t_buffer);
}

// Tests requirement: SequenceContainer expression X a(n, t), X (n, t)
TYPED_TEST(RingBufferTest, sizeValConstruction)
{
    const auto value = getValue<TypeParam>();
    ring_buffer<TypeParam> sizeVal(TEST_BUFFER_SIZE, value);

    ASSERT_EQ(sizeVal.size(), TEST_BUFFER_SIZE);

    for (auto elem : sizeVal)
    {
        ASSERT_EQ(elem, value);
    }

    ASSERT_EQ(sizeVal.size(), TEST_BUFFER_SIZE);
    ASSERT_EQ(std::distance(sizeVal.begin(), sizeVal.end()), TEST_BUFFER_SIZE);

    const auto sizeRval = ring_buffer<TypeParam>(TEST_BUFFER_SIZE, value);

    ASSERT_EQ(sizeVal, sizeRval);
}

// Tests requirement: SequenceContainer expression X a(i, j), X (i,j)
TYPED_TEST(RingBufferTest, rangeConstruction)
{

    std::vector<TypeParam> testVector(this->t_buffer.begin(), this->t_buffer.end());
    ring_buffer<TypeParam> rangeConstructed(testVector.begin(), testVector.end());
    ASSERT_EQ(rangeConstructed, this->t_buffer);

    const auto ranged = ring_buffer<TypeParam>(testVector.begin(), testVector.end());
    ASSERT_EQ(ranged, this->t_buffer);
}

// Tests requirement: SequenceContainer expression X(il)
TYPED_TEST(RingBufferTest, initListConstruction)
{
    const auto firstElem = getValue<TypeParam>();
    const auto secondElem = getValue<TypeParam>();
    const auto ThirdElem = getValue<TypeParam>();
    ring_buffer<TypeParam> buf{firstElem, secondElem, ThirdElem};

    ASSERT_EQ(buf.size(), 3);
    ASSERT_EQ(buf[0], firstElem);
    ASSERT_EQ(buf[1], secondElem);
    ASSERT_EQ(buf[2], ThirdElem);
}

// Tests requirement: SequenceContainer, expression a = il
TYPED_TEST(RingBufferTest, assignInitListOperator)
{
    std::initializer_list<TypeParam> initList{ static_cast<TypeParam>(getValue<TypeParam>()), static_cast<TypeParam>(getValue<TypeParam>()), static_cast<TypeParam>(getValue<TypeParam>()) };

    this->t_buffer = initList;

    for (auto i = 0; i < initList.size(); i++)
    {
        ASSERT_EQ(this->t_buffer[i], *(initList.begin() + i));
    }
    ASSERT_EQ(this->t_buffer.size(), initList.size());
}

// Tests requirement: SequenceContainer, emplace() expression a.emplace(p, args) where p is position iterator.
TEST(NonTypedTest, emplace)
{
    ring_buffer<std::pair<int, std::string>> testBuffer;

    std::pair<int, std::string> fillerOne { 51, "hello" };
    std::pair<int, std::string> fillerTwo{ 53, "world" };

    testBuffer.push_back(fillerOne);
    testBuffer.push_back(fillerTwo);


    std::pair<int, std::string> emplaced{ 1, "I love Mackerel" };

    auto posIt = testBuffer.begin() + 1;
    testBuffer.emplace(posIt, emplaced.first, emplaced.second);

    ASSERT_EQ(*posIt, emplaced);
}


// Tests requirement: SequenceContainer, insert() expression a.insert(a,b) where a is a postion iterator and b is the value.
TYPED_TEST(RingBufferTest, insert)
{
    const auto beginIt = this->t_buffer.begin();
    const auto size = this->t_buffer.size();

    // Test the returned iterator and inserted value.
    const auto value = getValue<TypeParam>();
    auto pointIt = this->t_buffer.insert(beginIt + 1, value);

    ASSERT_EQ(*pointIt, value);
    ASSERT_EQ(this->t_buffer[1], value);

    // Tests the same things for the last element.
    const auto pointIt2 = this->t_buffer.insert(beginIt + (size), value);
    ASSERT_EQ(*pointIt2, value);
    ASSERT_EQ(this->t_buffer[size], value);
}

// Tests requiremet: SequenceContainer, insert() expression a.insert(a, rv) where a is a postion iterator and rv is an rvalue.
TYPED_TEST(RingBufferTest, insertRV)
{
    const auto it = this->t_buffer.begin();
    const auto size = this->t_buffer.size();

    // Test that returned iterator points to correct element and that the value is correct
    const auto value = getValue<TypeParam>();
    auto pointIt = this->t_buffer.insert(it + 1, std::move(value));

    ASSERT_EQ(*pointIt, value);
    ASSERT_EQ(this->t_buffer[1], value);
}

// Tests requirement: SequenceContainer, insert() exprssion a.insert(p, n, t) where p is position iterator, n is a size_type and t is a value of value type a::value_type.
TYPED_TEST(RingBufferTest, insertSizeVal)
{
    const auto amount = 3;
    const auto insertPosIndex = 2;
    const auto value = getValue<TypeParam>();
    const auto insertPosIt = this->t_buffer.begin() + insertPosIndex;

    const auto refBuffer(this->t_buffer);

    this->t_buffer.insert(insertPosIt, amount, value);

    // Check inserted elements are correct
    for (size_t i = 0; i < amount; i++)
    {
        ASSERT_EQ(this->t_buffer[i + insertPosIndex], value);
    }

    // Check buffer outside inserted elements remains untouched.
    for (size_t i = 0; i < refBuffer.size(); i++)
    {
        if (i < insertPosIndex)
        {
            ASSERT_EQ(this->t_buffer[i], refBuffer[i]);
        }
        else
        {
            ASSERT_EQ(this->t_buffer[i + amount], refBuffer[i]);
        }
    }
}

// Tests requirement: SequenceContainer, insert() exprssion a.insert(p, i, j) where p is position iterator and [i, j) is a valid range.
TYPED_TEST(RingBufferTest, insertRange)
{
    ring_buffer<TypeParam> rangeSource = CreateBuffer<TypeParam>(TEST_BUFFER_SIZE);
    const auto refBuffer(this->t_buffer);

    const auto pos = 1;
    const auto amount = 2;
    const auto beginOffset = 0;

    const auto rangeBeginIt = rangeSource.begin();
    const auto rangeEndIt = rangeBeginIt + amount;
    const auto posIt = this->t_buffer.begin() + pos;

    const auto returnIt = this->t_buffer.insert(posIt, rangeBeginIt, rangeEndIt);

    // Verify buffer state.
    for (size_t i = 0; i < refBuffer.size(); i++)
    {
        // Check beginning of buffer is unchanged
        if (i < pos)
        {
            ASSERT_EQ(this->t_buffer[i], refBuffer[i]);
        }
        // Check remainder after range insertion is unchanged
        else if ((amount + pos) < i)
        {
            ASSERT_EQ(this->t_buffer[i + amount], refBuffer[i]);
        }
    }

    for (size_t i = 0; i < amount; i++)
    {
        ASSERT_EQ(returnIt[i], posIt[i]);
        ASSERT_EQ(rangeSource[i], this->t_buffer[pos + i]);
    }
}

// Tests requirement: SequenceContainer insert() expression insert(p, il) where il is std::initializer_list<T>
TYPED_TEST(RingBufferTest, insertInitializerList)
{
    const auto pos = 3;
    const auto posIt = this->t_buffer.begin() + pos;
    const auto refBuffer(this->t_buffer);
    std::initializer_list<TypeParam> initList{ static_cast<TypeParam>(getValue<TypeParam>()), static_cast<TypeParam>(getValue<TypeParam>()),static_cast<TypeParam>(getValue<TypeParam>()),
                                                static_cast<TypeParam>(getValue<TypeParam>()), static_cast<TypeParam>(getValue<TypeParam>()), static_cast<TypeParam>(getValue<TypeParam>()) };

    this->t_buffer.insert(posIt, initList);

    //Check the elements outside inserted range is correct.
    for (size_t i = 0; i < refBuffer.size(); i++)
    {   // Before inserted elements.
        if (i < pos)
        {
            ASSERT_EQ(refBuffer[i], this->t_buffer[i]);
        }
        // After inserted elements.
        else
        {
            ASSERT_EQ(refBuffer[i], this->t_buffer[i + initList.size()]);
        }
    }

    // Check the inserted elements are correct.
    for (size_t i = 0; i < initList.size(); i++)
    {
        ASSERT_EQ(*(initList.begin() + i), this->t_buffer[pos + i]);
    }
}

// Tests requirement: SequenceContainer, erase() expression erase(q) where q is a valid dereferenceable const iterator into a.
TYPED_TEST(RingBufferTest, erase)
{
    const auto offset = 1;
    const auto erasePos = this->t_buffer.begin() + offset;
    const auto refBuffer(this->t_buffer);

    const auto erasedIt = this->t_buffer.erase(erasePos);

    ASSERT_EQ(*erasedIt, *(erasePos + 1));

    for (auto i = 0; i < this->t_buffer.size(); i++)
    {
        if(i < offset)
        {
            ASSERT_EQ(refBuffer[i], this->t_buffer[i]);
        }
        else
        {
            ASSERT_EQ(refBuffer[i + offset], this->t_buffer[i]);
        }
    }

    auto endBeforeErase = this->t_buffer.end();
    auto erasedEnd = this->t_buffer.erase(this->t_buffer.end() - 1);

    ASSERT_TRUE(endBeforeErase != erasedEnd);
    ASSERT_TRUE(erasedEnd == this->t_buffer.end());
}

// Tests requirement: SequenceContainer erase() expression erase(q1 , q2)
TYPED_TEST(RingBufferTest, eraseRange)
{
    auto rangeBegin = this->t_buffer.begin() + 2;
    auto rangeEnd = this->t_buffer.begin() + 4;
    const auto diff = rangeEnd - rangeBegin;
    auto refBuffer(this->t_buffer);
    
    auto erasedIt = this->t_buffer.erase(rangeBegin, rangeEnd);

    // Before erased elements
    for (auto it = this->t_buffer.begin(); it != rangeBegin; it++)
    {
        ASSERT_EQ(refBuffer[it.getIndex()], *it);
    }

    // After erased elements.
    for (; erasedIt != this->t_buffer.end(); erasedIt++)
    {
        ASSERT_EQ(refBuffer[erasedIt.getIndex() + diff], *erasedIt);
    }
}

// Tests requirement: SequenceContainer clear()
TYPED_TEST(RingBufferTest, clear)
{
    this->t_buffer.clear();
    ASSERT_EQ(this->t_buffer.size(), 0);
    ASSERT_TRUE(this->t_buffer.empty());

    // Just try some operations, to check that buffer is still valid.
    this->t_buffer.push_back(getValue<TypeParam>());
    this->t_buffer[0];
}

// Tests requirement: SequenceContainer assign(i, j) where i and j are a valid range.
TYPED_TEST(RingBufferTest, assignRange)
{
    ring_buffer<TypeParam> sourceBuffer(CreateBuffer<TypeParam>());
    const auto rangeSize = 4;
    const auto beginOffset = 1;
    const auto rangeBeginIt = sourceBuffer.begin() + beginOffset;
    const auto rangeEndIt = rangeBeginIt + rangeSize;

    this->t_buffer.assign(rangeBeginIt, rangeEndIt);

    for (size_t i = 0; i < rangeSize; i++)
    {
        ASSERT_EQ(this->t_buffer[i], sourceBuffer[i + beginOffset]);
    }

    this->t_buffer.assign(sourceBuffer.begin(), sourceBuffer.end());

    for (size_t i = 0; i < sourceBuffer.size(); i++)
    {
        ASSERT_EQ(this->t_buffer[i], sourceBuffer[i]);
    }
}

// Tests requirement: SequenceContainer, assign(initializerList)
TYPED_TEST(RingBufferTest, assignInitList)
{
    std::initializer_list<TypeParam> initList {static_cast<TypeParam>(getValue<TypeParam>()), static_cast<TypeParam>(getValue<TypeParam>()), static_cast<TypeParam>(getValue<TypeParam>())};

    this->t_buffer.assign(initList);

    for(size_t i = 0; i < initList.size(); i++)
    {
        ASSERT_EQ(this->t_buffer[i], *(initList.begin() + i));
    }
    ASSERT_EQ(this->t_buffer.size(), initList.size());
}

// Tests requirement SequenceContainer assign(n, t) where n is an integral value and t is value type.
TYPED_TEST(RingBufferTest, assignSizeVal)
{
    const auto value = getValue<TypeParam>();
    const auto size = 4;

    const auto refBuffer(this->t_buffer);

    this->t_buffer.assign(size, value);

    for(size_t i = 0; i < this->t_buffer.size(); i++)
    {
        ASSERT_EQ(this->t_buffer[i], value);
    }
    ASSERT_EQ(this->t_buffer.size(), size);
}

// Tests requirement: SequenceContainer a.front().
TYPED_TEST(RingBufferTest, front)
{
    ASSERT_EQ(this->t_buffer.front(), *(this->t_buffer.begin()));
}

// Tests requirement: SequenceContainer a.back().
TYPED_TEST(RingBufferTest, back)
{
    ASSERT_EQ(this->t_buffer.back(), *(this->t_buffer.end() - 1));
}

// Tests requirement: SequenceContainer a.emplace_front(args)
TEST(NonTypedTest, emplace_front)
{
    ring_buffer<std::pair<int, std::string>> testBuffer;

    std::pair<int, std::string> fillerOne{ 51, "hello" };
    std::pair<int, std::string> fillerTwo{ 53, "world" };

    testBuffer.push_back(fillerOne);
    testBuffer.push_back(fillerTwo);


    std::pair<int, std::string> emplaced{ 1, "I love Mackerel" };

    testBuffer.emplace_front(emplaced.first, emplaced.second);

    ASSERT_EQ(*testBuffer.begin(), emplaced);
    ASSERT_EQ(*(testBuffer.begin() + 1), fillerOne);
    ASSERT_EQ(*(testBuffer.begin() + 2), fillerTwo);
}

// Tests requirement: SequenceContainer a.emplace_back(args)
TEST(NonTypedTest, emplace_back)
{
    ring_buffer<std::pair<int, std::string>> testBuffer;

    std::pair<int, std::string> fillerOne{ 51, "hello" };
    std::pair<int, std::string> fillerTwo{ 53, "world" };

    testBuffer.push_back(fillerOne);
    testBuffer.push_back(fillerTwo);

    std::pair<int, std::string> emplaced{ 1, "I love Mackerel" };

    testBuffer.emplace_back(emplaced.first, emplaced.second);

    ASSERT_EQ(*testBuffer.begin(), fillerOne);
    ASSERT_EQ(*(testBuffer.begin() + 1), fillerTwo);
    ASSERT_EQ(*(testBuffer.begin() + 2), emplaced);
}

// Tests requirement: SequenceContainer a.push_front(t).
TYPED_TEST(RingBufferTest, push_front)
{
    const auto someVal = getValue<TypeParam>();
    this->t_buffer.push_front(someVal);
    ASSERT_EQ(this->t_buffer.front(), someVal);
}

// Tests requirement: SequenceContainer a.push_front(rv), where rv is a rvalue ref.
TYPED_TEST(RingBufferTest, pushFrontRV)
{
    std::shared_ptr<TypeParam> owningPtr;
    ring_buffer<std::shared_ptr<TypeParam>> ptrBuffer;

    // Push one element to exist in the empty buffer before actually testing the function.
    owningPtr = std::make_shared<TypeParam>(getValue<TypeParam>());
    ptrBuffer.push_front(std::move(owningPtr));

    // Create test pointer and a copy of the underlying value as a reference.
    owningPtr = std::make_shared<TypeParam>(getValue<TypeParam>());
    const auto oldFront = ptrBuffer.front();
    const auto newFrontReferenceCopy = owningPtr;
    ptrBuffer.push_front(std::move(owningPtr));

    // Tests that the value is actually at the front.
    ASSERT_EQ(ptrBuffer.front(), newFrontReferenceCopy);

    // Old value is second in the buffer.
    ASSERT_EQ(*(ptrBuffer.begin() + 1), oldFront);

    ASSERT_TRUE(owningPtr.get() == nullptr);
}

// Tests requirement: SequenceContainer a.push_back(t)
TYPED_TEST(RingBufferTest, pushBack)
{
    const auto someVal = getValue<TypeParam>();
    this->t_buffer.push_back(someVal);
    ASSERT_EQ(this->t_buffer.back(), someVal);
}

// Tests requirement: SequenceContainer a.push_back(rv)
TYPED_TEST(RingBufferTest, pushBackRV)
{
    const auto someVal = getValue<TypeParam>();
    const auto refVal = someVal;
    this->t_buffer.push_back(std::move(someVal));

    ASSERT_EQ(this->t_buffer.back(), refVal);
}

// Tests requirement: SequenceContainer, a.pop_front()
TYPED_TEST(RingBufferTest, pop_front)
{
    const auto newFront = *(this->t_buffer.begin() + 1);
    this->t_buffer.pop_front();

    ASSERT_NE(newFront, this->t_buffer.back());
}

// Tests requirement: SequenceContainer, a.pop_back()
TYPED_TEST(RingBufferTest, pop_back)
{
    const auto oldBack = this->t_buffer.back();
    const auto newBack = *(this->t_buffer.end() - 2);
    this->t_buffer.pop_back();

    ASSERT_NE(oldBack, this->t_buffer.back());
    ASSERT_EQ(newBack, this->t_buffer.back());
}

// Tests requirement: SequenceContainer, a[n]
TYPED_TEST(RingBufferTest, accessOperator)
{
    ASSERT_EQ(this->t_buffer[0], *(this->t_buffer.begin()));
    ASSERT_EQ(this->t_buffer[this->t_buffer.size() - 1], *(this->t_buffer.end() - 1));
    ASSERT_FALSE(std::is_const<std::remove_reference_t<decltype(this->t_buffer[0])>>::value);
    
    // Tests const version of the function.
    const auto constBuffer(this->t_buffer);

    ASSERT_EQ(constBuffer[0], *(constBuffer.begin()));
    ASSERT_EQ(constBuffer[constBuffer.size() - 1], *(constBuffer.end() - 1));
    ASSERT_TRUE(std::is_const<std::remove_reference_t<decltype(constBuffer[0])>>::value);
}

// Tests requirement: SequenceContainer, a.at(n)
TYPED_TEST(RingBufferTest, at)
{
    for (size_t i = 0; i < this->t_buffer.size(); i++)
    {
        ASSERT_EQ(this->t_buffer[i], this->t_buffer.at(i));
    }

    const auto c_buffer(this->t_buffer);

    for (size_t i = 0; i < this->t_buffer.size(); i++)
    {
        ASSERT_EQ(c_buffer[i], c_buffer.at(i));
    }

    // Test OB access.
    ASSERT_THROW(this->t_buffer.at(6), std::out_of_range);
    ASSERT_THROW(c_buffer.at(6), std::out_of_range);
}

// Tests requirement: Container, a.swap(b), swap(a, b).
TYPED_TEST(RingBufferTest, swap)
{
    using std::swap;
    auto control = CreateBuffer<TypeParam>(TEST_BUFFER_SIZE);
    auto temp(control);

    // Temp gets populated buffer.
    swap(temp, this->t_buffer);

    ASSERT_EQ(control, this->t_buffer);
    ASSERT_NE(control, temp);

    // Temp gets empty buffer.
    temp.swap(this->t_buffer);

    ASSERT_EQ(temp, control);

    // Check validity of t_buffer.
    ASSERT_FALSE(this->t_buffer.empty());
}

// Tests requirement: Container, a.size().
TYPED_TEST(RingBufferTest, size)
{
    ASSERT_EQ(this->t_buffer.size(), std::distance(this->t_buffer.cbegin(), this->t_buffer.cend()));

    ring_buffer<TypeParam> emptyBuf(0);
    ASSERT_EQ(emptyBuf.size(), 0);
}

// Tests requirement: Container, a.max_size().
TEST(RingBufferTest, MaxSize)
{
    ring_buffer<int> intBuffer;
    ring_buffer<std::pair<int, char>> pairBuffer;

    ASSERT_TRUE(intBuffer.max_size() > 0);

    ASSERT_TRUE(intBuffer.max_size() >= pairBuffer.max_size());

    ASSERT_TRUE(intBuffer.max_size() == ring_buffer<int>().max_size());
}

// Tests requirement: Container, a.empty()
TYPED_TEST(RingBufferTest, empty)
{
    ring_buffer<TypeParam> control;
    typename ring_buffer<TypeParam>::iterator begin = control.begin();
    typename ring_buffer<TypeParam>::iterator end = control.end();
    
    ASSERT_EQ(control.end(), control.begin());
    ASSERT_EQ(begin, end);

    ASSERT_TRUE(control.empty());
    ASSERT_FALSE(this->t_buffer.empty());
}

// Tests requirement: data() must rotate the elements so that the first logical element matches the first physical
// element in the memory. Additionally, when size is 0 and capacity is non-zero, data() should return a valid pointer.
TYPED_TEST(RingBufferTest, data)
{
    ring_buffer<TypeParam> myBuf;
    myBuf.reserve(5);
    ASSERT_TRUE((myBuf.size() == 0 && myBuf.capacity() > 0));
    ASSERT_TRUE(myBuf.data() != nullptr);

    // Modify buffer memory layout.
    this->t_buffer.pop_front();
    this->t_buffer.pop_front();
    const auto testVal = getValue<TypeParam>();
    this->t_buffer.push_front(testVal);

    // Sorts the buffer and checks for state.
    auto* initialAddress = &this->t_buffer[0];
    auto* dataPtr = this->t_buffer.data();
    ASSERT_NE(dataPtr, initialAddress);
    ASSERT_EQ(dataPtr, &this->t_buffer[0]);
}



// Tests requirement: Optional a.shrink_to_fit().
TYPED_TEST(RingBufferTest, shrink_to_fit)
{
    this->t_buffer.reserve(100);
    this->t_buffer.shrink_to_fit();
    // Shrink to fit reduces cap to size () + 1 (the allocation buffer)
    ASSERT_EQ(this->t_buffer.size() + 1, this->t_buffer.capacity());
}

TYPED_TEST(RingBufferTest, find)
{
    this->t_buffer.pop_front();
    auto val = getValue<TypeParam>():
    this->t_buffer.insert(this->t_buffer.begin() + 2, val);
    std:find(this->t_buffer.begin(), this->t_buffer.end(), val);
}
}
