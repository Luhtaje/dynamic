#include <gtest/gtest.h>
#include "Ringbuffer.hpp"
#include <utility>
#include <string>
#include <vector>
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

// Iterator tests dont use typed tests, hence they need a different buffer to work with. TODO make into a fixture and run the iterator tests with that.
RingBuffer<int> itControl {1,2,3,4,5,6};


//
 /// @brief Tests the named requirements laid out by cppreference and thesis requirements if such apply.
//================Iterators=================//


//Tests requirement: LegacyInputIterator, ++r, (void)r++ , *r++;
TEST(Iterators, IncrementOperators)
{
    auto begin = itControl.begin();
    auto cbegin = itControl.cbegin();

    //Copies value post increment
    auto pre = ++begin;
    //Copies value pre increment
    auto post = begin++;
    //Values should be the same
    ASSERT_EQ(pre, post);
    ASSERT_EQ(*post, itControl[1]);

    //Same for constant iterator
    auto cpre = ++cbegin;
    auto cpost = cbegin++;
    ASSERT_EQ(cpre, cpost);
    ASSERT_EQ(*cpost, itControl[1]);
    
    ASSERT_EQ(*begin, itControl[2]);
    ASSERT_EQ(*cbegin, itControl[2]);

    auto controlIt = begin;
    ++begin;
    ASSERT_NE(*controlIt++, *begin);
    ASSERT_EQ(*controlIt,*begin);
}

//Tests requirement: CopyConstructible.
TEST(Iterators, IterCopyConstruction)
{
    RingBuffer<int>::iterator initial(itControl.begin());

    auto constructed(initial);
    auto assignConstructed = (initial);
    ASSERT_EQ(initial, constructed);
    ASSERT_EQ(assignConstructed, constructed);


    RingBuffer<int>::const_iterator cinitial = itControl.cbegin();

    auto constConstructed(cinitial);
    auto assignConstConstructed = (cinitial);
    ASSERT_EQ(cinitial, constConstructed);
    ASSERT_EQ(assignConstConstructed, constConstructed); 
}

// Tests requirement: MoveConstucrible
TEST(Iterators, IterMoveConstruction)
{
    auto it = itControl.begin();
    it++;
    auto reference(it);
    auto movedIt(std::move(it));
    ASSERT_EQ(*movedIt, *reference);
    auto something = *it;

    EXPECT_EQ(std::is_move_constructible<RingBuffer<int>::iterator>::value, true);
    EXPECT_EQ(std::is_move_constructible<RingBuffer<int>::const_iterator>::value, true);
}

// Tests requirement: CopyAssignable
TEST(Iterators, CopyAssignable)
{
    EXPECT_EQ(std::is_copy_assignable<RingBuffer<int>::iterator>::value, true);
    EXPECT_EQ(std::is_copy_assignable<RingBuffer<int>::const_iterator>::value, true);
}

// Tests requirement: Destructible
TEST(Iterators, Destructible)
{
    EXPECT_EQ(std::is_destructible<RingBuffer<int>::iterator>::value, true);
    EXPECT_EQ(std::is_destructible<RingBuffer<int>::const_iterator>::value, true);
}

// Tests requirement: Swappable
TEST(Iterators, Swappable)
{
    //To satisfy the requirement, it is required to have std::swap in the context.
    using std::swap;

    auto begin = itControl.begin();
    auto end = itControl.end();

    // Rotates end iterator from end to begin, from begin to refIt.
    auto refIt = itControl.begin();
    swap(begin,end);
    ASSERT_EQ(refIt, end);
    swap(refIt, begin);
    ASSERT_EQ(end, begin);

    //Same but const.
    auto cbegin = itControl.cbegin();
    auto cend = itControl.cend();

    auto cRef = itControl.cbegin();
    swap(cbegin, cend);
    ASSERT_EQ(cRef, cend);
    swap(cRef, cbegin);
    ASSERT_EQ(cend, cbegin);

}

// Tests requirement: EqualityComparable
TEST(Iterators, EqualityComparable)
{
    auto begin(itControl.begin());
    auto end(itControl.end());
    // Requires implicit conversion to bool.
    auto refIt(begin);
    ASSERT_TRUE(refIt == begin);
    ASSERT_TRUE(begin == refIt);
    ASSERT_FALSE(begin == end);

    // Const iterators
    auto cbegin(itControl.cbegin());
    auto cend(itControl.cend());
    // Requires implicit conversion to bool.
    auto crefIt(cbegin);
    ASSERT_TRUE(crefIt == cbegin);
    ASSERT_TRUE(cbegin == crefIt);
    ASSERT_FALSE(cbegin == cend);

    // Tests comparing non-const and const iterators
    ASSERT_TRUE(begin == begin);
}

