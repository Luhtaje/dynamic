#include <gtest/gtest.h>
#include "Ringbuffer.hpp"
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

//=================
// Default
//=================

template<class T>
RingBuffer<T> CreateBuffer();

template <>
RingBuffer<int> CreateBuffer<int>()
{
    return RingBuffer<int>{1,2,3,4,5,6};
}

template <>
RingBuffer<std::string> CreateBuffer<std::string>()
{
    return RingBuffer<std::string>{"abc", "def", "ghj", "cjk", "okm", "tyu"};
}

template <>
RingBuffer<char> CreateBuffer<char>()
{
    return RingBuffer<char>{'a','b','c','d','e','f'};
}

//====================
// By size
//====================
template<class T>
RingBuffer<T> CreateBuffer(int size)
{
    return RingBuffer<T>(size);
}

template <>
RingBuffer<char> CreateBuffer<char>(int size)
{
    srand(time(0));
    auto buf = RingBuffer<char>();
    for(int i = 0 ; i < size; i++)
    {
        buf.push_back(char(rand() % 26));
    }
    return buf;
}

template <>
RingBuffer<int> CreateBuffer<int>(int size)
{
    srand(time(0));
    auto buf = RingBuffer<int>();
    for(int i = 0 ; i < size; i++)
    {
        buf.push_back(rand() % 26);
    }
    return buf;
}

template <>
RingBuffer<std::string> CreateBuffer<std::string>(int size)
{
    srand(time(0));
    auto buf = RingBuffer<std::string>();
    for(int i = 0 ; i < size; i++)
    {
        std::string someString("");
        for(int j = 0 ; j < 3 ; j++)
        {
            someString.push_back(char(rand() % 100));
        }

    buf.push_back(someString);
    }
    return buf;
}

//===================
// Random value generators
//===================

template <typename T>
T getValue();

template <>
int getValue<int>()
{
    srand(time(0));
    return rand() % 100;
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
        str.push_back(char(rand() % 100));
    }
    return str;
}

//====================
// Fixture
//====================
template<typename T>
class RingBufferTest : public testing::Test
{
public:
    RingBuffer<T> t_buffer;
protected:
    RingBufferTest() : t_buffer(CreateBuffer<T>()) {}
};

using Types = ::testing::Types<char, int, std::string>;
TYPED_TEST_SUITE(RingBufferTest, Types);

//=======================================================================================================================================================================================
// Tests private functions of the buffer, requires the "private" identifier to be removed / commented in the ringbuffer code and TEST_INTERNALS to be set to 1 at the top of this file.
// NOTE! Using private functions like this leaves the buffer in unreliable state, and should only be used through the public API functions.
//=======================================================================================================================================================================================
#if TEST_INTERNALS
// Test internal shift function, used in insert and emplace operations. NOTE! Leaving the element uninitialized makes destructor try to destry uninitialized memory. After each shift initialize an element to the spot.

TYPED_TEST(RingBufferTest, increment)
{
    // Size() evaluates size by indexes.
    const auto size = t_buffer.size();

    t_buffer.increment(t_buffer.m_headIndex);

    for(int i = 0; i <= size; i++)
    {
        t_buffer.increment(t_buffer.m_headIndex);
    }
}

TYPED_TEST(RingBufferTest, shiftBegin)
{
    // Test borders
    auto beginIt = t_buffer.begin();
    auto firstVal = t_buffer[0];
    auto secondVal  = t_buffer[1];

    // Shift at begin by one, expect elements to be shifted by one.
    t_buffer.shift(beginIt,1);
    EXPECT_EQ(t_buffer[1], firstVal);
    EXPECT_EQ(t_buffer[2], secondVal);
    t_buffer.m_allocator.construct(&(*beginIt), getValue<TypeParam>());
}

TYPED_TEST(RingBufferTest, shiftMiddle)
{
    auto middleIt = t_buffer.begin() + 3;
    auto currentVal = *middleIt;
    auto nextVal = *(middleIt + 1);
    auto prevVal = *(middleIt - 1);

    // Shift at begin by one, expect elements to be shifted by one.
    t_buffer.shift(middleIt, 1);
    EXPECT_EQ(*(middleIt + 1), currentVal);
    EXPECT_EQ(*(middleIt - 1), prevVal);
    EXPECT_EQ(*(middleIt + 2), nextVal);

    t_buffer.m_allocator.construct(&(*middleIt), getValue<TypeParam>());
}

