#include "TestResultDebugOut.h"
#include "../Failure.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <windows.h>


void TestResultDebugOut::StartTests ()
{
    OutputDebugString(L"\n\nRunning unit tests...\n\n");
}


void TestResultDebugOut::AddFailure (const Failure & failure) 
{
    TestResult::AddFailure(failure);

    std::wostringstream woss;
    woss << failure;
    OutputDebugString(woss.str().c_str());
}

void TestResultDebugOut::EndTests () 
{
    TestResult::EndTests();

    std::wostringstream woss;
    woss << m_testCount << L" tests run" << std::endl;
    if (m_failureCount > 0)
        woss << L"****** There were " << m_failureCount << L" failures." << std::endl;
    else
        woss << L"There were no test failures." << std::endl;

    woss << L"Test time: " << std::setprecision(3) << m_secondsElapsed << L" seconds." << std::endl;

    OutputDebugString(woss.str().c_str());
    OutputDebugString(L"\n");
}
