#include "Test.h"
#include "TestRegistry.h"
#include "TestResult.h"
#include "Failure.h"
#include "TestException.h"

#ifdef LINUX
    #include "Linux/SignalHandler.h"
#endif


TestRegistrar::TestRegistrar(Test * test)
{
    TestRegistry::Instance().Add (test);    
}


Test::Test (const char * testName, 
            const char * filename, int linenumber)
    : m_name (testName)
    , m_filename (filename)
    , m_linenumber (linenumber)
{
}

Test::~Test()
{
}



void Test::Run (TestResult& result) 
{   
    try
    {       
#ifdef LINUX
        SignalHandler handler(ExceptionHandler::IsOn());
#endif
        
        RunTest (result);
    }
    catch (const TestException& e)
    {
        ExceptionHandler::Handle(result, e, m_name, m_filename, m_linenumber );
    }
    catch (const std::exception& e)
    {
        ExceptionHandler::Handle(result, e.what(), m_name, m_filename, m_linenumber);
    }
    catch (...) 
    {
        ExceptionHandler::Handle(result, "(unknown, GPF?) ", m_name, m_filename, m_linenumber);
    }
    
    result.TestWasRun();
}
