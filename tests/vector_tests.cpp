#include <gtest/gtest.h>

#include "MyVector.h"

// --- MyVector<int> (общий шаблон) ---

TEST(MyVectorInt, DefaultIsEmpty) {
    MyVector<int> v;
    EXPECT_EQ(v.size(), 0u);
    EXPECT_EQ(v.capacity(), 0u);
}

TEST(MyVectorInt, PushBackIncreasesSize) {
    MyVector<int> v;
    v.push_back(42);
    EXPECT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], 42);
}

TEST(MyVectorInt, OperatorBracketRoundTrip) {
    MyVector<int> v;
    v.push_back(1);
    v.push_back(2);
    v[0] = 100;
    EXPECT_EQ(v[0], 100);
    EXPECT_EQ(v[1], 2);
}

TEST(MyVectorInt, PopBackDecreasesSize) {
    MyVector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.pop_back();
    EXPECT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], 1);
}

TEST(MyVectorInt, CapacityGrowsWithPushBack) {
    MyVector<int> v;
    for (int i = 0; i < 10; ++i) {
        v.push_back(i);
    }
    EXPECT_EQ(v.size(), 10u);
    EXPECT_GE(v.capacity(), 10u);
}

TEST(MyVectorInt, ConstIndexRead) {
    MyVector<int> v;
    v.push_back(7);
    const MyVector<int>& cv = v;
    EXPECT_EQ(cv[0], 7);
}

// --- MyVector<bool> (специализация, биты в uint8_t) ---

TEST(MyVectorBool, DefaultIsEmpty) {
    MyVector<bool> v;
    EXPECT_EQ(v.size(), 0u);
    EXPECT_EQ(v.capacity(), 0u);
}

TEST(MyVectorBool, PushBackAndSubscript) {
    MyVector<bool> v;
    v.push_back(true);
    v.push_back(false);
    v.push_back(true);
    EXPECT_TRUE(v[0]);
    EXPECT_FALSE(v[1]);
    EXPECT_TRUE(v[2]);
}

TEST(MyVectorBool, ReferenceAssignment) {
    MyVector<bool> v;
    v.push_back(false);
    v.push_back(false);
    v[0] = true;
    v[1] = true;
    EXPECT_TRUE(v[0]);
    EXPECT_TRUE(v[1]);
}

TEST(MyVectorBool, MultipleBitsShareOneByte) {
    MyVector<bool> v;
    for (int i = 0; i < 8; ++i) {
        v.push_back(i % 2 == 0);
    }
    EXPECT_EQ(v.size(), 8u);
    for (std::size_t i = 0; i < 8; ++i) {
        EXPECT_EQ(static_cast<bool>(v[i]), (i % 2 == 0));
    }
}

TEST(MyVectorBool, ConstSubscriptReadsBits) {
    MyVector<bool> v;
    v.push_back(true);
    v.push_back(false);
    const MyVector<bool>& cv = v;
    EXPECT_TRUE(cv[0]);
    EXPECT_FALSE(cv[1]);
}

TEST(MyVectorBool, PopBack) {
    MyVector<bool> v;
    v.push_back(true);
    v.push_back(false);
    v.pop_back();
    EXPECT_EQ(v.size(), 1u);
    EXPECT_TRUE(v[0]);
}