TYPED_TEST(RingBufferTest, shiftEnd)
{
    auto endIt = t_buffer.end();
    auto lastVal = *(endIt-1);
    auto secondToLastVal = *(endIt -2);

    t_buffer.shift(endIt, 1);

    EXPECT_EQ(*(endIt - 1), lastVal);
    EXPECT_EQ(*(endIt - 2), secondToLastVal);

    t_buffer.m_allocator.construct(&(*endIt), getValue<TypeParam>());
}

TYPED_TEST(RingBufferTest, copy)
{
    RingBuffer<TypeParam> copy(t_buffer.capacity());
    copy.m_headIndex = t_buffer.m_headIndex;
    copy.m_tailIndex = t_buffer.m_tailIndex;
    const auto begin = t_buffer.begin();
    const auto end = t_buffer.end();

    t_buffer.copy(begin, end, copy.begin());

    for(int i = 0 ; i < t_buffer.size(); i++)
    {
        ASSERT_EQ(t_buffer[i], copy[i]);
    }
}

#endif /*TEST_INTERNALS*/

//==================mainframe ===================//
TYPED_TEST(RingBufferTest, DefaultConstruction)
{
    RingBuffer<TypeParam> defaultInitialized;
    EXPECT_TRUE(defaultInitialized.empty());

    RingBuffer<TypeParam> defaultValueInitialized{};
    EXPECT_TRUE(defaultValueInitialized.empty());

    EXPECT_TRUE(std::is_default_constructible<RingBuffer<TypeParam>>::value);
}

TYPED_TEST(RingBufferTest, CopyConstruction)
{
    RingBuffer<TypeParam> copy(t_buffer);
    ASSERT_EQ(copy, t_buffer);
    ASSERT_TRUE(std::is_copy_constructible<RingBuffer<TypeParam>>::value);

}

TYPED_TEST(RingBufferTest, CopyAssignment)
{
    RingBuffer<TypeParam> control = t_buffer;
    ASSERT_EQ(control, t_buffer);
    ASSERT_TRUE(std::is_copy_assignable<RingBuffer<TypeParam>>::value);
}

TYPED_TEST(RingBufferTest, RangeConstruction)
{
    RingBuffer<TypeParam> rangeConstructed(t_buffer.begin(), t_buffer.end());
    EXPECT_EQ(rangeConstructed, t_buffer);

    const auto ranged = RingBuffer<TypeParam>(t_buffer.begin(), t_buffer.end());
    EXPECT_EQ(ranged, t_buffer);
}


TYPED_TEST(RingBufferTest, MoveConstruction)
{
    RingBuffer<TypeParam> copy(t_buffer);
    EXPECT_EQ(t_buffer.empty(), false);

    RingBuffer<TypeParam> moved(std::move(t_buffer));

    EXPECT_EQ(moved, copy);
    EXPECT_EQ(t_buffer.size(), 0);

    ASSERT_TRUE(std::is_move_constructible<RingBuffer<TypeParam>>::value);
}

TYPED_TEST(RingBufferTest, MoveAssign)
{
    const auto copy(t_buffer);
    const auto moved = std::move(t_buffer);

    ASSERT_EQ(copy, moved);
    EXPECT_EQ(t_buffer.size(), 0);

    ASSERT_TRUE(std::is_move_assignable<RingBuffer<TypeParam>>::value);
}

TYPED_TEST(RingBufferTest, SizeValConstruction)
{
    const auto value = getValue<TypeParam>();
    RingBuffer<TypeParam> sizeVal (TEST_BUFFER_SIZE, value);
    RingBuffer<TypeParam>::iterator it = sizeVal.begin();

    EXPECT_EQ(sizeVal.size(), TEST_BUFFER_SIZE);
    ++it;
    EXPECT_EQ(*it, value);
    EXPECT_EQ(*it, sizeVal[TEST_BUFFER_SIZE-1]);
}


