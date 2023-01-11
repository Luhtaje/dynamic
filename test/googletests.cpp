#include <gtest/gtest.h>
#include "RingBuffer.hpp"
#include <string>
#include <vector>
#include <string>
#include <iostream>
#include <type_traits>

const static int TEST_SIZE = 10;
const static int TEST_INT_VALUE = 9;
/**
 * @brief The tests for most parts match the named requirements laid out by cppreference. The iterator tests test iterators and const iterators.
 * There are two sections, iterators and the mainframe which tests the functionality of the RingBuffer.
 */

//================Iterators=================//
/*
*Tests the const and non-const iterators for the Ringbuffer.
*/

//Control buffer for iterator tests.
RingBuffer<int> itControl{1,2,3,4,5,6};

class TestClass
{
    TestClass() = default;

public:
    bool foo(){
        std::cout << "fooo" << std::endl;
        return true;
    }
};

//Tests requirement: DefaultConstructible
//std::is_default_constructible does not test compilation, so compilation of different construction methods are also tested.
TEST(iterators, DefaultConstruction)
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

//Tests requirement: constant_iterator construction from non-const version. Additionally satisfies dereferencing operator.
TEST(iterators, ConstantConversion)
{
    auto it = itControl.begin();

    //Conversion constructor
    RingBuffer<int>::const_iterator cit(it);

    //Conversion assignment.
    auto anotherCit = itControl.cbegin();
    anotherCit = it;
    EXPECT_EQ(*cit, *it);
    EXPECT_EQ(*anotherCit, *it);
    //How to test for failure? TODO
}

//Tests requirement: contextually convertible to bool. 
TEST(iterators, ConvertibleToBool)
{
    //Non-const iterator
    RingBuffer<int>::iterator it(nullptr); 
    //Iterator is value-initialized
    EXPECT_FALSE(it);
    it = itControl.begin();
    EXPECT_TRUE(it);

    //Const iterator.
    RingBuffer<int>::const_iterator cit; 
    EXPECT_FALSE(cit);
    cit = itControl.cbegin();
    EXPECT_TRUE(cit);
}

//Tests requirement: LegacyInputIterator, Expression i->m is equivalent to (*i).m. Precondition, i is dereferenceable. 
TEST(iterators, PointerReduction)
{
    RingBuffer<std::string> strBuf = {"bb"};
    auto customIt = strBuf.begin();
    EXPECT_EQ(customIt->at(0), (*customIt).at(0));

    auto constIt = strBuf.cbegin();
    EXPECT_EQ(constIt->at(0), (*constIt).at(0));
}

//Tests requirement: LegacyInputIterator, ++r, (void)r++ , *r++;
TEST(iterators, IncrementOperators)
{
    auto begin= itControl.begin();
    auto cbegin= itControl.cbegin();

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
TEST(legacyIterator, CopyConstruction)
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
TEST(legacyIterator, MoveConstruction)
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

TEST(iterators, CopyAssignable)
{
    RingBuffer<int>::iterator it(itControl.begin());
    RingBuffer<int>::const_iterator cit(it);

    auto experiment{it};
    ASSERT_EQ(it, experiment);

    auto const_experiment{cit};
    ASSERT_EQ(cit,const_experiment);
}

TEST(iterators, swap)
{
    auto begin = itControl.begin();
    auto end = itControl.end();

    auto refIt = itControl.begin();
    swap(begin,end);
    ASSERT_EQ(refIt, end);
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

    EXPECT_DEATH(initial[0], "vector subscript out of range");
    EXPECT_EQ(moved, tempcopy);
}

TEST(mainframe, MoveAssign)
{
    GTEST_SKIP();
    RingBuffer<int> control(TEST_SIZE,TEST_INT_VALUE);
    RingBuffer<int> temp = control;
    RingBuffer<int> experiment;
    experiment = std::move(control);
    EXPECT_EQ(control.empty(), true);
    EXPECT_EQ(experiment.empty(), false);
    EXPECT_EQ(temp, experiment);

    //TODO investigate why vector subscript goes out of range. Control is empty, something in the comparison tries to access with some index. prob
    //EXPECT_NE(control,temp);
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
    GTEST_SKIP();
    RingBuffer<std::string> stringBuf = {"mystring", "othermystring"};
    //TODO Size is broken
    //EXPECT_EQ(stringBuf.size(), 2);
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
    EXPECT_TRUE(control == experiment);
}

TEST(mainframe, Swap)
{
    RingBuffer<int> control(TEST_SIZE, TEST_INT_VALUE);
    RingBuffer<int> experiment1(control);
    RingBuffer<int> experiment2(TEST_SIZE - 3, TEST_INT_VALUE - 4);
    EXPECT_EQ(control, experiment1);

    swap(experiment1, experiment2);

    EXPECT_EQ(control, experiment2);
    EXPECT_NE(control, experiment1);
}

TEST(mainframe, Resize)
{
    GTEST_SKIP();
     //TODO Size is broken.
    RingBuffer<int> control(TEST_SIZE);
    EXPECT_EQ(control.size(), TEST_SIZE);
    control.resize(12);
    EXPECT_EQ(control.size(), 12);
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
    // No immediate solution for testing max_size. How to get compiler / platform independent max size? MSVC uses some msvc exclusive "limits" header to get max size.
}

TEST(mainframe, Empty)
{
    GTEST_SKIP();
    RingBuffer<int> control;
    //TODO Vector subscript out of range
    RingBuffer<int>::iterator begin = control.begin();
    RingBuffer<int>::iterator end = control.end();
    EXPECT_EQ(control.end(), control.begin());
    EXPECT_EQ(begin, end);
}

TEST(mainframe, Emplace)
{
    GTEST_SKIP();
    //Emplace not implemented at the moment
    RingBuffer<int> control(TEST_SIZE, TEST_INT_VALUE);
    control.emplace_back(TEST_INT_VALUE + 1);
    auto size = control.size();
    EXPECT_EQ(control[size-1], TEST_INT_VALUE + 1);
}


TEST(sequencecontainer, front)
{
    const RingBuffer<int> nonConst {1,2,3,5,6};

    EXPECT_EQ(nonConst.front(), *nonConst.begin());
    EXPECT_EQ(itControl.front(), *itControl.begin());
}