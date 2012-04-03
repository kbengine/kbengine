#include "../CppUnitLite2.h"


class TestFixture 
{
public:
    TestFixture()
    {
        ++s_instanceCount;
        ++s_instancesEverCreated;
    }
    ~TestFixture()
    {
        --s_instanceCount;
    }
    
    static int s_instanceCount;
    static int s_instancesEverCreated;
};

int TestFixture::s_instanceCount = 0;
int TestFixture::s_instancesEverCreated = 0;


TEST(NoInstancesBeforeTest)
{
    CHECK_EQUAL(0, TestFixture::s_instanceCount);
}

TEST_F (TestFixture, OneInstanceDuringFixtureTest)
{
    CHECK_EQUAL(1, TestFixture::s_instanceCount);
}

TEST(NoInstancesAfterTest)
{
    CHECK_EQUAL(0, TestFixture::s_instanceCount);
    CHECK_EQUAL(1, TestFixture::s_instancesEverCreated);
}

TEST_F (TestFixture, OneInstanceDuringSecondFixtureTest)
{
    CHECK_EQUAL(1, TestFixture::s_instanceCount);
}

TEST(NoInstancesAfterSecondTest)
{
    CHECK_EQUAL(0, TestFixture::s_instanceCount);
    CHECK_EQUAL(2, TestFixture::s_instancesEverCreated);
}