TYPED_TEST(RingBufferTest, InitListConstruction)
{
    const auto firstElem = getValue<TypeParam>();
    const auto secondElem = getValue<TypeParam>();
    const auto ThirdElem = getValue<TypeParam>();
    RingBuffer<TypeParam> buf{firstElem, secondElem, ThirdElem};

    EXPECT_EQ(buf.size(), 3);
    EXPECT_EQ(buf[0], firstElem);
    EXPECT_EQ(buf[1], secondElem);
    EXPECT_EQ(buf[2], ThirdElem);
}

TYPED_TEST(RingBufferTest, EqualityComparable)
{
    RingBuffer<TypeParam> copy(t_buffer);
    EXPECT_TRUE(copy == t_buffer);

    copy.pop_back();
    EXPECT_TRUE(copy != t_buffer);

    t_buffer.pop_back();
    EXPECT_TRUE(copy == t_buffer);

    const auto randomBuffer(CreateBuffer<TypeParam>(TEST_BUFFER_SIZE));
    EXPECT_TRUE(randomBuffer != t_buffer);
}

TYPED_TEST(RingBufferTest, AccessOperator)
{
    const auto frontVal = getValue<TypeParam>();
    t_buffer.push_front(frontVal);
    ASSERT_EQ(frontVal, t_buffer[0]);

    const auto backVal = getValue<TypeParam>();
    t_buffer.push_back(backVal);
    ASSERT_EQ(backVal, t_buffer[t_buffer.size() -1]);
}

TYPED_TEST(RingBufferTest, Swap)
{
    using std::swap;
    auto control = CreateBuffer<TypeParam>(TEST_BUFFER_SIZE);
    auto experiment1(control);
    EXPECT_EQ(control, experiment1);

    swap(experiment1, t_buffer);

    EXPECT_EQ(control, t_buffer);
    EXPECT_NE(control, experiment1);
}

TYPED_TEST(RingBufferTest, Size)
{
    EXPECT_EQ(t_buffer.size(), std::distance(t_buffer.cbegin(), t_buffer.cend()));

    RingBuffer<TypeParam> emptyBuf(0);
    EXPECT_EQ(emptyBuf.size(), 0);
}

TYPED_TEST(RingBufferTest, MaxSize)
{
    // Yeah.
    EXPECT_NE(t_buffer.max_size(), 1);
}

TYPED_TEST(RingBufferTest, Empty)
{
    RingBuffer<TypeParam> control;
    RingBuffer<TypeParam>::iterator begin = control.begin();
    RingBuffer<TypeParam>::iterator end = control.end();
    
    EXPECT_EQ(control.end(), control.begin());
    EXPECT_EQ(begin, end);

    EXPECT_TRUE(control.empty());
    ASSERT_FALSE(t_buffer.empty());

}

TYPED_TEST(RingBufferTest, Data)
{
    RingBuffer<TypeParam> myBuf;
    myBuf.reserve(5);
    ASSERT_TRUE((myBuf.size() == 0 && myBuf.capacity() > 0));
    ASSERT_TRUE(myBuf.data() != nullptr);

    const auto firstAddress = &t_buffer[0];
    const auto secondAddress = &t_buffer[1];
    t_buffer.pop_front();
    t_buffer.pop_front();

    const auto testVal = getValue<TypeParam>();
    t_buffer.push_front(testVal);
    ASSERT_EQ(&t_buffer.front(), secondAddress);

    auto copy(t_buffer);
    // Sorts the buffer.
    t_buffer.data();
    ASSERT_EQ(copy, t_buffer);
    copy.data();
    ASSERT_EQ(copy, t_buffer);
}

TYPED_TEST(RingBufferTest, pushBack)
{
    // Push few times to make buffer allocate more memory.
    const auto someVal = getValue<TypeParam>();
    t_buffer.push_back(someVal);
    EXPECT_EQ(t_buffer.back(), someVal);

    const auto someVal2 = getValue<TypeParam>();
    t_buffer.push_back(someVal2);
    EXPECT_EQ(t_buffer.back(), someVal2);

    const auto someVal3 = getValue<TypeParam>();
    t_buffer.push_back(someVal3);
    EXPECT_EQ(t_buffer.back(), someVal3);

    const auto someVal4 = getValue<TypeParam>();
    t_buffer.push_back(someVal4);
    EXPECT_EQ(t_buffer.back(),someVal4);

    const auto someVal5 = getValue<TypeParam>();
    t_buffer.push_back(someVal5);
    EXPECT_EQ(t_buffer.back(), someVal5);

    const auto someVal6 = getValue<TypeParam>();
    t_buffer.push_back(someVal6);
    EXPECT_EQ(t_buffer.back(), someVal6);
}

