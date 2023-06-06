#include <gtest/gtest.h>
#include "ring_buffer.hpp"


// Reference buffer
ring_buffer<int> itControl {6,4,2,1,3,5};

//
/// @brief Tests the named requirements for LegacyRandomAccessIterator laid out by cppreference and thesis requirements if such apply.
//================Iterators=================//


//Tests requirement: LegacyForwardIterator requirement DefaultConstructible
TEST(Iterators, DefaultConstructible)
{
    // After making iterator internals private, need to figure out something else for test.
    ring_buffer<int>::iterator it;
    ring_buffer<int>::const_iterator cit;

    ASSERT_TRUE(it.getIndex() == cit.getIndex());

    ring_buffer<int>::iterator value_it{};
    ring_buffer<int>::const_iterator value_cit{};

    ASSERT_TRUE(value_it.getIndex() == value_cit.getIndex());

    it = ring_buffer<int>::iterator();
    cit = ring_buffer<int>::const_iterator();

    ASSERT_TRUE(it.getIndex() == cit.getIndex());

    EXPECT_EQ(std::is_default_constructible<ring_buffer<int>::iterator>::value, true);
    EXPECT_EQ(std::is_default_constructible<ring_buffer<int>::const_iterator>::value, true);
}

// Tests requirement: LegacyInputIterator requirement EqualityComparable
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

    ASSERT_TRUE(std::_Is_swappable<ring_buffer<int>::iterator>::value);
    ASSERT_TRUE(std::_Is_swappable<ring_buffer<int>::const_iterator>::value);
}

// Tests requirement: LegacyIterator / Destructible
TEST(Iterators, Destructible)
{
    ASSERT_TRUE(std::is_destructible<ring_buffer<int>::iterator>::value);
    ASSERT_TRUE(std::is_destructible<ring_buffer<int>::const_iterator>::value);
}

// Tests requirement: CopyAssignable / MoveAssignable
TEST(Iterators, MoveAssignable)
{
    auto beginIt = itControl.begin();
    ring_buffer<int>::iterator moveAssigned;
    moveAssigned = std::move(beginIt);

    auto cbeginIt = itControl.cbegin();
    ring_buffer<int>::const_iterator cmoveAssigned;
    cmoveAssigned = std::move(cbeginIt);

    ASSERT_EQ(*moveAssigned, *cmoveAssigned);
    ASSERT_EQ(moveAssigned, cmoveAssigned);

    ASSERT_TRUE(std::is_move_assignable<ring_buffer<int>::iterator>::value);
    ASSERT_TRUE(std::is_move_assignable<ring_buffer<int>::const_iterator>::value);
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

    EXPECT_EQ(std::is_copy_assignable<ring_buffer<int>::iterator>::value, true);
    EXPECT_EQ(std::is_copy_assignable<ring_buffer<int>::const_iterator>::value, true);
}

// Tests requirement: CopyConstructible / MoveConstucrible
TEST(Iterators, MoveConstructible)
{
    auto it = itControl.begin();
    const auto movedIt(std::move(it));

    const auto cit = itControl.cbegin();
    const auto movedCit(std::move(cit));

    ASSERT_EQ(movedIt,movedCit);
    ASSERT_EQ(it,cit);

    EXPECT_TRUE(std::is_move_constructible<ring_buffer<int>::iterator>::value);
    EXPECT_TRUE(std::is_move_constructible<ring_buffer<int>::const_iterator>::value);
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

    EXPECT_TRUE(std::is_copy_constructible<ring_buffer<int>::iterator>::value);
    EXPECT_TRUE(std::is_copy_constructible<ring_buffer<int>::const_iterator>::value);
}

// Tests requirement: LegacyInputIterator, Expression i!=j.
TEST(Iterators, Inequality)
{
    ring_buffer<int> someOtherBuffer{98,54,234,76};
    
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
TEST(Iterators, Dereferenceable)
{
    const auto it(itControl.begin());

    ASSERT_TRUE((std::is_same<decltype(*it), typename ring_buffer<int>::reference>::value));

    // Implicit conversion to value_type.
    using T = typename ring_buffer<int>::value_type;
    *it = 5;
    T convertibleToThis = *it;
    ASSERT_TRUE((std::is_same<decltype(convertibleToThis), typename ring_buffer<int>::value_type>::value));

    // Validate assignment to dereferenced iterator.
    ASSERT_EQ(convertibleToThis, 5);
}

//Tests requirement: LegacyInputIterator, Expression i->m is equivalent to (*i).m.
TEST(Iterators, PointerReduction)
{
    // Using a string here just to have some member to call.
    ring_buffer<std::string> strBuf{"abcd"};
    const auto customIt = strBuf.begin();
    EXPECT_EQ(customIt->at(0), (*customIt).at(0));

    const auto constIt = strBuf.cbegin();
    EXPECT_EQ(constIt->at(0), (*constIt).at(0));
}

//Tests requirement: constant_iterator construction from non-const version.
TEST(Iterators, ConstantConversion)
{
    const auto it = itControl.begin();

    //Conversion constructor
    ring_buffer<int>::const_iterator cit(it);

    //Conversion assignment.
    auto anotherCit = itControl.cbegin();

    ASSERT_TRUE((std::is_same<decltype(anotherCit), typename ring_buffer<int>::const_iterator>::value));
    ASSERT_TRUE((std::is_same<decltype(cit), typename ring_buffer<int>::const_iterator>::value));
    ASSERT_EQ(*cit, *it);
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

// Tests requirement: LegacyBidirectionalIterator, Expressions --a, (void)a--, *a--
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

// Tests requirement: LegacyRandomAccessIterator, expressions r += n, a + n / n + a. n can be negative.
TEST(Iterators, Addition)
{
    const auto capacity = itControl.capacity();
    const auto size = itControl.size();

    auto it = itControl.begin();
    auto end = itControl.end();

    it += 1;
    ASSERT_EQ(*it, itControl[1]);

    it += -1;
    ASSERT_EQ(*it, itControl[0]);
    // Test that operator works correctly when the index loops around a buffer boundary. Moving iterator by capacity loops it to same element.
    it += capacity * -1;
    ASSERT_EQ(*it, itControl[0]);
    it += capacity;
    ASSERT_EQ(*it, itControl[0]);

    // a + n returns temporary iterator.
    ASSERT_EQ(*(it + 1), itControl[1]);
    ASSERT_EQ(*(1 + it), itControl[1]);
    ASSERT_EQ(*(end + (-1)), itControl[size - 1]);
    ASSERT_EQ(*(-1 + end), itControl[size-1]);
    // Make sure iterator was not moved.
    ASSERT_EQ(*it, itControl[0]);

    auto cit = itControl.cbegin();
    auto cend = itControl.cend();

    // Same tests for const_iterator
    cit += 1;
    ASSERT_EQ(*cit, itControl[1]);

    cit += -1;
    ASSERT_EQ(*cit, itControl[0]);

    cit += capacity * -1;
    ASSERT_EQ(*cit, itControl[0]);
    cit += capacity;
    ASSERT_EQ(*cit, itControl[0]);

    ASSERT_EQ(*(cit+1), itControl[1]);
    ASSERT_EQ(*(1 + cit), itControl[1]);
    ASSERT_EQ(*(cend + (-1)), itControl[size -1]);
    ASSERT_EQ(*(-1 + cend), itControl[size-1]);

    ASSERT_EQ(*cit, itControl[0]);
}