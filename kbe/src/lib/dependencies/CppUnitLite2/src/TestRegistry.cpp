#include "TestRegistry.h"
#include "Test.h"
#include "TestResult.h"

static TestRegistry* g_registry = 0;


TestRegistry& TestRegistry::Instance () 
{
    if( !g_registry )
        g_registry = new TestRegistry;
    return *g_registry;
}

void TestRegistry::Destroy()
{
    delete g_registry;
    g_registry = 0;
}


TestRegistry::TestRegistry()
    : m_testCount(0)
{
}

void TestRegistry::Add (Test* test) 
{
    m_tests[m_testCount] = test;
    ++m_testCount;
}

void TestRegistry::Run (TestResult& result) 
{
    result.StartTests();
    for (int i = 0; i < m_testCount; ++i)
        m_tests[i]->Run (result);
    result.EndTests();
}



