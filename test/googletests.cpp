#include <gtest/gtest.h>
#include "RingBuffer.hpp"
#include "TestClass.hpp"
#include <string>
#include <vector>

TEST(mainframe, defaultConstruction){

    RingBuffer<int> control;
    RingBuffer<int> experiment(control);
    EXPECT_EQ(control.empty(), true);

}

TEST(mainframe, copyConstruction){

    RingBuffer<int> control(5);
    RingBuffer<int> experiment(control);
    EXPECT_EQ(experiment.size(), control.size());
    EXPECT_EQ(sizeof(experiment), sizeof(control));
}

    //EXPECT_EQ(experiment.end(), control.end());
    //EXPECT_EQ(experiment.begin(),control.begin());