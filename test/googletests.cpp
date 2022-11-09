#include <gtest/gtest.h>
#include "RingBuffer.hpp"
#include <string>
#include <vector>
#include <string>
#include <iostream>
#include <type_traits>

const static int TEST_SIZE = 10;
const static int TEST_INT_VALUE = 9;

//================Iterators=================//
TEST(iterators, Construction)
{
    RingBuffer<int> control= {1,2,3,4,5,6};
    RingBuffer<int>::iterator bit = control.begin();
    RingBuffer<int>::const_iterator cbit = control.cbegin();
    ASSERT_EQ(*bit, 1);
    ASSERT_EQ(*cbit,1);
    ++bit;
    ++cbit;
    ASSERT_EQ(*bit, 2);
    ASSERT_EQ(*cbit,2);

    RingBuffer<int>::iterator eit = control.end();
    RingBuffer<int>::const_iterator ceit = control.cend();
    --ceit;
    --eit;
    ASSERT_EQ(*eit, 6);
}


TEST(iterators, ForwardOperators)
{
    RingBuffer<int> control{1,2,3,4,5,6};
    RingBuffer<int>::iterator it= control.begin();
    auto temp = it;
    it += 4;
    EXPECT_EQ(*it, temp[4]);
    EXPECT_NE(*it, *temp);
}

//TODO Increase test coverage for iterators. By alot.

//==================mainframe ===================//
TEST(mainframe, DefaultConstruction)
{
    RingBuffer<int> defaultInitialized;
    EXPECT_EQ(defaultInitialized.empty(), true);

    RingBuffer<int> defaultValueInitialized{};
    EXPECT_EQ(defaultValueInitialized.empty(), true);

    EXPECT_EQ(std::is_default_constructible<RingBuffer<int>>::value, true);
}

TEST(mainframe, CopyConstruction)
{
    RingBuffer<int> control(TEST_SIZE);
    RingBuffer<int> experiment(control);
    EXPECT_EQ(experiment.size(), control.size());
    EXPECT_EQ(experiment, control);
    EXPECT_EQ(std::is_copy_constructible<RingBuffer<int>>::value, true);
}

TEST(mainframe, MoveConstruction)
{
    RingBuffer<std::string> initial = {"myfirst","mysecond","mythird"};
    RingBuffer<std::string> tempcopy(initial);
    EXPECT_EQ(initial.empty(), false);
    RingBuffer<std::string> moved(std::move(initial));
    EXPECT_EQ(initial.empty(), true);
    EXPECT_EQ(moved, tempcopy);
}

TEST(mainframe, MoveAssign)
{
    RingBuffer<int> control(TEST_SIZE,TEST_INT_VALUE);
    RingBuffer<int> temp = control;
    RingBuffer<int> experiment;
    experiment = std::move(control);
    EXPECT_EQ(control.empty(), true);
    EXPECT_EQ(experiment.empty(), false);
    EXPECT_EQ(temp, experiment);

    //This fails without size checking in != operator (size check has been implemented now, no problem anymore). "Expected: (control) != (temp) Actual: {} vs {9,9,9,9,9,9,9,9,9,9}""
    //And that is interesting.
    EXPECT_NE(control,temp);
}

TEST(mainframe, SizeValConstruction)
{
    RingBuffer<int> sizeVal (TEST_SIZE, TEST_INT_VALUE);
    RingBuffer<int>::iterator it = sizeVal.begin();
    EXPECT_EQ(sizeVal.size(), TEST_SIZE);
    ++it;
   EXPECT_EQ(*it, TEST_INT_VALUE);
}

TEST(mainframe, InitListConstruction)
{
    RingBuffer<std::string> stringBuf = {"mystring", "othermystring"};
    EXPECT_EQ(stringBuf.size(), 2);
    EXPECT_EQ(stringBuf[0], "mystring");
}

TEST(mainframe, CopyAssignment)
{
    RingBuffer<int> control(TEST_SIZE);
    RingBuffer<int> thirdWheel(9,5);
    control = thirdWheel;
    EXPECT_EQ(control, thirdWheel);
}

TEST(mainframe, EqualityComparators)
{
    RingBuffer<int> control(TEST_SIZE, TEST_INT_VALUE);
    RingBuffer<int> experiment(5,99);
    EXPECT_NE(control, experiment);
    experiment = control;
    EXPECT_EQ(control, experiment);
}

TEST(mainframe, Swap)
{
    RingBuffer<int> control(TEST_SIZE, TEST_INT_VALUE);
    RingBuffer<int> experiment1(control);
    RingBuffer<int> experiment2(TEST_SIZE - 3, TEST_INT_VALUE - 4);
    EXPECT_EQ(control, experiment1);

    RingBuffer<int>::swap(experiment2,experiment1);

    EXPECT_EQ(control, experiment2);
    EXPECT_NE(control, experiment1);

    experiment1.swap(experiment2);

    EXPECT_EQ(control, experiment1);
    EXPECT_NE(control, experiment2);
}

TEST(mainframe, Resize)
{
    RingBuffer<int> control(TEST_SIZE);
    EXPECT_EQ(control.size(), TEST_SIZE);

    control.resize(TEST_SIZE - 4);
    EXPECT_EQ(control.size(), TEST_SIZE - 4);
}

TEST(mainframe, Size)
{
    RingBuffer<std::string> control1 = {"first", "second", "third", "fourth"};
    RingBuffer<int> control2(TEST_SIZE, TEST_INT_VALUE);

    EXPECT_EQ(control1.size(), std::distance(control1.begin(), control1.end()));
    EXPECT_EQ(control2.size(), std::distance(control2.begin(), control2.end()));
}

TEST(mainframe, MaxSize)
{
    // TODO.
    // No immediate solution for testing max_size. How to get compiler / platform independent max size? MSVC uses some msvc exclusive limits header to get max size.
}

TEST(mainframe, Empty)
{
    RingBuffer<int> control;
    RingBuffer<int>::iterator begin = control.begin();
    RingBuffer<int>::iterator end = control.end();
    EXPECT_EQ(control.end(), control.begin());
    EXPECT_EQ(begin, end);
}

TEST(mainframe, Emplace)
{
    RingBuffer<int> control(TEST_SIZE, TEST_INT_VALUE);
    control.emplace_back(TEST_INT_VALUE + 1);
    auto size = control.size();
    EXPECT_EQ(control[size-1], TEST_INT_VALUE + 1);
}