TYPED_TEST(RingBufferTest, pushBackRV)
{
    auto refBuffer = CreateBuffer<TypeParam>();

    // Push rval ref to buffer, check against refValue.
    for(auto i = 0; i < TEST_BUFFER_SIZE ; i++)
    {
        const auto moveValue = getValue<TypeParam>();
        const auto refValue = moveValue;
        refBuffer.push_back(std::move(moveValue));
        ASSERT_EQ(refBuffer.back(), refValue);
    }
}

// Right now this test is pointless. It basicall tests decrement works. How to test if pop has actually removed an element?
TYPED_TEST(RingBufferTest, popBack)
{
    const auto refBuffer(t_buffer);
    t_buffer.pop_back();
    for(auto i = 0; i < t_buffer.size(); i++)
    {
        ASSERT_EQ(t_buffer[i], refBuffer[i]);
    }
 }


TYPED_TEST(RingBufferTest, pushFront)
{
    // Push few times to make buffer allocate more memory.
    const auto someVal = getValue<TypeParam>();
    t_buffer.push_front(someVal);
    EXPECT_EQ(t_buffer.front(), someVal);

    const auto someVal2 = getValue<TypeParam>();
    t_buffer.push_front(someVal2);
    EXPECT_EQ(t_buffer.front(), someVal2);

    const auto someVal3 = getValue<TypeParam>();
    t_buffer.push_front(someVal3);
    EXPECT_EQ(t_buffer.front(), someVal3);

    const auto someVal4 = getValue<TypeParam>();
    t_buffer.push_front(someVal4);
    EXPECT_EQ(t_buffer.front(), someVal4);

    const auto someVal5 = getValue<TypeParam>();
    t_buffer.push_front(someVal5);
    EXPECT_EQ(t_buffer.front(), someVal5);

    const auto someVal6 = getValue<TypeParam>();
    t_buffer.push_front(someVal6);
    EXPECT_EQ(t_buffer.front(), someVal6);
}

TYPED_TEST(RingBufferTest, pushFrontRV)
{
    auto refBuffer = CreateBuffer<TypeParam>();

    // Push rval ref to buffer, check against refValue.
    for(auto i = 0; i < TEST_BUFFER_SIZE ; i++)
    {
        const auto moveValue = getValue<TypeParam>();
        const auto refValue = moveValue;
        refBuffer.push_front(std::move(moveValue));
        ASSERT_EQ(refBuffer.front(), refValue);
    }
}

TYPED_TEST(RingBufferTest, Front)
{
    EXPECT_EQ(t_buffer.front(), *t_buffer.begin());
}

// Tests requiremet: SequenceContainer, Insert() expression a.insert(a,b) where a is a postion iterator and b is the value.
TYPED_TEST(RingBufferTest, Insert)
{
    const auto beginIt = t_buffer.begin();
    const auto size = t_buffer.size();

    // Test the returned iterator and inserted value.
    const auto value = getValue<TypeParam>();
    auto pointIt = t_buffer.insert(beginIt + 1, value);
    ASSERT_EQ(*pointIt, value);
    ASSERT_EQ(t_buffer[1], value);

    // Tests the same things for the last element.
    const auto pointIt2 = t_buffer.insert(beginIt + (size), value);
    ASSERT_EQ(*pointIt2, value);
    ASSERT_EQ(t_buffer[size], value);

}

// Tests requiremet: SequenceContainer, Insert() expression a.insert(a, rv) where a is a postion iterator and rv is an rvalue.
TYPED_TEST(RingBufferTest, InsertRV)
{
    const auto it = t_buffer.begin();
    const auto size = t_buffer.size();

    // Test that returned iterator points to correct element and that the value is correct
    const auto value = getValue<TypeParam>();
    auto pointIt = t_buffer.insert(it + 1, std::move(value));
    ASSERT_EQ(*pointIt, value);
    ASSERT_EQ(t_buffer[1], value);
}