// Tests requirement: LegacyInputIterator, Expression i!=j.
TEST(Iterators, Inequality)
{
    RingBuffer<int> someOtherBuffer{98,54,234,76};
    
    auto control(itControl.begin());
    auto other(someOtherBuffer.begin());

    // Index is same, but iterators point to different containers.
    ASSERT_TRUE(control != other);
    
    //Same buffer, but different indexes.
    auto snapshot(control);
    control++;
    ASSERT_TRUE(snapshot != control);

    // Same for const iterators
    auto constControl(itControl.cbegin());
    auto constOther(someOtherBuffer.cbegin());

    ASSERT_TRUE(constControl != constOther);

    auto constSnapshot(constControl);
    constControl++;
    ASSERT_TRUE(constControl != constSnapshot);

    // Tests comparing non-const and const. Constness should not change the result.
    // Only works one way, when const_iterator is on the left. This is the requirement by the standard, additional setup is optional but preferred.
    ASSERT_FALSE(constSnapshot != snapshot);
}

// Tests requirement: LegacyInputIterator, expression *i.
TEST(Iterators, dereferencing)
{
    auto it(itControl.begin());
    
    ASSERT_TRUE((std::is_same<decltype(*it), typename RingBuffer<int>::reference>::value));

    // Implicit conversion to value_type.
    using T = typename RingBuffer<int>::value_type;
    *it = 5;
    T convertibleToThis = *it;
    ASSERT_TRUE((std::is_same<decltype(convertibleToThis), typename RingBuffer<int>::value_type>::value));

    // Validate assignment to dereferenced iterator.
    ASSERT_EQ(convertibleToThis, 5);
}


//Tests requirement: DefaultConstructible
//std::is_default_constructible does not test compilation, so compilation of different construction methods are also tested.
TEST(Iterators, IterDefaultConstruction)
{
    RingBuffer<int>::iterator it;
    RingBuffer<int>::const_iterator cit;

    RingBuffer<int>::iterator value_it{};
    RingBuffer<int>::const_iterator value_cit{};

    it = RingBuffer<int>::iterator();
    cit = RingBuffer<int>::const_iterator();

    EXPECT_EQ(std::is_default_constructible<RingBuffer<int>::iterator>::value, true);
    EXPECT_EQ(std::is_default_constructible<RingBuffer<int>::const_iterator>::value, true);
}

//Tests requirement: constant_iterator construction from non-const version.
TEST(Iterators, ConstantConversion)
{
    auto it = itControl.begin();

    //Conversion constructor
    RingBuffer<int>::const_iterator cit(it);

    //Conversion assignment.
    auto anotherCit = itControl.cbegin();

    ASSERT_TRUE((std::is_same<decltype(anotherCit), typename RingBuffer<int>::const_iterator>::value));
    ASSERT_TRUE((std::is_same<decltype(cit), typename RingBuffer<int>::const_iterator>::value));
    ASSERT_EQ(*cit, *it);

}


//Tests requirement: LegacyInputIterator, dereferenceable Expression i->m is equivalent to (*i).m.
TEST(Iterators, PointerReduction)
{
    RingBuffer<std::string> strBuf{"abcd"};
    auto customIt = strBuf.begin();
    EXPECT_EQ(customIt->at(0), (*customIt).at(0));

    auto constIt = strBuf.cbegin();
    EXPECT_EQ(constIt->at(0), (*constIt).at(0));
}

TEST(Iterators, IterCopyAssignable)
{
    RingBuffer<int>::iterator it(itControl.begin());
    RingBuffer<int>::const_iterator cit(it);

    auto experiment{it};
    ASSERT_EQ(it, experiment);

    auto const_experiment{cit};
    ASSERT_EQ(cit,const_experiment);
}

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

TYPED_TEST(RingBufferTest, Resize)
{
    GTEST_SKIP();
    RingBuffer<TypeParam> control(TEST_SIZE);
    EXPECT_EQ(control.size(), TEST_SIZE);
    control.resize(12);
    EXPECT_EQ(control.size(), 12);
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
    GTEST_SKIP();
    RingBuffer<TypeParam> control;
    //TODO This does not test the empty function
    RingBuffer<TypeParam>::iterator begin = control.begin();
    RingBuffer<TypeParam>::iterator end = control.end();
    EXPECT_EQ(control.end(), control.begin());
    EXPECT_EQ(begin, end);
}


TYPED_TEST(RingBufferTest, data)
{
    //Not implemented yet
    GTEST_SKIP();
    RingBuffer<TypeParam> myBuf;
    myBuf.reserve(5);
    // TODO add test for data to rotate the buffer so that physical start matches logical start.
    ASSERT_TRUE((myBuf.size()== 0 && myBuf.getCapacity() > 0));
    ASSERT_TRUE(myBuf.data() != nullptr);
}

// TEST(mainframe, push_back)
// {
//     RingBuffer<int> myBuf;

//     myBuf.push_back(TEST_INT_VALUE);
//     ASSERT_EQ(myBuf.back(), TEST_INT_VALUE);

//     myBuf.push_back(TEST_SIZE);
//     ASSERT_EQ(myBuf.back(), TEST_SIZE);

//     RingBufferTest::t_buffer.push_back(TEST_INT_VALUE);
//     ASSERT_EQ(RingBufferTest::t_buffer.back(), TEST_INT_VALUE);
// }

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