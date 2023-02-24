#include <gtest/gtest.h>
#include "Ringbuffer.hpp"


// Reference buffer
RingBuffer<int> itControl {6,4,2,1,3,5};

//
/// @brief Tests the named requirements for LegacyRandomAccessIterator laid out by cppreference and thesis requirements if such apply.
//================Iterators=================//


//Tests requirement: LegacyForwardIterator / DefaultConstructible
TEST(Iterators, DefaultConstructible)
{
    RingBuffer<int>::iterator it;
    RingBuffer<int>::const_iterator cit;

    it.m_container = &itControl;
    ASSERT_TRUE(*it);

    cit.m_container = &itControl;
    ASSERT_TRUE(*cit);

    RingBuffer<int>::iterator value_it{};
    RingBuffer<int>::const_iterator value_cit{};

    it = RingBuffer<int>::iterator();
    cit = RingBuffer<int>::const_iterator();

    it.m_container = &itControl;
    ASSERT_TRUE(*it);

    cit.m_container = &itControl;
    ASSERT_TRUE(*cit);

    EXPECT_EQ(std::is_default_constructible<RingBuffer<int>::iterator>::value, true);
    EXPECT_EQ(std::is_default_constructible<RingBuffer<int>::const_iterator>::value, true);
}

// Tests requirement: LegacyInputIterator / EqualityComparable
TEST(Iterators, EqualityComparable)
{
    const auto begin(itControl.begin());
    const auto end(itControl.end());

    const auto refIt(begin);
    ASSERT_TRUE(refIt == begin);
    ASSERT_TRUE(begin == refIt);
    ASSERT_FALSE(begin == end);

    // Const iterators
    const auto cbegin(itControl.cbegin());
    const auto cend(itControl.cend());

    const auto crefIt(cbegin);
    ASSERT_TRUE(crefIt == cbegin);
    ASSERT_TRUE(cbegin == crefIt);
    ASSERT_FALSE(cbegin == cend);

    // Tests comparing non-const and const iterators both ways because of the const/non-const comparison semantics.
    ASSERT_TRUE(cbegin == begin);
    ASSERT_TRUE(begin == cbegin);
}

// Tests requirement: LegacyIterator / Swappable
TEST(Iterators, Swappable)
{
    //To satisfy the requirement, it is required to have std::swap in the context. Iterators dont have user defined swap, so *stud* swap is just used.
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

    ASSERT_TRUE(std::_Is_swappable<RingBuffer<int>::iterator>::value);
    ASSERT_TRUE(std::_Is_swappable<RingBuffer<int>::const_iterator>::value);
}

// Tests requirement: LegacyIterator / Destructible
TEST(Iterators, Destructible)
{
    ASSERT_TRUE(std::is_destructible<RingBuffer<int>::iterator>::value);
    ASSERT_TRUE(std::is_destructible<RingBuffer<int>::const_iterator>::value);
}

// Tests requirement: CopyAssignable / MoveAssignable
TEST(Iterators, MoveAssignable)
{
    const auto beginIt = itControl.begin();
    RingBuffer<int>::iterator moveAssigned;
    moveAssigned = std::move(beginIt);

    const auto cbeginIt = itControl.cbegin();
    RingBuffer<int>::const_iterator cmoveAssigned;
    cmoveAssigned = std::move(cbeginIt);

    ASSERT_EQ(*moveAssigned, *cmoveAssigned);
    ASSERT_EQ(moveAssigned, cmoveAssigned);

    ASSERT_TRUE(std::is_move_assignable<RingBuffer<int>::iterator>::value);
    ASSERT_TRUE(std::is_move_assignable<RingBuffer<int>::const_iterator>::value);
}

// Tests requirement: LegacyIterator / CopyAssignable
TEST(Iterators, CopyAssignable)
{
    const auto beginIt = itControl.begin();
    const auto otherIt = beginIt;
    ASSERT_EQ(beginIt, otherIt);

    const auto cbeginIt = itControl.cbegin();
    const auto cotherIt = beginIt;
    ASSERT_EQ(cbeginIt, cotherIt);

    EXPECT_EQ(std::is_copy_assignable<RingBuffer<int>::iterator>::value, true);
    EXPECT_EQ(std::is_copy_assignable<RingBuffer<int>::const_iterator>::value, true);
}

// Tests requirement: CopyConstructible / MoveConstucrible
TEST(Iterators, MoveConstructible)
{
    const auto it = itControl.begin();
    const auto movedIt(std::move(it));

    const auto cit = itControl.cbegin();
    const auto movedCit(std::move(cit));

    ASSERT_EQ(movedIt,movedCit);
    ASSERT_EQ(it,cit);

    EXPECT_TRUE(std::is_move_constructible<RingBuffer<int>::iterator>::value);
    EXPECT_TRUE(std::is_move_constructible<RingBuffer<int>::const_iterator>::value);
}