// Tests requirement: SequeanceContainer, insert() exprssion a.insert(p, n, t) where p is position iterator, n is a size_type and t is a value of value type a::value_type.
TYPED_TEST(RingBufferTest, InsertSizeVal)
{
    const auto amount = 3;
    const auto insertPosIndex = 2;
    const auto value = getValue<TypeParam>();
    const auto insertPosIt = t_buffer.begin() + insertPosIndex;

    const auto refBuffer(t_buffer);

    t_buffer.insert(insertPosIt, amount, value);

    // Check inserted elements are correct
    for(auto i = 0; i < amount; i++)
    {
        ASSERT_EQ(t_buffer[i + insertPosIndex], value);
    }

    // Check buffer outside inserted elements remains untouched.
    for(auto i = 0; i < refBuffer.size(); i++)
    {
        if(i < insertPosIndex)
        {
            ASSERT_EQ(t_buffer[i], refBuffer[i]);
        }
        else
        {
            ASSERT_EQ(t_buffer[i + amount], refBuffer[i]);
        }
    }
}

// Tests requirement: SequeanceContainer, insert() exprssion a.insert(p, i, j) where p is position iterator and [i, j) is a valid range.
TYPED_TEST(RingBufferTest, insertRange)
{
    const auto beginOffset = 2;
    const auto endOffset = 5;
    const auto amount = endOffset - beginOffset;
    const auto pos = 3;

    RingBuffer<TypeParam> rangeSource = CreateBuffer<TypeParam>(TEST_BUFFER_SIZE);
    const auto refBuffer(t_buffer);

    const auto rangeBeginIt = rangeSource.begin() + beginOffset;
    const auto rangeEndIt = rangeSource.begin() + endOffset;
    const auto posIt = t_buffer.begin() + pos;

    const auto returnIt = t_buffer.insert(posIt, rangeBeginIt, rangeEndIt);

    // Verify buffer state.
    for(auto i = 0; i < refBuffer.size(); i ++)
    {
        // Check beginning of buffer is unchanged
        if(i < pos)
        {
            ASSERT_EQ(t_buffer[i], refBuffer[i]);
        }
        // Check remainder after range insertion is unchanged
        else if ((amount + pos) < i)
        {
            ASSERT_EQ(t_buffer[i + amount], refBuffer[i]);
        }
        // Check range is inserted correctly.
    }

    for(auto i = 0; i < amount ; i++)
    {
        ASSERT_EQ(returnIt[i], posIt[i]);
        ASSERT_EQ(rangeSource[beginOffset + i], t_buffer[pos + i]);
    }
}

TYPED_TEST(RingBufferTest, insertInitializerList)
{
    const auto pos = 3;
    const auto posIt = t_buffer.begin() + pos;
    const auto refBuffer(t_buffer);
    std::initializer_list<TypeParam> initList {static_cast<TypeParam>(getValue<TypeParam>()), static_cast<TypeParam>(getValue<TypeParam>()),static_cast<TypeParam>(getValue<TypeParam>()),
                                                static_cast<TypeParam>(getValue<TypeParam>()), static_cast<TypeParam>(getValue<TypeParam>()), static_cast<TypeParam>(getValue<TypeParam>())};

    t_buffer.insert(posIt, initList);

    for(auto i = 0; i < t_buffer.size(); i++)
    {
        if(i < pos || (pos + initList.size()) < i)
        {
            ASSERT_EQ(refBuffer[i], t_buffer[i]);
        }
        else
        {
            ASSERT_EQ(*(initList.begin() + (i - pos)), t_buffer[i]);
        }
    }
}

// Tests requirement: SequenceContainer, erase() expression erase(q) where q is a valid dereferenceable const iterator into a.
TYPED_TEST(RingBufferTest, erase)
{
    const auto offset = 1;
    const auto beginIt = t_buffer.cbegin() + offset;
    const auto refBuffer(t_buffer);

    const auto erasedIt =  t_buffer.erase(beginIt);
    ASSERT_EQ(erasedIt, beginIt);
    for(auto i = offset; i < t_buffer.size() - offset; i++)
    {
        ASSERT_EQ(refBuffer[i + 1], t_buffer[i]);
    }
}

