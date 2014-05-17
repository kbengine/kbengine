#include "TestResultStdErr.h"
#include "Failure.h"
#include <iostream>
#include <iomanip>


void TestResultStdErr::AddFailure (const Failure & failure) 
{
    TestResult::AddFailure(failure);
    std::cerr << failure << std::endl;
}

void TestResultStdErr::EndTests () 
{
    TestResult::EndTests();
    std::cerr << m_testCount << " tests run" << std::endl;
    if (m_failureCount > 0)
        std::cerr << "****** There were " << m_failureCount << " failures." << std::endl;
    else
        std::cerr << "There were no test failures." << std::endl;

    std::cerr << "Test time: " << std::setprecision(3) << m_secondsElapsed << " seconds." << std::endl;
}