//Tests requirement: LegacyIterator / CopyConstructible.
TEST(Iterators, CopyConstructible)
{
    const auto it = itControl.begin();
    const auto copied(it);

    const auto cit = itControl.cbegin();
    const auto ccopied(cit);

    ASSERT_EQ(it, cit);
    ASSERT_EQ(it,copied);
    ASSERT_EQ(cit, ccopied);
    ASSERT_EQ(copied, ccopied);

    EXPECT_TRUE(std::is_copy_constructible<RingBuffer<int>::iterator>::value);
    EXPECT_TRUE(std::is_copy_constructible<RingBuffer<int>::const_iterator>::value);
}

// Tests requirement: LegacyInputIterator, Expression i!=j.
TEST(Iterators, Inequality)
{
    RingBuffer<int> someOtherBuffer{98,54,234,76};
    
    auto control(itControl.begin());
    const auto other(someOtherBuffer.begin());

    // Index is same, but iterators point to different containers.
    ASSERT_TRUE(control != other);
    
    //Same buffer, but different indexes.
    const auto snapshot(control);
    control++;
    ASSERT_TRUE(snapshot != control);

    // Same for const iterators
    auto constControl(itControl.cbegin());
    const auto constOther(someOtherBuffer.cbegin());

    ASSERT_TRUE(constControl != constOther);

    const auto constSnapshot(constControl);
    constControl++;
    ASSERT_TRUE(constControl != constSnapshot);

    // Tests comparing non-const and const. Constness should not change the result.
    // Only works one way, when const_iterator is on the left. This is the requirement by the standard, additional setup is optional but preferred.
    ASSERT_FALSE(constSnapshot != snapshot);
}

// Tests requirement: LegacyInputIterator, expression *i.
TEST(Iterators, dereferenceable)
{
    const auto it(itControl.begin());

    ASSERT_TRUE((std::is_same<decltype(*it), typename RingBuffer<int>::reference>::value));

    // Implicit conversion to value_type.
    using T = typename RingBuffer<int>::value_type;
    *it = 5;
    T convertibleToThis = *it;
    ASSERT_TRUE((std::is_same<decltype(convertibleToThis), typename RingBuffer<int>::value_type>::value));

    // Validate assignment to dereferenced iterator.
    ASSERT_EQ(convertibleToThis, 5);
}

//Tests requirement: LegacyInputIterator, Expression i->m is equivalent to (*i).m.
TEST(Iterators, pointerReduction)
{
    // Using a string here just to have some member to call.
    RingBuffer<std::string> strBuf{"abcd"};
    const auto customIt = strBuf.begin();
    EXPECT_EQ(customIt->at(0), (*customIt).at(0));

    const auto constIt = strBuf.cbegin();
    EXPECT_EQ(constIt->at(0), (*constIt).at(0));
}

//Tests requirement: LegacyInputIterator, Expressions : ++r, (void)r++ , *r++;
TEST(Iterators, IncrementOperators)
{
    auto begin = itControl.begin();
    auto cbegin = itControl.cbegin();

    const auto pre = ++begin;
    const auto post = begin++;
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

//Tests requirement: constant_iterator construction from non-const version.
TEST(Iterators, ConstantConversion)
{
    const auto it = itControl.begin();

    //Conversion constructor
    RingBuffer<int>::const_iterator cit(it);

    //Conversion assignment.
    auto anotherCit = itControl.cbegin();

    ASSERT_TRUE((std::is_same<decltype(anotherCit), typename RingBuffer<int>::const_iterator>::value));
    ASSERT_TRUE((std::is_same<decltype(cit), typename RingBuffer<int>::const_iterator>::value));
    ASSERT_EQ(*cit, *it);
}

// Tests requirement: LegacyBidirectionalIterator, Expressions --a, a--, *a--
TEST(Iterators, DecrementOperators)
{
    auto end(itControl.end());
    auto cend(itControl.cend());
    const auto size = itControl.size();

    const auto pre = --end;
    // Post increment return value is past the last element.
    ASSERT_EQ(pre, end);
    ASSERT_EQ(*pre, itControl[size - 1]);

    const auto post = end--;
    ASSERT_EQ(post, pre);
    ASSERT_NE(post, end);
    ASSERT_EQ(*end--, itControl[size - 2]);

    ASSERT_EQ(*end, itControl[size - 3]);

    const auto cpre = --cend;
    // Post increment return value is past the last element.
    ASSERT_EQ(cpre, cend);
    ASSERT_EQ(*cpre, itControl[size - 1]);

    const auto cpost = cend--;
    ASSERT_EQ(cpost, cpre);
    ASSERT_NE(cpost, cend);
    ASSERT_EQ(*cend--, itControl[size - 2]);

    ASSERT_EQ(*cend, itControl[size - 3]);
}