// Tests a special case of erase(). Erasing at end iterator (past the last element) basically does nothing.
TYPED_TEST(RingBufferTest, eraseLast)
{
    const auto offset = 1;
    const auto endIt = t_buffer.cend();
    const auto refIt = t_buffer.cend() - offset;
    const auto refBuffer(t_buffer);
    const auto newLastValue = *refIt;

    const auto erasedIt = t_buffer.erase(endIt);

    ASSERT_EQ(erasedIt, refIt);
    ASSERT_EQ(erasedIt, t_buffer.end());
    ASSERT_EQ(newLastValue, *erasedIt);

    for(auto i = 0; i < t_buffer.size(); i++)
    {
        ASSERT_EQ(refBuffer[i], t_buffer[i]);
    }
}

TYPED_TEST(RingBufferTest, eraseRange)
{
    const auto beginIndex = 2;
    const auto endIndex = 4;
    const auto beginIt = t_buffer.begin() + beginIndex;
    const auto endIt = t_buffer.begin() + endIndex;
    const auto refBuffer(t_buffer);
    const auto refValue = *endIt;

    const auto erasedIt = t_buffer.erase(beginIt, endIt);
    ASSERT_EQ(refValue, *erasedIt);

    for(auto i = 0; i < beginIndex; i++)
    {
        ASSERT_EQ(refBuffer[i], t_buffer[i]);
    }

    for(auto i = endIndex; i < t_buffer.size(); i++)
    {
        ASSERT_EQ(refBuffer[i], t_buffer[i]);
    }
}

TYPED_TEST(RingBufferTest, clear)
{
    t_buffer.clear();
    ASSERT_EQ(t_buffer.size(), 0);

    // Just try some operations to make sure they don't throw exceptions.
    t_buffer.push_back(getValue<TypeParam>());
    t_buffer[0];
}

// Tests requirement: SequenceContainer, assign(i, j) where i and j are a valid range.
TYPED_TEST(RingBufferTest, assignRange)
{
    RingBuffer<TypeParam> sourceBuffer(CreateBuffer<TypeParam>());
    const auto rangeSize = 4;
    const auto beginOffset = 1;
    const auto rangeBeginIt = sourceBuffer.begin() + beginOffset;
    const auto rangeEndIt = rangeBeginIt + rangeSize;

    t_buffer.assign(rangeBeginIt, rangeEndIt);

    for(auto i = 0; i < rangeSize; i++)
    {
        ASSERT_EQ(t_buffer[i], sourceBuffer[i + beginOffset]);
    }

    t_buffer.assign(sourceBuffer.begin(), sourceBuffer.end());

    for(auto i = 0; i < sourceBuffer.size(); i++)
    {
        ASSERT_EQ(t_buffer[i], sourceBuffer[i]);
    }
}

// Tests requirement: SequenceContainer, assign(initializerList)
TYPED_TEST(RingBufferTest, assignInitList)
{
    std::initializer_list<TypeParam> initList {static_cast<TypeParam>(getValue<TypeParam>()), static_cast<TypeParam>(getValue<TypeParam>()), static_cast<TypeParam>(getValue<TypeParam>())};

    t_buffer.assign(initList);

    for(auto i = 0; i < initList.size(); i++)
    {
        ASSERT_EQ(t_buffer[i], *(initList.begin() + i));
    }
    ASSERT_EQ(t_buffer.size(), initList.size());
}

TYPED_TEST(RingBufferTest, assignSizeVal)
{
    const auto value = getValue<TypeParam>();
    const auto size = 4;

    const auto refBuffer(t_buffer);

    t_buffer.assign(size, value);

    for(auto i = 0; i < t_buffer.size(); i++)
    {
        ASSERT_EQ(t_buffer[i], value);
    }
    ASSERT_EQ(t_buffer.size(), size);
}

TYPED_TEST(RingBufferTest, at)
{
    for(auto i = 0; i < t_buffer.size(); i++)
    {
        ASSERT_EQ(t_buffer[i], t_buffer.at(i));
    }

    const auto const_buffer(t_buffer);
    for(auto i = 0; i < t_buffer.size(); i++)
    {
        ASSERT_EQ(const_buffer[i], const_buffer.at(i));
    }

    // Test OB access.
    EXPECT_THROW(t_buffer.at(6), std::out_of_range);
    EXPECT_THROW(const_buffer.at(6), std::out_of_range);
}

}
