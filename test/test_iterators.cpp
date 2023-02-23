#include <gtest/gtest.h>
#include "Ringbuffer.hpp"


// Reference buffer
RingBuffer<int> itControl {6,4,2,1,3,5};

//
/// @brief Tests the named requirements for LegacyRandomAccessIterator laid out by cppreference and thesis requirements if such apply.
//================Iterators=================//


//Tests requirement: LegacyInputIterator, Expressions : ++r, (void)r++ , *r++;
TEST(Iterators, IncrementOperators)
{
    auto begin = itControl.begin();
    auto cbegin = itControl.cbegin();

    auto pre = ++begin;
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
    ASSERT_EQ(*controlIt, *begin);
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