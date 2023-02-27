#include <gtest/gtest.h>
#include "Ringbuffer.hpp"
#include <utility>
#include <string>
#include <ctime>
#include <string>
#include <type_traits>

namespace 
{

const static int TEST_SIZE = 10;
const static int TEST_INT_VALUE = 9;

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
    return RingBuffer<std::string>{"abc", "def", "ghj"};
}

template <>
RingBuffer<char> CreateBuffer<char>()
{
    return RingBuffer<char>{'a','b','c','d','e','f','g'};
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


//==================mainframe ===================//
TYPED_TEST(RingBufferTest, DefaultConstruction)
{
    RingBuffer<TypeParam> defaultInitialized;
    EXPECT_EQ(defaultInitialized.empty(), true);

    RingBuffer<TypeParam> defaultValueInitialized{};
    EXPECT_EQ(defaultValueInitialized.empty(), true);

    EXPECT_EQ(std::is_default_constructible<RingBuffer<TypeParam>>::value, true);
}

TYPED_TEST(RingBufferTest, CopyConstruction)
{
    RingBuffer<TypeParam> control(TEST_SIZE);
    RingBuffer<TypeParam> experiment(control);
    EXPECT_EQ(experiment.size(), control.size());
    EXPECT_EQ(experiment, control);
    EXPECT_EQ(std::is_copy_constructible<RingBuffer<TypeParam>>::value, true);
}

TYPED_TEST(RingBufferTest, MoveConstruction)
{
    RingBuffer<TypeParam> tempcopy(t_buffer);
    EXPECT_EQ(t_buffer.empty(), false);

    RingBuffer<TypeParam> moved(std::move(t_buffer));

    // Accessing moved memory gives weird error, expect_death did not work too well
    // EXPECT_DEATH(initial[0], "vector subscript is out of range");
    EXPECT_EQ(moved, tempcopy);
}

TYPED_TEST(RingBufferTest, MoveAssign)
{
    auto control(std::move(t_buffer));

    ASSERT_NE(control.empty(), true);
    //TODO investigate why vector subscript goes out of range. Control is empty, something in the comparison tries to access with some index. prob
    //EXPECT_NE(control,temp);
}

TYPED_TEST(RingBufferTest, SizeValConstruction)
{
    const auto value = getValue<TypeParam>();
    RingBuffer<TypeParam> sizeVal (TEST_SIZE, value);
    RingBuffer<TypeParam>::iterator it = sizeVal.begin();

    EXPECT_EQ(sizeVal.size(), TEST_SIZE);
    ++it;
    EXPECT_EQ(*it, value);
    EXPECT_EQ(*it, sizeVal[TEST_SIZE-1]);
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

TYPED_TEST(RingBufferTest, CopyAssignment)
{
    RingBuffer<TypeParam> control = t_buffer;
    ASSERT_EQ(control, t_buffer);
}

TYPED_TEST(RingBufferTest, EqualityComparators)
{
    RingBuffer<TypeParam> copy(t_buffer);
    EXPECT_TRUE(copy == t_buffer);

    RingBuffer<TypeParam> randomBuffer(TEST_SIZE);
    EXPECT_TRUE(randomBuffer != t_buffer);
}

TYPED_TEST(RingBufferTest, AccessOperator)
{
    t_buffer.pop_front();
    auto val(t_buffer[0]);
    t_buffer.pop_front();
    auto val2(t_buffer[0]);
}

TYPED_TEST(RingBufferTest, Swap)
{
    using std::swap;
    RingBuffer<TypeParam> control(TEST_SIZE);
    RingBuffer<TypeParam> experiment1(control);
    EXPECT_EQ(control, experiment1);

    swap(experiment1, t_buffer);

    EXPECT_EQ(control, t_buffer);
    EXPECT_NE(control, experiment1);
}

TYPED_TEST(RingBufferTest, Size)
{
    EXPECT_EQ(t_buffer.size(), std::distance(t_buffer.cbegin(), t_buffer.cend()));
}

TYPED_TEST(RingBufferTest, MaxSize)
{
    // TODO.
    // No immediate solution for testing max_size. How to get compiler / platform independent max size? MSVC uses some msvc exclusive "limits" header to get max size.
}

TYPED_TEST(RingBufferTest, Empty)
{
    RingBuffer<TypeParam> control;
    RingBuffer<TypeParam>::iterator begin = control.begin();
    RingBuffer<TypeParam>::iterator end = control.end();
    
    EXPECT_EQ(control.end(), control.begin());
    EXPECT_EQ(begin, end);

    EXPECT_TRUE(control.empty());
}

TYPED_TEST(RingBufferTest, data)
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

    RingBuffer<TypeParam> copy(t_buffer);
    // Sorts the buffer.
    t_buffer.data();
    ASSERT_EQ(copy, t_buffer);
    copy.data();
    ASSERT_EQ(copy, t_buffer);
}

TYPED_TEST(RingBufferTest, push_back)
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

TYPED_TEST(RingBufferTest, push_front)
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

TYPED_TEST(RingBufferTest, front)
{
    EXPECT_EQ(t_buffer.front(), *t_buffer.begin());
}

}