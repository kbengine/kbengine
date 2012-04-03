#include "../CppUnitLite2.h"

TEST (MyFirstTest)
{
    int a = 102;
    CHECK_EQUAL (102, a);
}


struct MyFixture
{
    MyFixture() 
    {
        someValue = 2.0f;
        str = "Hello";
    }

    float someValue;
    std::string str;
};


TEST_F (MyFixture, TestCase1)
{
    CHECK_CLOSE (someValue, 2.0f, 0.00001);
    someValue = 13;
}

TEST_F (MyFixture, TestCase2)
{
    CHECK_CLOSE (someValue, 2.0f, 0.00001);
    CHECK_EQUAL (str, "Hello");
}

